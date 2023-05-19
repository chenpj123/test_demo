#include <stdio.h>
#include "udpcom.h"

int udp_client_connect(const char *destip,unsigned short destport)
{
    struct sockaddr_in server_addr;
    int sock;
    
    sock = socket(AF_INET,SOCK_DGRAM,0);
    if (sock == INVALID_SOCKET)
    {
        fprintf(stderr,"create udp sock error!\n");
        return -1;
    }
    
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(destip);
    server_addr.sin_port = htons(destport);
    
    if (connect(sock,(struct sockaddr *)&server_addr,sizeof(struct sockaddr_in))==SOCKET_ERROR)
    {
        fprintf(stderr,"can't connect to server!\n");
        closesocket(sock);
        return -1;
    }

    return sock;
}

int udp_server_init(unsigned short server_port)
{
    struct sockaddr_in local_addr;
    int reuse_port = 1;
    int sock;

    sock = socket(AF_INET,SOCK_DGRAM,0);
    if (sock == INVALID_SOCKET)
    {
        fprintf(stderr,"create udp sock error!\n");
        return -1;
    }
    
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    local_addr.sin_port = htons(server_port);
   
    if (SOCKET_ERROR == bind(sock,(struct sockaddr *)&local_addr,sizeof(struct sockaddr_in)))
    {
        fprintf(stderr,"bind failed!\n");
        closesocket(sock);
        return -1;
    }

    return sock; 
}

int udp_client_send_sock(int sockfd, char *buf, int len)
{
    int n;

    n = send(sockfd, buf, len, MSG_NOSIGNAL);
    if (n != len)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

int udp_server_send_sock(int sockfd, char *buf, int len, struct sockaddr_in *destaddr)
{
    int n;

    n = sendto(sockfd, buf, len, MSG_NOSIGNAL, (struct sockaddr *)destaddr, sizeof(struct sockaddr_in));
    if (n != len)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

int udp_read_sock(int sockfd, char *buf, int length, int timeout, struct sockaddr_in *srcaddr)
{
    int nbytes = 0;
    fd_set rfds;
    struct timeval tv;
    int retval;

    FD_ZERO(&rfds);
    FD_SET((unsigned)sockfd, &rfds);
    tv.tv_sec = timeout/1000;
    tv.tv_usec = (timeout %1000)*1000;

    retval = select(sockfd+1, &rfds, NULL, NULL, &tv);
    if (retval > 0)
    {
        if (srcaddr == NULL)
        {
            nbytes = recv(sockfd, buf, length, 0);
        }
        else
        {
            int len = sizeof(struct sockaddr_in);
            nbytes = recvfrom(sockfd, buf, length, 0, (struct sockaddr *)srcaddr, &len);
        }

        if (nbytes <= 0)
        {
            return -1;
        }
        else
        {
            return nbytes;
        }
    }
    else    /* retval=0 timeout <0 error */
    {
        return 0;
    }
}

