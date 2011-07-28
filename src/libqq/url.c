#include <url.h>
#include <sys/socket.h>
#include <string.h>
#include <glib.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <glib/gprintf.h>

#include <zlib.h>

Connection* connection_new()
{
    Connection *con = g_slice_new(Connection);
    con -> fd = -1;
    con -> channel = NULL;
    return con;
}

void connection_free(Connection *con)
{
    if(con == NULL){
        return;
    }
    g_slice_free(Connection, con);
}

Connection* connect_to_host(const char *hostname, int port)
{
    if(NULL == hostname){
        return NULL;
    }
    int sockfd = -1, err;
    struct addrinfo *ailist = NULL, *aip = NULL;
    struct addrinfo hint;
    
    memset(&hint, 0, sizeof(hint));
    hint.ai_socktype = SOCK_STREAM;
    
    if((err = getaddrinfo(hostname, NULL, &hint, &ailist)) != 0){
        g_warning("Can't get the address information of %s (%s, %d)"
                ,hostname , __FILE__, __LINE__);
        return NULL;
    }
    
    struct sockaddr_in *sinp;
    
    for(aip = ailist; aip != NULL; aip = aip -> ai_next){
        
        if((sockfd =socket(aip -> ai_family, SOCK_STREAM, 0)) < 0){
            g_warning("Can't create a socket.(%s, %d)"
                    , __FILE__, __LINE__);
            return NULL;
        }
        
        sinp = (struct sockaddr_in *)aip -> ai_addr;
        /*
         * the http protocol uses port 80
         */
        sinp -> sin_port = htons((gint16)port);
        
        if(connect(sockfd,aip -> ai_addr, aip -> ai_addrlen) < 0){
            close(sockfd);
            sockfd = -1;
            g_warning("Can't connect to the server.(%s, %d)"
                    , __FILE__, __LINE__);
            continue;
        }
        //connect to the host success.
        break;
    }
    freeaddrinfo(ailist);
    
    Connection *con = connection_new();
    con -> fd = sockfd;
    
    GIOChannel *channel = g_io_channel_unix_new(sockfd);
    //read and write binary data.
    g_io_channel_set_encoding(channel, NULL, NULL);
    con -> channel = channel;
    
    return con;
}

void close_con(Connection *con)
{
    if(con == NULL){
        return;
    }
    g_io_channel_shutdown(con -> channel, TRUE, NULL);
    g_io_channel_unref(con -> channel);
    //shutdown(fd);
    close(con -> fd);
}

gint send_request(Connection *con, Request *r)
{
    if(con == NULL || r == NULL){
        return -1;
    }
    
    GString *rq = request_tostring(r);
//    g_printf("\nMESSAGE:  (%s, %d)Send reqeust : %s\n"
//            ,__FILE__, __LINE__,  rq -> str);

    GIOStatus status;
    GError *err = NULL;
    gsize bytes_written = 0;
    gsize has_written = 0;

    while(has_written < rq -> len){
        status = g_io_channel_write_chars(con -> channel
                , rq -> str + has_written
                , rq -> len - has_written
                , &bytes_written
                , &err);
        switch(status)
        {
        case G_IO_STATUS_NORMAL:
            //write success.
            has_written += bytes_written;
            //g_debug("Write %d bytes data.(%s, %d)"
            //        , bytes_written, __FILE__
            //        , __LINE__);
            break;
        case G_IO_STATUS_EOF:
            g_warning("Write data EOF!! What's happenning?(%s, %d)"
                    , __FILE__, __LINE__);
            return -1;
        case G_IO_STATUS_ERROR:
            g_warning("Write data ERROR!! code:%d msg:%s (%s, %d)"
                    , err -> code, err -> message
                    , __FILE__, __LINE__);
            g_error_free(err);
            return -1;
        case G_IO_STATUS_AGAIN:
            g_debug("Channel temporarily unavailable.(%s, %d)"
                    , __FILE__, __LINE__);
            break;
        default:
            g_warning("Unknown io status!(%s, %d)"
                    , __FILE__, __LINE__);
            return -1;
        }
    }
    status = g_io_channel_flush(con -> channel, &err);
    if(status != G_IO_STATUS_NORMAL){
        g_warning("Flush io channel error! But don't warry...(%s, %d)"
                , __FILE__, __LINE__);
    }    
    //g_debug("Write all date.(%s, %d)", __FILE__, __LINE__);
    g_string_free(rq, TRUE);
    return 0;
}

/*
 * Ungzip data.
 * use zlib
 */
static int ungzip(GString *in, GString *out)
{
    if(in == NULL || out == NULL){
        return 0;
    }

    gulong inlen = in -> len;
    g_string_truncate(out, 0);

    int ret;
#define BUFSIZE 4096
    gchar buf[BUFSIZE];

    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    
    /*
     * 47 enable zlib and gzip decoding with automatic header detection.
     */
    ret = inflateInit2(&strm, 47);
    switch(ret)
    {
    case Z_OK:
        g_debug("Initial zlib. done.(%s, %d)", __FILE__, __LINE__);
        break;
    case Z_MEM_ERROR:
    case Z_VERSION_ERROR:
    case Z_STREAM_ERROR:
        g_warning("inflateInit2() Error!. %s (%s, %d)", strm.msg, __FILE__
                    , __LINE__);
        return -1;
    default:
        g_warning("Unknown return!!(%s, %d)", __FILE__, __LINE__);
        return -1;
    }
    
    gboolean done = FALSE;
    strm.avail_in = inlen;
    strm.next_in = (Bytef*)(in -> str);
    while(!done){
        strm.avail_out = BUFSIZE;
        strm.next_out = (Bytef*)buf;
        ret = inflate(&strm, Z_NO_FLUSH);
        switch(ret)
        {
        case Z_STREAM_END:
            done = TRUE;
            g_debug("Unzip done.(%s, %d)", __FILE__, __LINE__);
            break;
        case Z_OK:
            break;
        case Z_BUF_ERROR:
            g_debug("Unzip error!(%s, %d)", __FILE__, __LINE__);
            done = TRUE;
            break;
        case Z_DATA_ERROR:
        case Z_MEM_ERROR:
        case Z_STREAM_ERROR:
            g_warning("Ungzip stream error! %s(%s, %d)", strm.msg, __FILE__
                    , __LINE__);
            g_string_truncate(out, 0);
            return -1;
        }
        g_string_append_len(out, buf, BUFSIZE - strm.avail_out);
    }
#undef BUFSIZE
    
    g_debug("Before %d bytes, after %d bytes (%s, %d)", in -> len, out -> len
                                        , __FILE__, __LINE__);
    g_printf("UNZIP: %s\n", out -> str);
    return 1;

}

gint rcv_response(Connection *con, Response **rp)
{
    if(con == NULL || rp == NULL){
        return -1;
    }
    Response *r = NULL;
    gsize need_to_read = G_MAXSIZE;     //we don't know the length of
                                        //the data that need to read.
    //store the data that has read
    GString *data = g_string_new(NULL);
    gchar *tev = NULL;                  //the transfer encoding
    gint cl = -1;                       //the content length
    gboolean gotcl = FALSE;             //got content lenght or not
                                        //so, we should do sth
    gboolean ischunked = FALSE;
    gint chunkbegin = 0;
    gint chunklen = -1;                 //the chunk length
    gint totalchunklen = 0;

    gboolean conclose = FALSE;          //connection close
    gboolean gotallheaders = FALSE;
    gboolean isgzip = FALSE;

    #define BUFSIZE 500
    gchar buf[BUFSIZE];
    GIOStatus status;
    GError *err = NULL;
    gsize bytes_read = 0;
    gsize want_read = 0;
    
    //g_debug("Begin to read data.(%s, %d)", __FILE__, __LINE__);
    while(need_to_read > 0){
        want_read = BUFSIZE < need_to_read ? BUFSIZE : need_to_read;
        status = g_io_channel_read_chars(con -> channel, buf
                                            , want_read, &bytes_read, &err);
        switch(status)
        {
        case G_IO_STATUS_NORMAL:
            //read success.
            need_to_read -= bytes_read;
            break;
        case G_IO_STATUS_EOF:
            if(conclose){
                /*
                 * The Connection header is close.
                 * The server will close the conntion after 
                 * send all the data.
                 */
                //we got all the data.
                g_debug("Server close the connection. "
                        "data has gotten: %u(%s, %d)"
                        , (unsigned int)bytes_read ,__FILE__, __LINE__);
                break;
            }
            break;
        case G_IO_STATUS_ERROR:
            g_warning("Read data ERROR!! code:%d msg:%s(%s, %d)"
                            , err -> code, err -> message
                            , __FILE__, __LINE__);
            g_string_free(data, TRUE);
            g_error_free(err);
            return -1;
        case G_IO_STATUS_AGAIN:
            g_warning("Channel temporarily unavailable.");
            break;
        default:
            g_warning("Unknown io status!(%s, %d)", __FILE__, __LINE__);
            g_string_free(data, TRUE);
            return -1;
        }
        g_string_append_len(data, buf, bytes_read);
        
        if(!gotallheaders && g_strstr_len(data -> str, data -> len
                                                    , "\r\n\r\n") != NULL){
            //We have gotten all the headers;
            r = response_new_parse(data);
            g_string_truncate(data, 0);
            gotallheaders = TRUE;
            //Find the Content-Length 's value
            gchar *clen = response_get_header_chars(r, "Content-Length");
            if(clen != NULL){
                gotcl = TRUE;
                //calculate the message we have not read.
                cl = atoi(clen);
                g_debug("Content-Length: %d.(%s, %d)" , cl, __FILE__, __LINE__);
                need_to_read = cl - r -> msg -> len;
                //g_debug("Message need to read %d bytes."
                //        "(%s, %d)", need_to_read
                //        , __FILE__, __LINE__);
            }

            //Find the Transfering-Encoding 's value
            tev = response_get_header_chars(r, "Transfer-Encoding");
            if(tev != NULL && g_strstr_len(tev, -1, "chunked") != NULL){
                g_debug("The message body is chunked.(%s, %d)"
                                , __FILE__, __LINE__);
                ischunked = TRUE;

                //copy the message back to data
                g_string_truncate(data , 0);
                g_string_append_len(data, r -> msg -> str, r -> msg -> len);
                g_string_truncate(r -> msg, 0);

                chunklen = -1;
                chunkbegin = 0;
                /*
                 * We will read the data according to the
                 * chunked
                 */
                need_to_read = G_MAXSIZE;
            }

            gchar *connection = response_get_header_chars(r
                        , "Connection");    
            if(connection != NULL){
                //g_debug("Connection: %s (%s, %d)", connection
                //        , __FILE__, __LINE__);
                if(g_strstr_len(connection, -1, "close") 
                            != NULL){
                    conclose = TRUE;
                }
            }

            gchar *ce = response_get_header_chars(r
                    , "Content-Encoding");
            if(ce != NULL){
                g_debug("Content-Encoding: %s (%s, %d)", ce
                        , __FILE__, __LINE__);
                if(g_strstr_len(ce, -1, "gzip") != NULL){
                    isgzip = TRUE;
                }
            }
        }

        if(gotallheaders && ischunked){
            while(TRUE){
                if(chunkbegin + 2 > data -> len){
                    g_debug("need more data ... (%s, %d)", __FILE__, __LINE__);
                    break;
                }
                gchar *tmp = g_strstr_len(data -> str + chunkbegin
                                        , data -> len - chunkbegin, "\r\n");
                if(tmp != NULL){
                    /*
                     * we got the length
                     */
                    *tmp = '\0';
                    gchar *end = NULL;
                    chunklen = strtol(data -> str + chunkbegin, &end, 16);
                    *tmp = '\r';
                    if(end != data -> str + chunkbegin && chunklen == 0){
                        // Get all data
                        need_to_read = -1;
                        g_debug("Get all chunks! Totla len %d (%s, %d)"
                                        , r -> msg -> len, __FILE__, __LINE__);
                        break;
                    }
                }else{
                    g_debug("More chunks... Begin %d len %d %s(%s, %d)"
                                        , chunkbegin, data -> len
                                        , data -> str + chunkbegin
                                        , __FILE__, __LINE__);
                    break;
                }

                if(chunklen != -1 && (tmp - data -> str) + 2 + chunklen 
                                                            <= data -> len){
                    totalchunklen += chunklen;
                    chunkbegin = tmp - data -> str;
                    chunkbegin += 2;

                    g_debug("Append chunk. begin: %d len %d length : %d "
                                            "+ %d(%s, %d)", chunkbegin
                                            , data -> len, r -> msg -> len
                                            , chunklen, __FILE__, __LINE__);

                    g_string_append_len(r -> msg, data -> str + chunkbegin
                                            , chunklen);
                    chunkbegin += chunklen + 2;
                    chunklen = -1;
                }else{
                    break;
                }
            }
        }
        if(bytes_read < want_read){
            break;
        }    
    }//end of while(need_to_read > 0)...
    //g_debug("Read all data.(%s, %d)", __FILE__, __LINE__);

    if(r == NULL){
        //we do not find "\r\n\r\n".
        //Should not happen.
        g_warning("Read all data, but not find all headers.!"
                    "(%s, %d)", __FILE__, __LINE__);
        g_string_free(data, TRUE);
        return -1;
    }

    if(!ischunked){
        //copy the message to r -> msg;
        g_string_append_len(r -> msg, data -> str, data -> len);
    }else{
        //g_debug("Total chunk length: %d (%s, %d)", totalchunklen
        //            , __FILE__, __LINE__);
    }
    #undef BUFSIZE
    
    if(gotcl && r -> msg -> len != cl && tev == NULL){
        g_warning("No read all the message!! content length:%d"
                    " msg -> len: %u. (%s, %d)"
                    , cl, (unsigned int)r -> msg -> len, __FILE__, __LINE__);
    }

    if(isgzip){
        /*
         * ungzip the data
         */
        GString *out = g_string_new(NULL);
        ungzip(r -> msg, out);
        g_string_truncate(r -> msg, 0);
        g_string_append(r -> msg, out -> str);
        g_string_free(out, TRUE);
        g_debug("Ungzip data. After len %u.(%s, %d)"
                    , (unsigned int)r -> msg -> len, __FILE__, __LINE__);
    }

    *rp = r;
    g_string_free(data, TRUE);
    return 0;
}

