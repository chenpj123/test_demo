#include <stdio.h>
#include <pthread.h>
#include "netlib.h"

#define COMM_TIMEOUT            500     //500ms

#define PACKET_BUF_SIZE                 512

#define COMM_STATUS_OK            0
#define COMM_STATUS_ERR           -1

int sys_create_thread(int (*thread_func)(void *),void *arg)
{
    pthread_t cur_thread;
    int ret = pthread_create(&cur_thread,NULL,(void *)thread_func,arg);
    pthread_detach(cur_thread);
    return ret;
}

static int tcpserverdemo_build_send_packet(char *send_buf, int buf_len)
{
    static unsigned int send_count = 0;
    int send_len;

    send_count++;
    send_len = sprintf(send_buf, "tcp server send packet: %u\n", send_count);
    return (send_len+1);  /* include terminating null character */
}

static int tcpserverdemo_transaction(char *recv_packet, int recv_buf_size, int *recv_len, 
                                     int sockfd,int timeout)
{
    char send_packet_buf[PACKET_BUF_SIZE];
    int send_len;
    
    if (sockfd == COMM_STATUS_ERR)
        return COMM_STATUS_ERR;
       
    //read packet
    if (!read_sock(sockfd, (char*)recv_len, 4, timeout)) /* ³¤¶È×Ö¶Î */
         return COMM_STATUS_ERR;
 
    if (*recv_len > recv_buf_size)
        return COMM_STATUS_ERR;
    if (!read_sock(sockfd,recv_packet, *recv_len, timeout))
         return COMM_STATUS_ERR;

    fprintf(stderr, "tcp server recv packet: %s\n", recv_packet);

    if ((send_len =tcpserverdemo_build_send_packet(send_packet_buf, sizeof(send_packet_buf))) == 0)
        return COMM_STATUS_ERR;
    
    if (!send_sock(sockfd, (char*)&send_len, 4))
        return COMM_STATUS_ERR;
    if (!send_sock(sockfd, send_packet_buf, send_len))
        return COMM_STATUS_ERR;

    fprintf(stderr, "tcp server send packet: %s\n", send_packet_buf);        
    return COMM_STATUS_OK;
}

static int tcp_server_transaction(void *arg)
{
    int cli_sock = (int)arg;
    int ret, recv_len = 0;
    char recv_packet_buf[PACKET_BUF_SIZE];

    while(1)
    {
        if ( (wait_sock(cli_sock)) <= 0)
        {
            closesocket(cli_sock);
            break;
        }
        
        ret = tcpserverdemo_transaction(recv_packet_buf, sizeof(recv_packet_buf), &recv_len, cli_sock, COMM_TIMEOUT);
        if (ret != COMM_STATUS_OK)
        {
            closesocket(cli_sock);
            break;
        }
   }

    return 0;
}

static int tcp_server_thread(void *arg)
{
    int cli_sock;
    int sockfd = (int)arg;
    struct sockaddr_in from;
    int addr_size=sizeof(struct sockaddr_in);
    
    while(1)
    {
        if(( cli_sock = accept(sockfd,(struct sockaddr *)&from,(socklen_t*)&addr_size)) == INVALID_SOCKET)
        {
            fprintf(stderr, "!!!!!!tcp_server_thread accept failed!!!!!!!!\n");
            break;  /* listen socket exit or error. */
        }
        else
        {
            sys_create_thread(tcp_server_transaction,(void *)cli_sock);
        }
    }
    return 0;
}

static int tcpserver_run(unsigned short listen_port)
{
    int listen_sockfd=0 ;
    
    listen_sockfd = tcplink_server_init(listen_port);
    if (listen_sockfd == INVALID_SOCKET)
        return -1;

    sys_create_thread(tcp_server_thread,(void *)listen_sockfd);
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "usage: tcp_client_test listen_port\n");
        return -1;
    }

    tcpserver_run((unsigned short)atoi(argv[1]));
    while(1)
        sleep(1);
    return 0;
}
