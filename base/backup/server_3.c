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
#include <sys/epoll.h>
#include <getopt.h>
#include <stdlib.h>

#include "myDebug.h"
#include "mySocket.h"
#include "myDns.h"

#define SER_PORT    8888
#define SER_IP		"0.0.0.0"
#define BACKLOG		13
#define MAX_EVENTS	512

int main(int argc, char *argv[])
{
	sock_infor				serv_infor_t; //server information(ip, port...)
	int						connfd = -1;
	//struct sockaddr_in 	cli_addr;
	//socklen_t				cliaddr_len;
	int						rv = -20, i=0;
	char					buf[64];
	char					buf1[64]="hello, you are client!\n";
	int 					daemon_flag = 0;
	char					dfIp[32] = "0.0.0.0"; //Default Ip
	
	int						epollfd;
	int						events;
	struct epoll_event		event;
	struct epoll_event		event_array[MAX_EVENTS];

	int						ch = -1;
	struct option 			opts[] = {
		{"--ipaddr", optional_argument, NULL, 'i'},
		{"--daemon", optional_argument, NULL, 'd'},
		{"--port",   required_argument, NULL, 'p'},
		{"--help",   no_argument, 	    NULL, 'h'},
		{0, 0, 0, 0}    ///!!!
	}; ///!!!

	memset(&serv_infor_t, 0, sizeof(serv_infor_t));
	serv_infor_t.fd = -1;
	serv_infor_t.ip = dfIp;
	serv_infor_t.port = 6666;

	while( (ch=getopt_long(argc, argv, "i::d::p:h", opts, NULL)) != -1 )
	{
		switch( ch )
		{
			case 'p':
				serv_infor_t.port = atoi(optarg);
				break;

			case 'd':
				daemon_flag = 1;
				break;

			case 'h':
				printf("tbd...\n");
				return 0;
		}
	}

	if( !serv_infor_t.port )
	{
		printf("Please enter the port number to listen for(-p xxx)\n");
		return -2;
	}

	printf("22\n");
	//set max open socket count
	set_socket_rlimit();

	//---------------- socket server  -----------------
	//Initialize the server
	if( server_init(&serv_infor_t, BACKLOG) < 0)
	{
		printf("server initialization error!\n");
		return -23;
	}

	//Backgrounder
	if(daemon_flag)	daemon(0, 0);

	//---------------- epoll -----------------
	event.events = EPOLLIN; //Specify event type
	event.data.fd = serv_infor_t.fd;

	//1. create epoll
	if( (epollfd = epoll_create(MAX_EVENTS)) < 0 )
	{
		printf("create epoll failure: %s\n", strerror(errno));
		return -24;
	}

	//2. epoll_ctl() Modify the interest list of epoll
	if( (rv = epoll_ctl(epollfd, EPOLL_CTL_ADD, serv_infor_t.fd, &event)) < 0 )
	{
		printf("epoll add listen socket failure: %s\n", strerror(errno));
		return -25;
	}
	
	//---------------- daemon ----------------
	if(daemon_flag) 
	{
		daemon(1,1);
	}

	while(1)
	{
		printf("Start waiting and acccept new client connect...\n");

		//3. epoll_wait() Event wait
		//Return the number of elements in the event arra array
		events = epoll_wait(epollfd, event_array, MAX_EVENTS, -1);
		if(events < 0)
		{
			printf("epoll waiting faliure: %s\n", strerror(errno));
			break;
		}
		else if( 0==events )   //No descriptor is in a ready state at a given time
		{
			printf("epoll wait timeout!\n");
			continue;
		}


		for(i=0; i<events; i++)
		{
			//Error event detection
			if( (EPOLLERR & event_array[i].events) || (EPOLLHUP & event_array[i].events) )
			{
				printf("epoll_wait get error on fd[%d]: %s\n", event_array[i].data.fd, strerror(errno));
				epoll_ctl(epollfd, EPOLL_CTL_DEL, event_array[i].data.fd, NULL);
				close(event_array[i].data.fd);
			}

			//Event judgment
			if( event_array[i].data.fd == serv_infor_t.fd )
			{
				if( (connfd = accept(serv_infor_t.fd, (struct sockaddr *)NULL, NULL)) < 0 )
				{
					printf("accept new client failure: %s\n", strerror(errno));
					continue;
				}

				event.data.fd = connfd;
				event.events = EPOLLIN;
				if( epoll_ctl(epollfd, EPOLL_CTL_ADD, connfd, &event) < 0 )
				{
					printf("add client to epoll failure: %s\n", strerror(errno));
					close(event_array[i].data.fd);
					continue;
				}
				printf("epoll add new client socket[%d] successfully!\n", connfd);
			}
			else
			{
				memset(buf, 0, sizeof(buf));
				if( (rv=read(event_array[i].data.fd, buf, sizeof(buf))) < 0)
				{
					printf("Read data from client socket[%d] failure: %s\n", event_array[i].data.fd, strerror(errno));
					close(event_array[i].data.fd);
					continue;
				}
				else if( rv == 0 )
				{
					printf("client socket[%d] disconnected\n", event_array[i].data.fd);
					close(event_array[i].data.fd);
					continue;
				}
				printf("read %d bytes data from client[%d] and echo it back: '%s'\n", rv, event_array[i].data.fd, buf);
		
				if( write(event_array[i].data.fd, buf1, rv) < 0 )
				{
					printf("Write %d bytes data back to client[%d] failure: %s\n", rv, event_array[i].data.fd, strerror(errno));
					close(event_array[i].data.fd);
				}
			}
		}
		
		//sleep(1);
		//close(clifd);
	}


Exit1:
	if(rv<0)
		close(serv_infor_t.fd);
	else
		rv = 0;

	return 0;
}







