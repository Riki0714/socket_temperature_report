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
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/signal.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "debug.h"
#include "socket.h"
#include "packet.h"


#define SO_NOSIGPIPE	0x1022
#define	IP_LEN			32
#define BACKLOG			13
#define PACK_BUF_LEN	64	

//char g_dns_ipBuf[10][32];

int socket_init(socket_t *sock, char *host, int port)
{
	sock->fd = -1;
	strcpy(sock->host, host);
	sock->port = port;
	sock->connected = 0;  
	
	return 0;
}

int socket_close(socket_t *sock)
{
	close(sock->fd);
	sock->fd = -1;

	sock->connected = 0;

	return 0;
}

int socket_diag(socket_t *sock)
{
	struct tcp_info 	info;
	int					size=sizeof(info);

	if(sock->fd <= 0 )
	{
		return -1;
	}

	getsockopt(sock->fd, IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&size);
	if( (info.tcpi_state == 1) )
	{
		sock->connected = 1;
		printf("socket connected\n");
		return 1;
	}
	else
	{
		sock->connected = 0;
		printf("disconnected\n");
	}

	return 0;
}


int socket_write(socket_t *sock, char *data, int bytes)
{
	char	*str_tmp = data;
	int		rv = -1;
	int		len = 0;
	int		tobesend_bytes = bytes;
	
	while( tobesend_bytes>0 )
	{
		len = tobesend_bytes>512 ? 512 : tobesend_bytes;

		rv = write(sock->fd, str_tmp, tobesend_bytes);
		if( rv<0 )
		{
			if( errno==EINTR )  
			{
				continue;
			}
			else
			{
				printf("write %d bytes data back to client[%d] failure: %s\n", bytes, sock->fd, strerror(errno));
			//	close(sock->fd);
				return -1;
			}
		}

		tobesend_bytes -= rv;
		str_tmp += rv;
	}

	return 0;
}

int socket_send_packet(socket_t *sock, packet_t *pack)
{
	char	pack_buf[PACK_BUF_LEN]={0};

	pack_data(pack, pack_buf, PACK_BUF_LEN);
	if( socket_write(sock, pack_buf, PACK_BUF_LEN) < 0 )
	{
		printf("write data to server failure\n");
		return -1;
	}

	return 0;
}
/*
int socket_read(socket_t *sock, char *data, int bytes)
{
	int rv=0;

	memset(data, 0, bytes);
	if( (rv=read(sock->fd, data, bytes)) < 0  )
	{
		printf("read data from server[%d] failure: %s\n", sock->fd, strerror(errno));
		return -1;
		//close(sock->fd);
	}
	else if( rv==0 )
	{
		printf("the connection to the server[%d] is disconnected\n", sock->fd);
		return -2;
		//close(sock->fd);
	}
	else
	{
		printf("read %d bytes data from server[%d] and echo it back: '%s'\n", rv, sock->fd, data);
	}

	return 0;
}
*/
int socket_keepalive(socket_t *sock, int keep_idle, int keep_intv, int keep_count)
{
	int						value = 1;
	int						keepAlive = 1;
	int						keepIdle = 60;
	int						keepInterval = 5;
	int						keepCount = 3;

	if( sock->fd <= 0 )
	{
		return -1;
	}

	if( setsockopt( sock->fd, SOL_SOCKET, SO_KEEPALIVE, &value, sizeof(value)) )
	{
		printf("set SOL_SOCKET failure: %s\n", strerror(errno));
		return -2;
	}
	if( setsockopt( sock->fd, SOL_TCP, TCP_KEEPIDLE, (void *)&keep_idle, sizeof(keep_idle)) )
	{
		printf("set TCP_KEEPIDLE failure: %s\n", strerror(errno));
		return -3;
	}
	if( setsockopt( sock->fd, SOL_TCP, TCP_KEEPINTVL, (void *)&keep_intv, sizeof(keep_intv)) )
	{
		printf("set TCP_KEEPINTVL failure: %s\n", strerror(errno));
		return -4;
	}
	if( setsockopt( sock->fd, SOL_TCP, TCP_KEEPCNT, (void *)&keep_count, sizeof(keep_count)) )
	{
		printf("set TCP_KEEPCNT failure: %s\n", strerror(errno));
		return -5;
	}

	return 0;
}


int socket_listen(socket_t *sock)
{
	int 					rv = -1;
	int 					on = 1;
	int						value = 1;

	struct sockaddr_in	 	ser_addr;

	if( ( sock->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		printf("get socket failure :%s\n", strerror(errno));
		return -1;
	}

	memset(&ser_addr, 0, sizeof(ser_addr));
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons( sock->port);
	inet_aton( sock->host, &(ser_addr.sin_addr));
 
	socket_keepalive(sock, 60, 5, 3);
	setsockopt(sock->fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
	setsockopt(sock->fd, SOL_SOCKET, SO_NOSIGPIPE, &value, sizeof(value));

	if( bind( sock->fd, (struct sockaddr *)&ser_addr, sizeof(struct sockaddr_in))< 0)
	{
		printf("bind failure: %s\n", strerror(errno));
		socket_close(sock);
		return -2;
	}

	if( listen( sock->fd, BACKLOG) < 0)
	{
		printf("listen failure: %s\n", strerror(errno));
		socket_close(sock);
		return -3;
	}

	return 0;
}

int socket_connect(socket_t *sock)
{
	int 					rv = 0;
	int						value = 1;
	int     	            flags = NI_NUMERICHOST;
	char					ip[IP_LEN] = {0};
	struct addrinfo 	    hints; 
	struct addrinfo			*listp;
	struct addrinfo			*p; 
	struct sockaddr_in	 	cli_addr;
	in_addr_t				addr = inet_addr(sock->host);

	memset(&cli_addr, 0, sizeof(cli_addr));
	cli_addr.sin_family = AF_INET;
	cli_addr.sin_port = htons( sock->port);

	if( addr >= 0 )
	{
		if( ( sock->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
		{
			printf("get socket failure :%s\n", strerror(errno));
			return -1;
		}

		inet_aton( sock->host, &(cli_addr.sin_addr) );
		socket_keepalive(sock, 60, 5, 3);
		setsockopt(sock->fd, SOL_SOCKET, SO_NOSIGPIPE, &value, sizeof(value));

		if( connect( sock->fd, (struct sockaddr *)&cli_addr, sizeof(cli_addr))< 0)
		{
			//printf("connect failure!\n");
			socket_close(sock);
			return -2;
		}
	}
	else  //dns
	{
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET; //AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;  //0;
		hints.ai_protocol = IPPROTO_TCP;  //0;
		hints.ai_flags = 0;

		if( ( rv=(getaddrinfo(sock->host, NULL, &hints, &listp)) ) != 0 ) 
		{   
			printf("getaddrinfo error: %s", gai_strerror(rv));
			return -3;
		}

		for( p=listp; p!=NULL; p=p->ai_next )
		{	
			//memset(sock->host, 0, IP_LEN);
			//getnameinfo(p->ai_addr, p->ai_addrlen, new_ip, IP_LEN, NULL, 0, flags);
			//strncpy(cli_addr.sin_addr, new_ip, IP_LEN);

			if( ( sock->fd = socket(p->ai_family, p->ai_socktype, 0)) < 0 )
			{
				printf("get socket failure :%s\n", strerror(errno));
				continue;
			}
			socket_keepalive(sock, 60, 5, 3);
			setsockopt(sock->fd, SOL_SOCKET, SO_NOSIGPIPE, &value, sizeof(value));

			if( connect( sock->fd, p->ai_addr, sizeof(cli_addr))< 0)
			{
				printf("connect failure!\n");
				socket_close(sock);
				continue;
			}
			else
			{
				sock->connected = 1;
				break;
			}
		}

		if( sock->connected == 0 )  
		{
			printf("Faild to connect to the server\n");
			//socket_close(sock);
			return -4;
		}

		freeaddrinfo(listp);
	}

	return 0;
}

void set_socket_rlimit(void)
{
	struct rlimit limit = {0};

	getrlimit(RLIMIT_NOFILE, &limit);
	limit.rlim_cur = limit.rlim_max;
	setrlimit(RLIMIT_NOFILE, &limit);

	printf("set socket open fd max count to %d\n", limit.rlim_max);
}

/*  
void socket_dns(char *doname)
{
	struct addrinfo     hints; 
	struct addrinfo		*listp;
	struct addrinfo		*p; 
	char				ip[IP_LEN]={0};
	int                 rv=-1;
	int					i=0;
	int                 flags = NI_NUMERICHOST;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = 0;
	hints.ai_protocol = 0;
	hints.ai_flags = 0;

	if( ( rv=(getaddrinfo(doname, NULL, &hints, &listp)) ) != 0 ) 
	{   
		printf("getaddrinfo error: %s", gai_strerror(rv));
	}   

	memset(g_dns_ipBuf, 0, sizeof(g_dns_ipBuf));
	for( p=listp; p!=NULL; p=p->ai_next )
	{   
		memset(ip, 0, sizeof(ip));
		getnameinfo(p->ai_addr, p->ai_addrlen, ip, IP_LEN, NULL, 0, flags);
		strncpy(g_dns_ipBuf[i], ip, IP_LEN);
		i++;
	}   

	freeaddrinfo(listp);
}
*/

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

