#ifndef __NETLIB_H
#define __NETLIB_H

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#define INVALID_SOCKET	-1
#define SOCKET_ERROR	-1
#define closesocket close

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

int is_socket_connected(int sockfd,int timeout);
int set_socket_nonblock_mode(int sockfd,int nonblock);
int read_sock(int sockfd,char *buf,int length,int timeout);
int send_sock(int sockfd, char *buf, int len);
int wait_sock(int sockfd);
int tcplink_client_sock_init(const char *remote_ip,unsigned short remote_port,int timeout);
int tcplink_server_init(unsigned short server_port);

#ifdef __cplusplus
}
#endif  /* __cplusplus */
#endif

