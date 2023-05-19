#include <stdio.h>
#include "udpcom.h"

#define COMM_TIMEOUT            500     //500ms

#define PACKET_BUF_SIZE                 512

#define COMM_STATUS_OK            0
#define COMM_STATUS_ERR           -1

static int udpserver_build_send_packet(char *send_buf, int buf_len)
{
    static unsigned int send_count = 0;
    int send_len;

    send_count++;
    send_len = sprintf(send_buf, "udp server send packet: %u\n", send_count);
    return (send_len+1);  /* include terminating null character */
}

static int udpserver_transaction(char *recv_packet, int recv_buf_size, int *recv_len, int sockfd,int timeout)
{
    struct sockaddr_in srcaddr;
    char send_packet_buf[PACKET_BUF_SIZE];
    int send_len;
	
    if (sockfd == INVALID_SOCKET)
        return COMM_STATUS_ERR;

    //read packet
	*recv_len = udp_read_sock(sockfd, recv_packet, recv_buf_size, timeout, &srcaddr);
    if (*recv_len <= 0)
         return COMM_STATUS_ERR;

    if ((send_len =udpserver_build_send_packet(send_packet_buf, sizeof(send_packet_buf))) == 0)
        return COMM_STATUS_ERR;
    if (!udp_server_send_sock(sockfd, send_packet_buf, send_len, &srcaddr))
        return COMM_STATUS_ERR;
    fprintf(stderr, "udp server send packet: %s\n", send_packet_buf);
       
    return COMM_STATUS_OK;
}

static int udpserver_run(unsigned short server_port)
{
    int ret, send_len, recv_len = 0;
    char recv_packet_buf[PACKET_BUF_SIZE];
    int server_socket = INVALID_SOCKET;

    server_socket = udp_server_init(server_port);
    if (server_socket == INVALID_SOCKET)
    {
        return COMM_STATUS_ERR;
    }

    while(1)
    {       
        ret = udpserver_transaction(recv_packet_buf, sizeof(recv_packet_buf), &recv_len, server_socket, COMM_TIMEOUT);
        if (ret == COMM_STATUS_OK)
        {
            fprintf(stderr, "udp server recv packet: %s\n", recv_packet_buf);
        }
    }
    return COMM_STATUS_OK;
}


int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "usage: udp_server_test server_port\n");
        return -1;
    }

    udpserver_run((unsigned short)atoi(argv[1]));
    return 0;
}

