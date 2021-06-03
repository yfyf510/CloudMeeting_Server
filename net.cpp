#include "unp.h"

ssize_t	Readn(int fd, void * buf, size_t size)
{
    ssize_t lefttoread = size, hasread = 0;
    char *ptr = (char *)buf;
    while(lefttoread > 0)
    {
        if((hasread = read(fd, ptr, lefttoread))<0)
        {
            if(errno == EINTR)
            {
                hasread = 0;
            }
            else
            {
                return -1;
            }
        }
        else if(hasread == 0) //eof
        {
            break;
        }
        lefttoread -= hasread;
        ptr += hasread;
    }
    return size - lefttoread;
}

ssize_t writen(int fd, const void * buf, size_t n)
{
    ssize_t lefttowrite = n, haswrite = 0;
    char *ptr = (char *)buf;
    while(lefttowrite > 0)
    {
        if((haswrite = write(fd, ptr, lefttowrite)) < 0)
        {
            if(haswrite < 0 && errno == EINTR)
            {
                haswrite = 0;
            }
            else
            {
                return -1; //error
            }
        }
        lefttowrite -= haswrite;
        ptr += haswrite;
    }
    return n;
}



char *Sock_ntop(char * str, int size ,const sockaddr * sa, socklen_t salen)
{
    switch (sa->sa_family)
    {
    case AF_INET:
        {
            struct sockaddr_in *sin = (struct sockaddr_in *) sa;
            if(inet_ntop(AF_INET, &sin->sin_addr, str, size) == NULL)
            {
                err_msg("inet_ntop error");
                return NULL;
            }
            if(ntohs(sin->sin_port) > 0)
            {
                snprintf(str + strlen(str), size  -  strlen(str), ":%d", ntohs(sin->sin_port));
            }
            return str;
        }
    case AF_INET6:
        {
            struct sockaddr_in6 *sin = (struct sockaddr_in6 *) sa;
            if(inet_ntop(AF_INET6, &sin->sin6_addr, str, size) == NULL)
            {
                err_msg("inet_ntop error");
                return NULL;
            }
            if(ntohs(sin->sin6_port) > 0)
            {
                snprintf(str + strlen(str), size -  strlen(str), ":%d", ntohs(sin->sin6_port));
            }
            return str;
        }
     default:
        return "srever error";
    }
    return NULL;
}


void Setsockopt(int fd, int level, int optname, const void * optval, socklen_t optlen)
{
    if(setsockopt(fd, level, optname, optval, optlen) < 0)
    {
        err_msg("setsockopt error");
    }
}

void Close(int fd)
{
    if(close(fd) < 0)
    {
        err_msg("Close error");
    }
}

void Listen(int fd, int backlog)
{
    if(listen(fd, backlog) < 0)
    {
        err_quit("listen error");
    }
}

int	Tcp_connect(const char * host, const char * serv)
{
    int sockfd, n;
    struct addrinfo hints, *res, *ressave;
    bzero(&hints, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if((n = getaddrinfo(host, serv, &hints, &res)) != 0)
    {
        err_quit("tcp_connect error for %s, %s: %s", host, serv, gai_strerror(n));
    }

    ressave = res;
    do
    {
        sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if(sockfd < 0) continue;
        if(connect(sockfd, res->ai_addr, res->ai_addrlen) == 0) break;
        close(sockfd);
    }while((res = res->ai_next) != NULL);

    if(res == NULL)
    {
        err_quit("tcp_connect error for %s,%s", host, serv);
    }
    freeaddrinfo(ressave);
    return sockfd;
}

int Tcp_listen(const char * host, const char * service, socklen_t * addrlen)
{
    int listenfd, n;
    const int on = 1;
    struct addrinfo hints, *res, *ressave;
    bzero(&hints, sizeof(struct addrinfo));

    hints.ai_flags = AI_PASSIVE; //设置了AI_PASSIVE标志，但没有指定主机名，那么return ipv6和ipv4通配地址
    hints.ai_family = AF_UNSPEC; //返回的是适用于指定主机名和服务名且适合任意协议族的地址
    hints.ai_socktype = SOCK_STREAM;

    char addr[MAXSOCKADDR];

    if((n = getaddrinfo(host, service, &hints, &res)) > 0)
    {
        err_quit("tcp listen error for %s %s: %s", host, service, gai_strerror(n));
    }
	ressave = res;
	do
	{
		listenfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if(listenfd < 0)
		{
			continue; //error try again
		}
        Setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
        if(bind(listenfd, res->ai_addr, res->ai_addrlen) == 0)
        {
            printf("server address: %s\n", Sock_ntop(addr, MAXSOCKADDR, res->ai_addr, res->ai_addrlen));
            break; //success
        }
        Close(listenfd);
	}while((res = res->ai_next) != NULL);
    freeaddrinfo(ressave); //free

    if(res == NULL)
    {
        err_quit("tcp listen error for %s: %s", host, service);
    }

    Listen(listenfd, LISTENQ);

    if(addrlen)
    {
        *addrlen = res->ai_addrlen;
    }

    return listenfd;
}

int Accept(int listenfd, SA * addr, socklen_t *addrlen)
{
    int n;
    for(;;)
    {
        if((n = accept(listenfd, addr, addrlen)) < 0)
        {
            if(errno == EINTR)
                continue;
            else
                err_quit("accept error");
        }
        else
        {
            return n;
        }
    }
}