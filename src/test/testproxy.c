#include <qqproxy.h>
#include <stdio.h>
#include <stdlib.h>
int main(int argc , char* argv[])
{
    if ( argc != 5 )
    {
        printf("Usage : \n $ proxyip proxyport destip destport\n");
        exit(1);
    }
    
    set_relay(METHOD_HTTP, argv[1],atoi(argv[2]));
    int sock = get_authenticated_socket( argv[3],atoi(argv[4]));
    if( sock == SOCKET_ERROR )
        g_debug("Can not connect to server bypass proxy.");
    else{
        g_debug("Succed to connect to server bypass proxy.");
    }
    return 0;
}
