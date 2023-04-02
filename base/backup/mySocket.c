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

#include "myDebug.h"

#define SER_PORT    8888
#define SER_IP		"0.0.0.0"
#define BACKLOG		13

typedef struct socket_information{
	int		 fd;
	char	*ip;
	int		 port;
} sock_infor;

/*  
int server_init(sock_infor *serv_infor_t, int backlog)
{
	int 					rv = -1, on = 1;
	struct sockaddr_in	 	ser_addr;

	if( ( serv_infor_t->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		printf("get socket failure :%s\n", strerror(errno));
		return -51;
	}
	printf("socket successfully!\n");

	setsockopt( serv_infor_t->fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	memset(&ser_addr, 0, sizeof(ser_addr));
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_port = htons( serv_infor_t->port);
	inet_aton( (*serv_infor_t).ip, &ser_addr.sin_addr);

	if( bind( (*serv_infor_t).fd, (struct sockaddr *)&ser_addr, sizeof(ser_addr))< 0)
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

	printf("The server has been successfully initialized!\n");
	rv = 0;

Exit1:
	if( rv<0 )
		close( (*serv_infor_t).fd );
	else
		rv = 0;

	return rv;
}*/

int client_init(sock_infor *cli_infor_t)
{
	int 					rv = 0;
	struct sockaddr_in	 	cli_addr;

	if( ( cli_infor_t->fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		printf("get socket failure :%s\n", strerror(errno));
		return -51;
	}
	printf("socket successfully!\n");

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

	printf("%d\n", cli_infor_t->fd);
	printf("The client has been successfully initialized!\n");

Exit1:
	if( rv<0 )
		close( cli_infor_t->fd );
	else
		rv = 0;

	return rv;
}



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







