#include <stdio.h>
#include <unistd.h>
#include "netlib.h"

#define COMM_TIMEOUT            500     //500ms

#define PACKET_BUF_SIZE                 512

#define COMM_STATUS_OK            0
#define COMM_STATUS_ERR           -1

static int tcpclient_build_send_packet(char *send_buf, int buf_len)
{
    static unsigned int send_count = 0;
    int send_len;

    send_count++;
    send_len = sprintf(send_buf, "tcp client send packet: %u\n", send_count);
    return (send_len+1);  /* include terminating null character */
}

static int tcpclient_transaction(char *send_packet, int send_len, char *recv_packet, int recv_buf_size, int *recv_len, 
                                     int sockfd,int timeout)
{
    if (sockfd == INVALID_SOCKET)
        return COMM_STATUS_ERR;
    
    if (!send_sock(sockfd, (char*)&send_len, 4)) /* length field */
        return COMM_STATUS_ERR;

    if (!send_sock(sockfd, send_packet, send_len))  /* content */
        return COMM_STATUS_ERR;

    fprintf(stderr, "tcp client send packet: %s\n", send_packet);

    //read packet
    if (!read_sock(sockfd, (char*)recv_len, 4, timeout))
         return COMM_STATUS_ERR;
 
    if (*recv_len > recv_buf_size)
        return COMM_STATUS_ERR;
    if (!read_sock(sockfd,recv_packet, *recv_len, timeout))
         return COMM_STATUS_ERR;
        
    return COMM_STATUS_OK;
}

static int tcpclient_run(const char *server_ip, unsigned short server_port)
{
    int ret, send_len, recv_len = 0;
    char send_packet_buf[PACKET_BUF_SIZE];
    char recv_packet_buf[PACKET_BUF_SIZE];
    int sockfd = INVALID_SOCKET;

    while(1)
    {
        if (sockfd == INVALID_SOCKET)
        {
            sockfd = tcplink_client_sock_init(server_ip, server_port, COMM_TIMEOUT);
        }

        if (sockfd != INVALID_SOCKET)
        {
            if ((send_len =tcpclient_build_send_packet(send_packet_buf, sizeof(send_packet_buf))) == 0)
                return COMM_STATUS_ERR;
        
            ret = tcpclient_transaction(send_packet_buf, send_len, recv_packet_buf, sizeof(recv_packet_buf), &recv_len, sockfd, COMM_TIMEOUT);
            if (ret == COMM_STATUS_OK)
            {
                fprintf(stderr, "tcp client recv packet: %s\n", recv_packet_buf);
            }
            else
            {
                closesocket(sockfd);
                sockfd = INVALID_SOCKET;
            }
        }
        else
            fprintf(stderr, "connect server(%s:%d) failed\n", server_ip, server_port);

        usleep(1000*1000);
    }
    return COMM_STATUS_OK;
}


int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "usage: tcp_client_test server_ip server_port\n");
        return -1;
    }

    tcpclient_run(argv[1], (unsigned short)atoi(argv[2]));
    return 0;
}
