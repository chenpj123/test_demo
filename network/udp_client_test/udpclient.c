#include <stdio.h>
#include <unistd.h>
#include "udpcom.h"

#define COMM_TIMEOUT            500     //500ms

#define PACKET_BUF_SIZE                 512

#define COMM_STATUS_OK            0
#define COMM_STATUS_ERR           -1

static int udpclient_build_send_packet(char *send_buf, int buf_len)
{
    static unsigned int send_count = 0;
    int send_len;

    send_count++;
    send_len = sprintf(send_buf, "udp client send packet: %u\n", send_count);
    return (send_len+1);  /* include terminating null character */
}

static int udpclient_transaction(char *send_packet, int send_len, char *recv_packet, int recv_buf_size, int *recv_len, 
                                     int sockfd,int timeout)
{
    if (sockfd == INVALID_SOCKET)
        return COMM_STATUS_ERR;
    
    if (!udp_client_send_sock(sockfd, send_packet, send_len))
        return COMM_STATUS_ERR;

    fprintf(stderr, "udp client send packet: %s\n", send_packet);

    //read packet
	*recv_len = udp_read_sock(sockfd, recv_packet, recv_buf_size, timeout, NULL);
    if (*recv_len <= 0)
         return COMM_STATUS_ERR;
        
    return COMM_STATUS_OK;
}

static int udpclient_run(const char *server_ip, unsigned short server_port)
{
    int ret, send_len, recv_len = 0;
    char send_packet_buf[PACKET_BUF_SIZE];
    char recv_packet_buf[PACKET_BUF_SIZE];
    int sockfd = INVALID_SOCKET;

    while(1)
    {
        if (sockfd == INVALID_SOCKET)
        {
            sockfd = udp_client_connect(server_ip, server_port);
        }

        if (sockfd != INVALID_SOCKET)
        {
            if ((send_len =udpclient_build_send_packet(send_packet_buf, sizeof(send_packet_buf))) == 0)
                return COMM_STATUS_ERR;
        
            ret = udpclient_transaction(send_packet_buf, send_len, recv_packet_buf, sizeof(recv_packet_buf), &recv_len, sockfd, COMM_TIMEOUT);
            if (ret == COMM_STATUS_OK)
            {
                fprintf(stderr, "udp client recv packet: %s\n", recv_packet_buf);
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
        fprintf(stderr, "usage: udp_client_test server_ip server_port\n");
        return -1;
    }

    udpclient_run(argv[1], (unsigned short)atoi(argv[2]));
    return 0;
}
