/*********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  server.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(03/04/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "03/04/23 07:22:53"
 *                 
 ********************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <netinet/tcp.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "debug.h"
#include "socket.h"

//#define SER_PORT    8888
//#define SER_IP		"0.0.0.0"
//#define BACKLOG		13

/*
typedef struct socket_information{
	int		 fd;
	char	*ip;
	int		 port;
} sock_infor;
*/

#define SO_NOSIGPIPE   0x1022




int socket_bind(int fd, const struct sockaddr *addr, int len)
{
	if( bind(fd, addr, len) < 0 )
	{
		printf("bind failure: %s\n", strerror(errno));
		close(fd);

		return -1;
	}
	
	return 0;
}

int socket_listen(int fd, int backlog)
{
	if( listen(fd, backlog) < 0 )
	{
		printf("listen failure: %s\n", strerror(errno));
		close(fd);

		return -1;
	}

	return 0;
}

int socket_connect(int fd, const struct sockaddr *addr, int len)
{
	if( connect(fd, addr, len) < 0)
	{
		printf("connect failure: %s\n", strerror(errno));
		close(fd);

		return -1;
	}

	return 0;
}


int socket_write(int fd, char *buf, int len)
{
	if(  write(fd, buf, len) < 0  )
	{
		printf("write %d bytes data back to client[%d] failure: %s\n", len, fd, strerror(errno));
		close(fd);
		return -1;
	}

	return 0;
}


int socket_read(int fd, char *buf, int len)
{
	int rv=0;

	memset(buf, 0, len);
	if( (rv=read(fd, buf, len)) < 0  )
	{
		printf("read data from server[%d] failure: %s\n", fd, strerror(errno));
		close(fd);
	}
	else if( rv==0 )
	{
		printf("the connection to the server[%d] is disconnected\n", fd);
		close(fd);
	}
	else
	{
		printf("read %d bytes data from server[%d] and echo it back: '%s'\n", rv, fd, buf);
	}

	return rv;
}


int server_init(sock_infor *serv_infor_t, int backlog)
{
	int 					rv = -1, on = 1;
	int						value = 1;
	int						keepAlive = 1;
	int						keepIdle = 60;
	int						keepInterval = 5;
	int						keepCount = 3;

	struct sockaddr_in	 	ser_addr;

	if( ( serv_infor_t->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		printf("get socket failure :%s\n", strerror(errno));
		return -51;
	}
	printf("socket successfully!\n");

	setsockopt( serv_infor_t->fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	setsockopt( serv_infor_t->fd, SOL_SOCKET, SO_NOSIGPIPE, &value, sizeof(value));
	setsockopt( serv_infor_t->fd, SOL_SOCKET, SO_KEEPALIVE, &value, sizeof(value));
	setsockopt( serv_infor_t->fd, SOL_TCP, 	TCP_KEEPINTVL, (void *)&keepIdle, sizeof(keepIdle));
	setsockopt( serv_infor_t->fd, SOL_TCP, 	TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval));
	setsockopt( serv_infor_t->fd, SOL_TCP, 	TCP_KEEPINTVL, (void *)&keepCount, sizeof(keepCount));


	memset(&ser_addr, 0, sizeof(ser_addr));
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons( serv_infor_t->port);
	inet_aton( (*serv_infor_t).ip, &ser_addr.sin_addr);
 
	if( bind( serv_infor_t->fd, (struct sockaddr *)&ser_addr, sizeof(ser_addr))< 0)
	{
		printf("bind failure!\n");
		rv = -52;
		goto Exit1;
	}

	if( listen( (*serv_infor_t).fd, backlog) < 0)
	{
		printf("listen failure!\n");
		rv = -53;
		goto Exit1;
	}

/*  
	if( socket_bind( serv_infor_t->fd, (struct sockaddr *)&ser_addr, sizeof(ser_addr)) < 0)
	{
		return -52;
	}
	if( listen( (*serv_infor_t).fd, backlog )  <  0 )
	{
		return -53;
	}
*/
	printf("The server has been successfully initialized!\n");
	rv = 0;

Exit1:
	if( rv<0 )
		close( (*serv_infor_t).fd );
	else
		rv = 0;

	return rv;
}

int client_init(sock_infor *cli_infor_t)
{
	int 					rv = 0;
	int						value = 1;
	int						keepAlive = 1;
	int						keepIdle = 60;
	int						keepInterval = 5;
	int						keepCount = 3;

	struct sockaddr_in	 	cli_addr;

	if( ( cli_infor_t->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		printf("get socket failure :%s\n", strerror(errno));
		return -51;
	}
	printf("socket successfully!\n");

	setsockopt( cli_infor_t->fd, SOL_SOCKET, SO_NOSIGPIPE, &value, sizeof(value));
	setsockopt( cli_infor_t->fd, SOL_SOCKET, SO_KEEPALIVE, &value, sizeof(value));
	setsockopt( cli_infor_t->fd, SOL_TCP, 	TCP_KEEPINTVL, (void *)&keepIdle, sizeof(keepIdle));
	setsockopt( cli_infor_t->fd, SOL_TCP, 	TCP_KEEPINTVL, (void *)&keepInterval, sizeof(keepInterval));
	setsockopt( cli_infor_t->fd, SOL_TCP, 	TCP_KEEPINTVL, (void *)&keepCount, sizeof(keepCount));

	memset(&cli_addr, 0, sizeof(cli_addr));
	cli_addr.sin_family = AF_INET;
	cli_addr.sin_port = htons( cli_infor_t->port);
	inet_aton( cli_infor_t->ip, &cli_addr.sin_addr);

	if( connect( cli_infor_t->fd, (struct sockaddr *)&cli_addr, sizeof(cli_addr))< 0)
	{
		printf("connect failure!\n");
		rv = -52;
		goto Exit1;
	}

//	printf("%d\n", cli_infor_t->fd);
	printf("The client has been successfully initialized!\n");

Exit1:
	if( rv<0 )
		close( cli_infor_t->fd );
	else
		rv = 0;

	return rv;
}

void set_socket_rlimit(void)
{
	struct rlimit limit = {0};

	getrlimit(RLIMIT_NOFILE, &limit);
	limit.rlim_cur = limit.rlim_max;
	setrlimit(RLIMIT_NOFILE, &limit);

	printf("set socket open fd max count to %d\n", limit.rlim_max);
}
char *socket_dns(char *doname, int size)
{
	struct addrinfo     hints; 
	struct addrinfo		*listp;
	struct addrinfo		*p; 
	char 				*ip;
	int                 rv=-1;
	int                 falgs = NI_NUMERICHOST;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = 0;
	hints.ai_protocol = 0;
	hints.ai_flags = 0;

	if( ( rv=(getaddrinfo(doname, NULL, &hints, &listp)) ) != 0 ) 
	{   
		printf("getaddrinfo error: %s", gai_strerror(rv));
	}   

	p = listp;
	{   
		getnameinfo(p->ai_addr, p->ai_addrlen, ip, size, NULL, 0, falgs);

	}   
	freeaddrinfo(listp);
	return  ip;
}


/*
int main(int argc, char *argv[])
{
	sock_infor				cli_infor_t;
	int						rv = -20;
	char					buf[64]="hello!\n";

	memset(&cli_infor_t, 0, sizeof(cli_infor_t));
	printf("%d\n", sizeof(cli_infor_t));
	cli_infor_t.ip = "127.0.0.1";
	cli_infor_t.port = 8888;
	cli_infor_t.fd = -1;

	printf("wu%d\n", cli_infor_t.fd);
	if( client_init(&cli_infor_t) < 0)
	{
		printf("client initialization error!\n");
		return -23;
	}

	printf("hei%d\n", cli_infor_t.fd);
		printf("Start Accept...\n");

		if( write(cli_infor_t.fd, buf, strlen(buf)) < 0 )
		{
			printf("Write %d bytes data back to client[%d] failure: %s\n", rv, cli_infor_t.fd, strerror(errno));
			close(cli_infor_t.fd);
		}
		
		memset(buf, 0, sizeof(buf));
		if( (rv=read(cli_infor_t.fd, buf, sizeof(buf))) < 0)
		{
			printf("Read data from client socket[%d] failure: %s\n", cli_infor_t.fd, strerror(errno));
			close(cli_infor_t.fd);
			return -1;
		}
		else if( rv == 0 )
		{
			printf("client socket[%d] disconnected\n", cli_infor_t.fd);
			close(cli_infor_t.fd);
			return -2;
		}
		printf("read %d bytes data from client[%d] and echo it back: '%s'\n", rv, cli_infor_t.fd, buf);
		

	return 0;
}
*/






