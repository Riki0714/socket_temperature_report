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

#include "mySocket.h"
#include "myDns.h"
#include "mySql.h"

//#define  CONFIG_DEBUG	
#include "myDebug.h"


#define BACKLOG		13
#define MAX_EVENTS	512
#define BUF_LEN		64
#define BASENAME    "serData.db"
#define TABLENAME	"TEMP"


void usage_print(char *proName)
{
	printf("Some parameters about this program[%s]:\n", proName);
	printf("1.The parameters you must enter:\n");
	printf("Port: '-p' '--port', such as: -p 6666 \n");
}

int main(int argc, char *argv[])
{
	sock_infor				serv_infor_t;  //server information(ip, port...)
	int						connfd = -1;   //fd -- Connect to a new client
	int						rv = -20, i=0; //for test
	char					bufRece[BUF_LEN] = {0}; //Stores data sent by the client
	char					bufToDb[BUF_LEN] = {0}; //The data that the server wants to upload to the database
	char					bufToCli[BUF_LEN]="hello, your data has been received!\n"; //Data returned by the server to the client
	int 					daemon_flag = 0;   //1:Server background  0:Front-end print data
	int						backlog = BACKLOG; //listen backlog
	char					dfIp[32] = "0.0.0.0"; //Default Ip
	int						dataIndex = 0;
	
	int						epollfd;  //Parameters about epoll
	int						events;
	struct epoll_event		event;
	struct epoll_event		event_array[MAX_EVENTS];

	char				   *dbName = BASENAME;
	char				   *tbName = TABLENAME;
//	char					dbName[32] = BASENAME; //Parameters about database
//	char					tbName[32] = TABLENAME;
	sqlite3				   *db = NULL;
	char				   *errmsg=NULL;

	int						ch = -1; //Parameters about command line argument parsing
	struct option 	 		opts[] = 
	{
		{"--port",    		required_argument,	NULL, 'p'},
		{"--ipaddr",  		optional_argument, 	NULL, 'i'},
		{"--daemon",  		optional_argument, 	NULL, 'd'},
		{"--backlog",	 	optional_argument, 	NULL, 'b'},
		{"--databaseName",  optional_argument, 	NULL, 'a'},
		{"--tableName", 	optional_argument, 	NULL, 't'},
		{"--help",    		no_argument, 	    NULL, 'h'},
		{0, 0, 0, 0}    ///!!!
	}; ///!!!

	memset(&serv_infor_t, 0, sizeof(serv_infor_t));
	serv_infor_t.fd = -1;
	serv_infor_t.ip = dfIp;
	serv_infor_t.port = 6666;

	while( (ch=getopt_long(argc, argv, "p:i::d::b::a::t::h", opts, NULL)) != -1 )
	{
		switch( ch )
		{
			case 'p':
				serv_infor_t.port = atoi(optarg);
				break;

			case 'd':
				daemon_flag = 1;
				break;

			case 'b':
				backlog = atoi(optarg);
				break;

			case 'a':
				dbName = optarg;
				break;

			case 't':
				tbName = optarg;
				break;

			case 'h':
				usage_print(argv[0]);
				return 0;
		}
	}

	if( !serv_infor_t.port )
	{
		printf("Please enter the port number to listen for(-p xxx)\n");
		usage_print(argv[0]);
		return -2;
	}

	dbg_print("%d\n", serv_infor_t.port);
	//set max open socket count
	set_socket_rlimit();

	//---------------- socket server  -----------------
	//Initialize the server
	if( server_init(&serv_infor_t, BACKLOG) < 0)
	{
		printf("server initialization error!\n");
		return -23;
	}

	//---------------- create dataBase
	rv = -1;
	rv = sqlite3_open(dbName, &db);
	if( rv )
	{
		printf("open database %s failure: %s\n", dbName, sqlite3_errmsg(db));
		sqlite3_close(db);
		return -24;
	}
	if( sql_op(db, tbName, FIND, NULL) ) //If 1 is returned, the table does not exist
	{
		sql_op(db, tbName, CREATE, "id int, content char"); //create a new table
	}
	sql_op(db, tbName, CREATE, "id int, content char"); //create a new table
	sqlite3_exec(db, "INSERT INTO TEMP VALUES(0, '------ new data ------')", NULL, NULL, &errmsg);

	dbg_print("Open database successfully! %d\n", rv);


	//---------------- epoll -----------------
	event.events = EPOLLIN; //Specify event type
	event.data.fd = serv_infor_t.fd;

	//1. create epoll
	if( (epollfd = epoll_create(MAX_EVENTS)) < 0 )
	{
		printf("create epoll failure: %s\n", strerror(errno));
		return -25;
	}
	dbg_print("create epoll successfully! %d\n", rv);

	//2. epoll_ctl() Modify the interest list of epoll
	if( (rv = epoll_ctl(epollfd, EPOLL_CTL_ADD, serv_infor_t.fd, &event)) < 0 )
	{
		printf("epoll add listen socket failure: %s\n", strerror(errno));
		return -25;
	}
	
	printf("Start waiting and acccept new client connect...\n");

	//---------------- daemon ----------------
	if(daemon_flag) 
	{
		if( daemon(0,0) < 0 )
		{
			printf("daemon failure: %s\n", strerror(errno));
			return -26;
		}
		
	}

	while(1)
	{
		printf("waiting...\n");

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
				memset(bufRece, 0, sizeof(bufRece));
				if( (rv=read(event_array[i].data.fd, bufRece, sizeof(bufRece))) < 0)
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
				else
				{
					char *errmsg=NULL;
					printf("read %d bytes data from client[%d] and echo it back: '%s'\n", rv, event_array[i].data.fd, bufRece);
		
					//Put the data into the database
					if(dataIndex<10000) dataIndex++;
					else	dataIndex=1;

					snprintf(bufToDb, sizeof(bufToDb),"%d, '%s'", dataIndex, bufRece);
					sql_op(db, tbName, INSERT, bufToDb);
					printf("Successfully put the data into the database [%s - %s]\n", dbName, tbName);
				}

				if( write(event_array[i].data.fd, bufToCli, rv) < 0 )
				{
					printf("Write %d bytes data back to client[%d] failure: %s\n", rv, event_array[i].data.fd, strerror(errno));
					close(event_array[i].data.fd);
				}
			}
		}
		
		//sleep(1);
		//close(clifd);
	}

	close(serv_infor_t.fd);
	sqlite3_close(db);

Exit1:
	if(rv<0)
		close(serv_infor_t.fd);
		sqlite3_close(db);

	return rv;
}







