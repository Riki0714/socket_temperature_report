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
#include "mySocket.h"

#define SER_PORT    8888
#define SER_IP		"0.0.0.0"
#define BACKLOG		13

/*  
typedef struct socket_information{
	int 	fd;
	char   *ip;
	int		port;
	char a;
	long long 	b;
} sock_infor;
*/
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
}
*/

int main(int argc, char *argv[])
{
	sock_infor				serv_infor_t;
	int						clifd = -1;
	struct sockaddr_in 		cli_addr;
	socklen_t				cliaddr_len;
	int						rv = -20;
	char					buf[64];
	char					buf1[64]="hello, you are client!\n";
	int 					daemon_flag = 0;

	memset(&serv_infor_t, 0, sizeof(serv_infor_t));
	printf("%d\n", sizeof(serv_infor_t));
	serv_infor_t.ip = SER_IP;
	serv_infor_t.port = SER_PORT;
	serv_infor_t.fd = -1;

	//set max open socket count
	set_socket_rlimit();

	//Initialize the server
	if( server_init(&serv_infor_t, BACKLOG) < 0)
	{
		printf("server initialization error!\n");
		return -23;
	}

	//Backgrounder
	if(daemon_flag)	daemon(0, 0);

	while(1)
	{
		printf("Start waiting and acccept new client connect...\n");

		clifd = accept(serv_infor_t.fd, (struct sockaddr *)&cli_addr, &cliaddr_len);
		if(clifd<0)
		{
			printf("accept new client failure: %s\n", strerror(errno));
			rv = -24;
			goto Exit1;
		}
		printf("Accept new client[%s:%d] with fd [%d]\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), clifd);

		memset(buf, 0, sizeof(buf));
		if( (rv=read(clifd, buf, sizeof(buf))) < 0)
		{
			printf("Read data from client socket[%d] failure: %s\n", clifd, strerror(errno));
			close(clifd);
			continue;
		}
		else if( rv == 0 )
		{
			printf("client socket[%d] disconnected\n", clifd);
			close(clifd);
			continue;
		}
		printf("read %d bytes data from client[%d] and echo it back: '%s'\n", rv, clifd, buf);
		
		if( write(clifd, buf1, rv) < 0 )
		{
			printf("Write %d bytes data back to client[%d] failure: %s\n", rv, clifd,strerror(errno));
			close(clifd);
		}
		
		sleep(1);
		close(clifd);
	}


Exit1:
	if(rv<0)
		close(serv_infor_t.fd);
	else
		rv = 0;

	return 0;
}







