#include <qqproxy.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
int main(int argc , char* argv[])
{
    if ( argc != 5  &&  argc != 3 && argc !=7 )
    {
        printf("Usage : \n $ proxyip proxyport destip destport\n");
        exit(1);
    }
    int sock = 0;
    if (argc == 5)
    {
        sock = get_authenticated_socket( argv[3],atoi(argv[4]));
    }
    else if (argc == 3)
    {
        set_relay(METHOD_DIRECT,NULL, 0, NULL,NULL);
    }
    else
    {
        set_relay(METHOD_HTTP, argv[1], atoi(argv[2]), argv[3], argv[4]);
    }
    sock =  get_authenticated_socket(argv[1], atoi(argv[2]));

    if( sock == SOCKET_ERROR )
        g_debug("Can not connect to server bypass proxy.");
    else{
        g_debug("Succed to connect to server bypass proxy.");
    }
    close(sock);
    return 0;
}
