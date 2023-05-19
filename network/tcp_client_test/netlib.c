#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "netlib.h"
#include <fcntl.h>
#include <errno.h>

int is_socket_connected(int sockfd,int timeout)
{
    fd_set writefds;
    int l_rv;
    struct timeval tv;
    int error=-1, len=sizeof(int);
    int timeout_sec,timeout_usec;
    
    timeout_sec = timeout/1000;
    timeout_usec = (timeout %1000)*1000;

    FD_ZERO(&writefds);
    FD_SET((unsigned int)sockfd ,&writefds);
    tv.tv_sec = timeout_sec; 
    tv.tv_usec = timeout_usec;
    l_rv = select(sockfd + 1,NULL,&writefds,NULL,&tv);
    if ( l_rv <= 0 )
        return 0;
    else
    {
        getsockopt(sockfd, SOL_SOCKET, SO_ERROR, (char *)&error, &len);
        if (error == 0)
            return 1;
        else 
            return 0;
    }
}

int set_socket_nonblock_mode(int sockfd,int nonblock)
{
    int ret; 
    int flag;
	
    flag = fcntl(sockfd,F_GETFL,0);
    if (nonblock)
        ret = fcntl(sockfd,F_SETFL,flag|O_NONBLOCK);
    else
        ret = fcntl(sockfd,F_SETFL,flag&(~O_NONBLOCK));

    return (ret>=0);
}

int read_sock(int sockfd,char *buf,int length,int timeout)
{
    int nbytes =0,pos =0;
    fd_set rfds;
    struct timeval tv;
    int retval;
    int timeout_sec,timeout_usec;
    timeout_sec = timeout/1000;
    timeout_usec = (timeout %1000)*1000;

    FD_ZERO(&rfds);
    FD_SET((unsigned)sockfd, &rfds);
    tv.tv_sec = timeout_sec;
    tv.tv_usec = timeout_usec;

    while(1)
    {
        tv.tv_sec = timeout_sec;        
        tv.tv_usec = timeout_usec;
        retval = select(sockfd+1, &rfds, NULL, NULL, &tv);
        if (retval > 0)
        {
            if ((nbytes = recv(sockfd,buf+pos,length,0)) <= 0)
                return 0;
        }
        else    /* retval=0 timeout <0 error */
            return 0;
        pos += nbytes;
        if (length == nbytes)   /* read ok */
            break;
        length -= nbytes;
    }
    return 1;
}

int send_sock_timeout(int sockfd, char *buf, int len, int timeout)
{
    int total = 0;        
    int bytesleft = len;
    int n;
    int retval;
    fd_set wfds;
    struct timeval tv;
    
    int timeout_sec,timeout_usec;
    timeout_sec = timeout/1000;
    timeout_usec = (timeout %1000)*1000;
    
    FD_ZERO(&wfds);
    FD_SET((unsigned)sockfd, &wfds);
    tv.tv_sec = timeout_sec;
    tv.tv_usec = timeout_usec;
    
    while(total < len)
    {
        tv.tv_sec = timeout_sec;        
        tv.tv_usec = timeout_usec;
        retval = select(sockfd+1, NULL, &wfds, NULL, &tv);
        if (retval > 0)
        {
            n = send(sockfd, buf+total, bytesleft, MSG_NOSIGNAL);
            if (n < 0) 
                return 0;
        }
        else
            return 0;
        total += n;
        bytesleft -= n;
    }
    return 1;
}

int send_sock(int sockfd, char *buf, int len)
{
    return send_sock_timeout(sockfd,buf,len,5000);
}

int wait_sock(int sockfd)
{
    fd_set rfds;

    FD_ZERO(&rfds);
    FD_SET((unsigned)sockfd, &rfds);

    return select(sockfd+1, &rfds, NULL, NULL, NULL);
}

int tcplink_client_sock_init(const char *remote_ip,unsigned short remote_port,int timeout)
{
    int sock;
    struct sockaddr_in remote_addr;
    int ret;

    sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

    if (sock != INVALID_SOCKET) 
    {
        remote_addr.sin_family = AF_INET;
        remote_addr.sin_addr.s_addr = inet_addr(remote_ip);
        remote_addr.sin_port = htons(remote_port);
    
        set_socket_nonblock_mode(sock,1);
        ret = connect(sock,(struct sockaddr *)&remote_addr,sizeof(struct sockaddr));
        if (ret < 0 && errno != EINPROGRESS)
        {
            closesocket(sock);
            return -1;
        }

        if (is_socket_connected(sock,timeout))
        {
            set_socket_nonblock_mode(sock,0);
            return sock;
        }
        else
        {
            closesocket(sock); 
            return -1;
        }
        
    }
    return 0;
}

int tcplink_server_init(unsigned short server_port)
{
	struct sockaddr_in local_addr;
	int reuse_port = 1;
	int sockfd;

	sockfd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if (sockfd == INVALID_SOCKET)
	{
		return 0;
	}
    
	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	local_addr.sin_port = htons(server_port);

	if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(const char *)&reuse_port,sizeof(int)) == SOCKET_ERROR)
	{
		closesocket(sockfd);
		return 0;
	}
	
	if (SOCKET_ERROR == bind(sockfd,(struct sockaddr *)&local_addr,sizeof(struct sockaddr_in)))
	{
		closesocket(sockfd);
		return 0;
	}

	listen(sockfd,5);
    return sockfd;    
}
