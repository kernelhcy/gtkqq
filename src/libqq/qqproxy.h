
#ifndef __QQ_PROXY_H__
#define __QQ_PROXY_H__
#include <glib.h>
#include <netinet/in.h>
typedef struct {
    int num;
    const char *str;
} LOOKUP_ITEM;

#define SOCKET int

#define METHOD_UNDECIDED 0
#define METHOD_DIRECT    1
#define METHOD_SOCKS     2
#define METHOD_HTTP      3
#define METHOD_TELNET    4


#define RESOLVE_UNKNOWN 0
#define RESOLVE_LOCAL   1
#define RESOLVE_REMOTE  2
#define RESOLVE_BOTH    3

#define LOCAL_STDIO     0
#define LOCAL_SOCKET    1

#define WITH_RESOLVER 1

#define ENV_SOCKS_SERVER  "SOCKS_SERVER"        /* SOCKS server */
#define ENV_SOCKS5_SERVER "SOCKS5_SERVER"
#define ENV_SOCKS4_SERVER "SOCKS4_SERVER"

#define ENV_SOCKS_RESOLVE  "SOCKS_RESOLVE"      /* resolve method */
#define ENV_SOCKS5_RESOLVE "SOCKS5_RESOLVE"
#define ENV_SOCKS4_RESOLVE "SOCKS4_RESOLVE"

#define ENV_SOCKS5_USER     "SOCKS5_USER"       /* auth user for SOCKS5 */
#define ENV_SOCKS4_USER     "SOCKS4_USER"       /* auth user for SOCKS4 */
#define ENV_SOCKS_USER      "SOCKS_USER"        /* auth user for SOCKS */
#define ENV_SOCKS5_PASSWD   "SOCKS5_PASSWD"     /* auth password for SOCKS5 */
#define ENV_SOCKS5_PASSWORD "SOCKS5_PASSWORD"   /* old style */

#define ENV_HTTP_PROXY          "HTTP_PROXY"    /* common env var */
#define ENV_HTTP_PROXY_USER     "HTTP_PROXY_USER" /* auth user */
#define ENV_HTTP_PROXY_PASSWORD "HTTP_PROXY_PASSWORD" /* auth password */

#define ENV_TELNET_PROXY          "TELNET_PROXY"    /* common env var */

#define ENV_CONNECT_USER     "CONNECT_USER"     /* default auth user name */
#define ENV_CONNECT_PASSWORD "CONNECT_PASSWORD" /* default auth password */

#define ENV_SOCKS_DIRECT   "SOCKS_DIRECT"       /* addr-list for non-proxy */
#define ENV_SOCKS5_DIRECT  "SOCKS5_DIRECT"
#define ENV_SOCKS4_DIRECT  "SOCKS4_DIRECT"
#define ENV_HTTP_DIRECT    "HTTP_DIRECT"
#define ENV_CONNECT_DIRECT "CONNECT_DIRECT"

#define ENV_SOCKS5_AUTH "SOCKS5_AUTH"
#define ENV_SSH_ASKPASS "SSH_ASKPASS"           /* askpass program */

/* Prefix string of HTTP_PROXY */
#define HTTP_PROXY_PREFIX "http://"
#define PROXY_AUTH_NONE 0
#define PROXY_AUTH_BASIC 1
#define PROXY_AUTH_DIGEST 2

#define REASON_UNK              -2
#define REASON_ERROR            -1
#define REASON_CLOSED_BY_LOCAL  0
#define REASON_CLOSED_BY_REMOTE 1

/* return value of relay start function. */
#define START_ERROR -1
#define START_OK     0
#define START_RETRY  1

#ifndef SOCKET_ERROR
#define SOCKET_ERROR -1
#endif

#define MAX_DIRECT_ADDR_LIST 256

void fatal( const char *fmt, ... );
void *xmalloc (size_t size);
char * downcase( char *str );

char * expand_host_and_port (const char *fmt, const char *host, int port);
int lookup_resolve( const char *str );

char *getusername(void);
int expect( char *str, char *substr);

#define PARAMETER_FILE "/etc/connectrc"
#define PARAMETER_DOTFILE ".connectrc"
typedef struct {
    char* name;
    char* value;
} PARAMETER_ITEM;
PARAMETER_ITEM parameter_table[] = {
    { ENV_SOCKS_SERVER, NULL },
    { ENV_SOCKS5_SERVER, NULL },
    { ENV_SOCKS4_SERVER, NULL },
    { ENV_SOCKS_RESOLVE, NULL },
    { ENV_SOCKS5_RESOLVE, NULL },
    { ENV_SOCKS4_RESOLVE, NULL },
    { ENV_SOCKS5_USER, NULL },
    { ENV_SOCKS5_PASSWD, NULL },
    { ENV_SOCKS5_PASSWORD, NULL },
    { ENV_HTTP_PROXY, NULL },
    { ENV_HTTP_PROXY_USER, NULL },
    { ENV_HTTP_PROXY_PASSWORD, NULL },
    { ENV_CONNECT_USER, NULL },
    { ENV_CONNECT_PASSWORD, NULL },
    { ENV_SSH_ASKPASS, NULL },
    { ENV_SOCKS5_DIRECT, NULL },
    { ENV_SOCKS4_DIRECT, NULL },
    { ENV_SOCKS_DIRECT, NULL },
    { ENV_HTTP_DIRECT, NULL },
    { ENV_CONNECT_DIRECT, NULL },
    { ENV_SOCKS5_AUTH, NULL },
    { NULL, NULL }
};


struct ADDRPAIR {
    struct in_addr addr;
    struct in_addr mask;
    char *name;
    int negative;
};
#define socket_errno() (errno)

PARAMETER_ITEM* find_parameter_item(const char* name);
void set_timeout(int timeout);
void sig_timeout(void);

void mask_addr (void *addr, void *mask, int addrlen);

int cmp_addr (void *addr1, void *addr2, int addrlen);
int domain_match(const char *s1, const char *s2);
void mask_addr (void *addr, void *mask, int addrlen);

int is_direct_name (const char *name);

int is_direct_address (const struct in_addr addr);

int check_direct(const char *host);

int local_resolve (const char *host, struct sockaddr_in *addr);
int open_connection( const char *host, short port );

int atomic_out( SOCKET s, char *buf, int size );
int atomic_in( SOCKET s, char *buf, int size );
int line_input( SOCKET s, char *buf, int size );
char * cut_token( char *str, char *delim);

char *make_base64_string(const char *str);
int basic_auth (SOCKET s);


int begin_http_relay( SOCKET s );

void set_relay(int method, const char * server, int port);

int get_authenticated_socket(const char * host, int port);

#endif
