/*********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  server.c
 *    Description:  server test
 *                 
 *        Version:  1.5.2(03/21/23)
 *         Author:  xiao
 *      ChangeLog:  1, Release initial version on "03/21/23 07:22:53"
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

#include "socket.h"
#include "packet.h"
#include "sql.h"

//#define  CONFIG_DEBUG	
#include "debug.h"


#define BACKLOG		13
#define MAX_EVENTS	512
#define BUF_LEN		64
#define BASENAME    "ser_data.db"


void usage_print(char *proName);


int main(int argc, char *argv[])
{
	socket_t				sock;  //server information(ip, port...)
	packet_t				pack;
	int						connfd = -1;   //fd -- Connect to a new client
	int						rv = -20;
	int						i = 0; 
	char					buf_rece[BUF_LEN] = {0}; 
	char					buf_trans[BUF_LEN]="hello, your data has been received!\n"; 
	int						daemon_flag = 0;  //1:Server background  0:Front-end print data
	char					dfIp[32] = "0.0.0.0"; 
	int						port = 7799;
	int						backlog = 13;
	
	int						epollfd = -1; 
	int						events = -1;
	struct epoll_event		event;
	struct epoll_event		event_array[MAX_EVENTS];

	char					*db_name = BASENAME;
	char					*errmsg = NULL;

	int						ch = -1; 
	struct option			opts[] = 
	{
		{"--port",    		required_argument,	NULL, 'p'},
		{"--ipaddr",  		optional_argument, 	NULL, 'i'},
		{"--daemon",  		optional_argument, 	NULL, 'd'},
		{"--backlog",	 	optional_argument, 	NULL, 'b'},
		{"--help",    		no_argument, 	    NULL, 'h'},
		{0, 0, 0, 0}    ///!!!
	}; ///!!!


	while( (ch=getopt_long(argc, argv, "p:i::d::b::h", opts, NULL)) != -1 )
	{
		switch( ch )
		{
			case 'p':
				port = atoi(optarg);
				break;

			case 'd':
				daemon_flag = 1;
				break;

			case 'b':
				backlog = atoi(optarg);
				break;

			case 'h':
				usage_print(argv[0]);
				return 0;
		}
	}

	//set max open socket count
	set_socket_rlimit();

	//---------------- socket server  -----------------
	memset(&sock, 0, sizeof(sock));
	if( socket_init(&sock, dfIp, port) < 0)
	{
		printf("server initialization error!\n");
		return -23;
	}

	//---------------- socket server  -----------------
	if( socket_listen(&sock) < 0)
	{
		printf("server listen error!\n");
		return -23;
	}

	//---------------- create dataBase  -----------------
	rv = db_open(db_name);
	if( rv<0 )
	{
		db_close();
		return -24;
	}
	dbg_print("Open database successfully! %d\n", rv);


	//---------------- epoll -----------------
	event.events = EPOLLIN; //Specify event type
	event.data.fd = sock.fd;

	//1. create epoll
	if( (epollfd = epoll_create(MAX_EVENTS)) < 0 )
	{
		printf("create epoll failure: %s\n", strerror(errno));
		return -25;
	}
	dbg_print("create epoll successfully! %d\n", rv);

	//2. epoll_ctl() Modify the interest list of epoll
	if( (rv = epoll_ctl(epollfd, EPOLL_CTL_ADD, sock.fd, &event)) < 0 )
	{
		printf("epoll add listen socket failure: %s\n", strerror(errno));
		return -25;
	}
	
	printf("Start waiting and acccept new client connect...\n");

	//---------------- daemon ----------------
	if(daemon_flag) 
	{
		if( daemon(1,1) < 0 )
		{
			printf("daemon failure: %s\n", strerror(errno));
			return -26;
		}
		
	}

	while(1)
	{
		//printf("server waiting...\n");

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
			if( event_array[i].data.fd == sock.fd )
			{
				if( (connfd = accept(sock.fd, (struct sockaddr *)NULL, NULL)) < 0 )
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
				memset(buf_rece, 0, sizeof(buf_rece));
				rv = read(event_array[i].data.fd, buf_rece, sizeof(buf_rece));
				if( rv<0 )
				{
					printf("read data from server[%d] failure: %s\n", sock.fd, strerror(errno));
					continue;
				}
				else if( rv==0 )
				{
					printf("now disconnect\n");
					continue;
				}
				else
				{
					printf("read %d bytes from client[%d] and echo it back: '%s'\n", rv, sock.fd, buf_rece);

					//Put the data into the database
					unpack_data(&pack, buf_rece, BUF_LEN);
					db_insert(&pack);
					printf("Successfully put the data into the database [%s]\n", db_name);
				}


				/* 
				rv = socket_read(event_array[i].data.fd, buf_rece, sizeof(buf_rece));
				if( rv<0 || rv==0 )
				{
					close(event_array[i].data.fd);
					continue;
				}
				if( rv>0 )
				{
					printf("receive %d bytes from client[%d] and echo it:'%s'\n", rv, event_array[i].data.fd, buf_rece);

					//Put the data into the database
					unpack_data(&pack, buf_rece, BUF_LEN);
					db_insert(&pack);
					printf("Successfully put the data into the database [%s]\n", db_name);
				}
				*/

				/*  
				if( socket_write(event_array[i].data.fd, buf_trans, sizeof(buf_trans)) < 0)
				{
					close(event_array[i].data.fd);
				}
				*/
			}
		}
	}

	socket_close(&sock);
	db_close();

	return 0;
}


void usage_print(char *proName)
{
	printf("Some parameters about this program[%s]:\n", proName);
	printf("1.The parameters you must enter:\n");
	printf("Port: '-p' '--port', such as: -p 6666 \n");
	printf("\n2.You can choose which parameters to enter:\n");
	printf("-i(--ipaddr): ip address of client\n");
	printf("-b(--backlog): listen backlog\n");
	printf("-d(--daemon): If the value is '1', the server starts background work\n");
	printf("-t(--backlog): The name of the table that holds the data received from the client\n");
	printf("-h(--help): some help\n");
}




