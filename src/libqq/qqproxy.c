/* Last modified Time-stamp: <2011-12-06 11:35:08 Tuesday by devil>
 * @(#)qqproxy.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <memory.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <stdarg.h>
#include <fcntl.h>
#include <signal.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>
#include <pwd.h>
#include <termios.h>
#include <sys/time.h>
#include <stdarg.h>
#include <glib/gprintf.h>

#include <arpa/nameser.h>
#include <resolv.h>
#include <netdb.h>

#include <qqproxy.h>

const char *digits    = "0123456789"; 
const char *dotdigits = "0123456789.";

struct ADDRPAIR direct_addr_list[MAX_DIRECT_ADDR_LIST];
int n_direct_addr_list = 0;

struct sockaddr_in socks_ns;
int connect_timeout = 0; 
int proxy_auth_type = PROXY_AUTH_NONE;
/* local input type */

const char *dest_host = NULL;
// struct sockaddr_in dest_addr; for socks5 
u_short dest_port = 0;

/* char *local_type_names[] = { "stdio", "socket" }; */
/* int   local_type = LOCAL_STDIO; */
/* u_short local_port = 0;                         /\* option 'p' *\/ */
/* int f_hold_session = 0;                         /\* option 'P' *\/ */

static int   relay_method = METHOD_DIRECT;          /* relaying method */
static char *relay_host = NULL;                        /* hostname of relay server */
static u_short relay_port = 0;                         /* port of relay server */
//static char *relay_user = NULL;  


void *xmalloc (size_t size)
{
    void *ret = malloc(size);
    if (ret == NULL)
        g_error("Cannot allocate memory: %d bytes.\n", size);
    return ret;
}

char * downcase( char *str )
{
    char *buf = str;
    while ( *buf ) {
        if ( isupper(*buf) )
            *buf += 'a'-'A';
        buf++;
    }
    return str;                            
}


char *
expand_host_and_port (const char *fmt, const char *host, int port)
{
    const char *src;
    char *buf, *dst;
    size_t len = strlen(fmt) + strlen(host) + 20;
    buf = xmalloc (len);
    dst = buf;
    src = fmt;
    
    while (*src) {
        if (*src == '%') {
            switch (src[1]) {
                case 'h':
                    strcpy (dst, host);
                    src += 2;
                    break;
                case 'p':
                    snprintf (dst, len, "%d", port);
                    src += 2;
                    break;
                default:
                    src ++;
                    break;
            }
            dst = buf + strlen (buf);
        } else if (*src == '\\') {
            switch (src[1]) {
                case 'r':			
                    *dst++ = '\r';
                    src += 2;
                    break;
                case 'n':			
                    *dst++ = '\n';
                    src += 2;
                    break;
                case 't':			
                    *dst++ = '\t';
                    src += 2;
                    break;
                default:
                    src ++;
                    break;
            }
        } else {
           
            *dst++ = *src++;
        }
        *dst = '\0';
    }
    assert (strlen(buf) < len);
    return buf;
}

int  lookup_resolve( const char *str )
{
    char *buf = strdup( str );
    int ret;

    downcase( buf );
    if ( strcmp( buf, "both" ) == 0 )
        ret = RESOLVE_BOTH;
    else if ( strcmp( buf, "remote" ) == 0 )
        ret = RESOLVE_REMOTE;
    else if ( strcmp( buf, "local" ) == 0 )
        ret = RESOLVE_LOCAL;
    else if ( strspn(buf, dotdigits) == strlen(buf) ) {
#ifndef WITH_RESOLVER
        g_error("Sorry, you can't specify to resolve the hostname with the -R option on Win32 environment.");
#endif 
        ret = RESOLVE_LOCAL;                  
        socks_ns.sin_addr.s_addr = inet_addr(buf);
        socks_ns.sin_family = AF_INET;
    }
    else
        ret = RESOLVE_UNKNOWN;
    free(buf);
    return ret;
}


char *
getusername(void)
{
    struct passwd *pw = getpwuid(getuid());
    if ( pw == NULL )
        g_error("getpwuid() failed for uid: %d\n", getuid());
    return pw->pw_name;
}

int
expect( char *str, char *substr)
{
    int len = strlen(substr);
    while ( 0 < len-- ) {
        if ( toupper(*str) != toupper(*substr) )
            return 0;                         
        str++, substr++;
    }
    return 1;                 
}


PARAMETER_ITEM* find_parameter_item(const char* name)
{
    int i;
    for( i = 0; parameter_table[i].name != NULL; i++ ){
        if ( strcmp(name, parameter_table[i].name) == 0 )
            return &parameter_table[i];
    }
    return NULL;
}


void sig_timeout(void)
{
    signal( SIGALRM, SIG_IGN );
    alarm( 0 );
    g_error( "timed out\n" );
    exit(1);
}


void set_timeout(int timeout)
{
   
    if ( timeout == 0 ) {
        g_debug( "clearing timeout\n" );
        signal( SIGALRM, SIG_IGN );
        alarm( 0 );
    } else {
        g_debug( "setting timeout: %d seconds\n", timeout );
        signal(SIGALRM, (void *)sig_timeout);
        alarm( timeout );
    }
}

int domain_match(const char *s1, const char *s2)
{
    int len1, len2;
    const char *tail1, *tail2;
    len1 = strlen(s1);
    len2 = strlen(s2);
    if (len1 < len2 || len1 == 0 || len2 == 0)
        return 0;                             
    tail1 = s1 + len1;
    tail2 = s2 + len2;
    while (0 < len1 && 0 < len2) {
        if (*--tail1 != *--tail2)
            break;                            
        len1--, len2--;
    }
    if (len2 != 0)
        return 0;                              
    
    if (tail1 == s1 || tail1[-1] == '.')
        return 1;                             
    return 0;                                
}

int  is_direct_name (const char *name)
{
    int len, i;
    g_debug("checking %s is for direct?\n", name);
    name = downcase(strdup(name));
    len = strlen(name);
    if (len < 1)
        return 0;                             
    for (i=0; i<n_direct_addr_list; i++ ) {
        int neg;
        const char *dname;
        dname = direct_addr_list[i].name;
        if (dname == NULL)
            continue;                          
        neg = direct_addr_list[i].negative;
        if (domain_match(name, dname)) {
            //debug("match with: %s%s\n", dname, neg? " (negative)": "");
            if (neg) {
                return 0;      
            } else {
                return 1;      
            }
        }
    }
    return 0;                               
}

void mask_addr (void *addr, void *mask, int addrlen)
{
    char *a, *m;
    a = addr;
    m = mask;
    while ( 0 < addrlen-- )
        *a++ &= *m++;
}

int cmp_addr (void *addr1, void *addr2, int addrlen)
{
    return memcmp( addr1, addr2, addrlen );
}

int is_direct_address (const struct in_addr addr)
{
    int i, neg;
    struct in_addr iaddr;

    /* Note: assume IPV4 address !! */
    for (i=0; i<n_direct_addr_list; i++ ) {
        if (direct_addr_list[i].name != NULL)
            continue;                           /* it's name entry */
        neg = direct_addr_list[i].negative;
        iaddr = addr;
        mask_addr( &iaddr, &direct_addr_list[i].mask,
                   sizeof(struct in_addr));
        if (cmp_addr(&iaddr, &direct_addr_list[i].addr,
                     sizeof(struct in_addr)) == 0) {
            char *a, *m;
            a = strdup(inet_ntoa(direct_addr_list[i].addr));
            m = strdup(inet_ntoa(direct_addr_list[i].mask));
            g_debug("match with: %s/%s%s\n", a, m, neg? " (negative)": "");
            free(a);
            free(m);
            return !neg? 1: 0;
        }
    }
    g_debug("not matched, addr to be relayed: %s\n", inet_ntoa(addr));
    return 0;                   /* not direct */
}

int check_direct(const char *host)
{
    struct in_addr addr;
    addr.s_addr = inet_addr(host);
    if (addr.s_addr != INADDR_NONE) {
      
        if (is_direct_address(addr)) {
            g_debug("%s is for direct.\n", host);
            return 1;                        
        }
    } else {
       
        if (is_direct_name(host)) {
            g_debug("%s is for direct.\n", host);
            return 1;                          
        }
    }
    g_debug("%s is for not direct.\n", host);
    return 0;                                  
}


int local_resolve (const char *host, struct sockaddr_in *addr)
{
    struct hostent *ent;
    if ( strspn(host, dotdigits) == strlen(host) ) {
        addr->sin_family = AF_INET;
        addr->sin_addr.s_addr = inet_addr(host);
    } else {
        g_debug("resolving host by name: %s\n", host);
        ent = gethostbyname (host);
        if ( ent ) {
            memcpy (&addr->sin_addr, ent->h_addr, ent->h_length);
            addr->sin_family = ent->h_addrtype;
            g_debug("resolved: %s (%s)\n",
                    host, inet_ntoa(addr->sin_addr));
        } else {
            g_debug("failed to resolve locally.\n");
            return -1;                          /* failed */
        }
    }
    return 0;                                   /* good */
}

int open_connection( const char *host, short port )
{
    SOCKET s;
    struct sockaddr_in saddr;

    /* resolve address of proxy or direct target */
    if (local_resolve (host, &saddr) < 0) {
        g_error("can't resolve hostname: %s\n", host);
        return SOCKET_ERROR;
    }
    saddr.sin_port = htons(port);

    g_debug("connecting to %s:%u\n", inet_ntoa(saddr.sin_addr), port);
    s = socket( AF_INET, SOCK_STREAM, 0 );
    if ( connect( s, (struct sockaddr *)&saddr, sizeof(saddr))
         == SOCKET_ERROR) {
        g_debug( "connect() failed.\n");
        return SOCKET_ERROR;
    }
    return s;
}

int atomic_out( SOCKET s, char *buf, int size )
{
    int ret, len;

    assert( buf != NULL );
    assert( 0<=size );
  
    ret = 0;
    while ( 0 < size ) {
        len = send( s, buf+ret, size, 0 );
        if ( len == -1 )
            g_error("atomic_out() failed to send(), %d\n", socket_errno());
        ret += len;
        size -= len;
    }
    g_debug("atomic_out()  [%d bytes]\n", ret);
    return ret;
}

int
atomic_in( SOCKET s, char *buf, int size )
{
    int ret, len;

    assert( buf != NULL );
    assert( 0<=size );

 
    ret = 0;
    while ( 0 < size ) {
        len = recv( s, buf+ret, size, 0 );
        if ( len == -1 ) {
            g_error("atomic_in() failed to recv(), %d\n", socket_errno());
        } else if ( len == 0 ) {
            g_error( "Connection closed by peer.\n");
        }
        ret += len;
        size -= len;
    }
    g_debug("atomic_in()  [some bytes]\n");
    return ret;
}

int
line_input( SOCKET s, char *buf, int size )
{
    char *dst = buf;
    if ( size == 0 )
        return 0;                             
    size--;
    while ( 0 < size ) {
        switch ( recv( s, dst, 1, 0) ) {      
            case SOCKET_ERROR:
                g_error("recv() error\n");
                return -1;                       
            case 0:
                size = 0;                         
                break;
            default:
           
                if ( *dst == '\n' ) {
              
                    size = 0;
                } else {
              
                    size--;
                }
                dst++;
        }
    }
    *dst = '\0';
   
    return 0;
}


char *
cut_token( char *str, char *delim)
{
    char *ptr = str + strcspn(str, delim);
    char *end = ptr + strspn(ptr, delim);
    if ( ptr == str )
        return NULL;
    while ( ptr < end )
        *ptr++ = '\0';
    return ptr;
}

int sendf(SOCKET s, const char *fmt,...)
{
    static char buf[10240];                   
    
    va_list args;
    va_start( args, fmt );
    vsnprintf( buf, sizeof(buf), fmt, args );
    va_end( args );

    //report_text(">>>", buf);
    if ( send(s, buf, strlen(buf), 0) == SOCKET_ERROR ) {
        g_debug("failed to send http request. errno=%d\n", socket_errno());
        return -1;
    }
    return 0;
}


const char *base64_table =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

char * make_base64_string(const char *str)
{
    static char *buf;
    char *src;
    char *dst;
    int bits, data, src_len, dst_len;
    /* make base64 string */
    src_len = strlen(str);
    dst_len = (src_len+2)/3*4;
    buf = xmalloc(dst_len+1);
    bits = data = 0;
    src = (char *)str;
    dst = (char *)buf;
    while ( dst_len-- ) {
        if ( bits < 6 ) {
            data = (data << 8) | *src;
            bits += 8;
            if ( *src != 0 )
                src++;
        }
        *dst++ = base64_table[0x3F & (data >> (bits-6))];
        bits -= 6;
    }
    *dst = '\0';
    /* fix-up tail padding */
    switch ( src_len%3 ) {
        case 1:
            *--dst = '=';
        case 2:
            *--dst = '=';
    }
    return buf;
}

/*
  int basic_auth (SOCKET s)
  {
  char *userpass;
  char *cred;
  const char *user = relay_user;
  char *pass = NULL;
  int len, ret;

    
  if (user == NULL)
  g_error("Cannot decide username for proxy authentication.");
  if ((pass = determine_relay_password ()) == NULL &&
  (pass = readpass("Enter proxy authentication password for %s@%s: ",
  relay_user, relay_host)) == NULL)
  g_error("Cannot decide password for proxy authentication.");

  len = strlen(user)+strlen(pass)+1;
  userpass = xmalloc(len+1);
  snprintf(userpass, len+1, "%s:%s", user, pass);
  memset (pass, 0, strlen(pass));
  cred = make_base64_string(userpass);
  memset (userpass, 0, len);

  ret = sendf(s, "Proxy-Authorization: Basic %s\r\n", cred);
  f_report = 1;
    
  memset(cred, 0, strlen(cred));
  free(cred);

  return ret;
  }
*/

int begin_http_relay( SOCKET s )
{
    char buf[1024];
    int result;
    char *auth_what;

    g_debug("begin_http_relay()\n");

    if (sendf(s,"CONNECT %s:%d HTTP/1.0\r\n", dest_host, dest_port) < 0)
        return START_ERROR;
    /*
      if (proxy_auth_type == PROXY_AUTH_BASIC  *&& basic_auth (s)  < 0) 
      return START_ERROR;
    */
    if (sendf(s,"\r\n") < 0)
        return START_ERROR;

    
    if ( line_input(s, buf, sizeof(buf)) < 0 ) {
        g_debug("failed to read http response.\n");
        return START_ERROR;
    }

   
    if (!strchr(buf, ' ')) {
        g_error ("Unexpected http response: '%s'.\n", buf);
        return START_ERROR;
    }
    result = atoi(strchr(buf,' '));

    switch ( result ) {
        case 200:
         
            g_debug("connected, start user session.\n");
            break;
        case 302:                                  
            do {
                if (line_input(s, buf, sizeof(buf)))
                    break;
                downcase(buf);
                if (expect(buf, "Location: ")) {
                    relay_host = cut_token(buf, "//");
                    cut_token(buf, "/");
                    relay_port = atoi(cut_token(buf, ":"));
                }
            } while (strcmp(buf,"\r\n") != 0);
            return START_RETRY;

           
        case 401:                                   
        case 407:                                  
            /*TODO - authentication*/
            if (proxy_auth_type != PROXY_AUTH_NONE) {
                g_error("Authentication failed.\n");
                return START_ERROR;
            }
            auth_what = (result == 401) ? "WWW-Authenticate:" : "Proxy-Authenticate:";
            do {
                if ( line_input(s, buf, sizeof(buf)) ) {
                    break;
                }
                downcase(buf);
                if (expect(buf, auth_what)) {
                  
                    char *scheme, *realm;
                    scheme = cut_token(buf, " ");
                    realm = cut_token(scheme, " ");
                    if ( scheme == NULL || realm == NULL ) {
                        g_debug("Invalid format of %s field.", auth_what);
                        return START_ERROR;        
                    }
                   
                    if (expect(scheme, "basic")) {
                        proxy_auth_type = PROXY_AUTH_BASIC;
                    } else {
                        g_debug("Unsupported authentication type: %s", scheme);
                    }
                }
            } while (strcmp(buf,"\r\n") != 0);
            if ( proxy_auth_type == PROXY_AUTH_NONE ) {
                g_debug("Can't find %s in response header.", auth_what);
                return START_ERROR;
            } else {
                return START_RETRY;
            }

        default:
            g_debug("http proxy is not allowed.\n");
            return START_ERROR;
    }
  
    do {
        if ( line_input(s, buf, sizeof(buf) ) ) {
            g_debug("Can't skip response headers\n");
            return START_ERROR;
        }
    } while ( strcmp(buf,"\r\n") != 0 );

    return START_OK;
}

void  switch_ns (struct sockaddr_in *ns)
{
    res_init();
    memcpy (&_res.nsaddr_list[0], ns, sizeof(*ns));
    _res.nscount = 1;
    g_debug("Using nameserver at %s\n", inet_ntoa(ns->sin_addr));
}

void set_relay(int method, const char * server , int port)
{
    if ( server == NULL || port < 0 )
    {
        if ( method != METHOD_DIRECT)
        {
            g_error("invalid proxy server and port...(%s,%d)", __FILE__, __LINE__);
        }
        
        else
        {
            g_debug("did not use proxy for transfering data...(%s,%d)",__FILE__, __LINE__);
            return ;
        }
    }
    relay_method = method;
    relay_host = strdup(server);
    relay_port = port;
}

int get_authenticated_socket( const char * host, int port)
{
    int remote = 0 ;
    int ret;

    dest_host = strdup(host);
    dest_port = port;
    
    if (0 < connect_timeout)
        set_timeout (connect_timeout);
    //if (check_direct(dest_host))
    //   relay_method = METHOD_DIRECT;
  
    if ( relay_method == METHOD_DIRECT ) {
        remote = open_connection (dest_host, dest_port);
        if ( remote == SOCKET_ERROR )
            g_error( "Unable to connect to destination host, errno=%d\n",
                     socket_errno());
    } else {
        remote = open_connection (relay_host, relay_port);
        if ( remote == SOCKET_ERROR )
            g_error( "Unable to connect to relay host, errno=%d\n",
                     socket_errno());
    }

    if (socks_ns.sin_addr.s_addr != 0)
        switch_ns (&socks_ns);

    switch ( relay_method ) {
        case METHOD_HTTP:
            ret = begin_http_relay(remote);
            switch (ret) {
                case START_ERROR:
                    close (remote);
                    g_error("failed to begin relaying via HTTP.\n");
                case START_OK:
                    break;
                case START_RETRY:
                    
                    close (remote);
            }
            break;
        case METHOD_DIRECT:
            g_debug("Did not using proxy bypass ...(%s,%d)",__FILE__, __LINE__);
            break;
    }
    return remote;
}

