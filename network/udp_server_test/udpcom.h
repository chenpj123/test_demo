#ifndef _UDP_COM
#define _UDP_COM

#include <string.h>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

int udp_client_connect(const char *destip,unsigned short destport);
int udp_server_init(unsigned short server_port);
int udp_client_send_sock(int sockfd, char *buf, int len);
int udp_server_send_sock(int sockfd, char *buf, int len, struct sockaddr_in *destaddr);
int udp_read_sock(int sockfd, char *buf, int length, int timeout, struct sockaddr_in *srcaddr);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif
