/*********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  client.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(02/26/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "02/26/23 08:20:49"
 *                 
 ********************************************************************************/
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/signal.h>

#include "get_temp.h"
#include "mySocket.h"
#include "myDns.h"
#include "myTimer.h"
#include "mySql.h"
#include "sqlite3.h"
#include "myLinkList_temp.h"

#define STR_LEN		256
#define NAME_LEN	64
#define TIME 		5

#define INPUT_PARA_ERROR  	-2
#define GET_TEMP_ERROR 	  	-3
#define BASENAME			"cliData.db"
#define TABELNAME			"TEMP"

//#define CONFIG_DEBUG
#include "myDebug.h"


void print_usage(char *proname); //man -- Command line argument parsing
void sig_alrm(int sig); //sigalrm callback
void sig_pipe(int sig); //sigalrm callback

int alrm_flag=1; //timer flag
int	link_flag = 0;  //1:have connected to the server   0:unconnected


int main(int argc, char *argv[])
{
	sock_infor		cli_infor_t; 	//Basic information about the client(ip address, port...)
	int				time = TIME; 	//sample interval
	int 			rv = -1;	    //return value
	char 			serialNum[STR_LEN] = {0}; //The obtained temperature string
	char 		   *serialNum_b = (char *)calloc(0, STR_LEN);
	int				errorFlag = 0;	//Gets the number of temperature errors
	//int				link_flag = 0;  //1:have connected to the server   0:unconnected
	char    		ReceBuf[STR_LEN]={0};  //Stores the data received from the server
	char			bufToDb[STR_LEN]={0}; //Data to be temporarily stored in the database

	Node 		   *pList = NULL; //A header pointer to hold temporary database
	Node		   *pHead = NULL; //tmp

	//database
	sqlite3		   *db=NULL;
	char		   *errmsg = NULL;
	int			  	dataIndex = 0;
	char		   *dbName = BASENAME;
	char		   *tbName = TABELNAME;

	memset(&cli_infor_t, 0, sizeof(cli_infor_t));
	cli_infor_t.ip = (char *)&rv;
	cli_infor_t.fd = -1;

	//-------------- Command line argument parsing  --------------
	int				ch = -1; //Parameter resolution return value
	struct option	opts[] = {
		{"ipaddr", 	 required_argument,  NULL, 'i'},
		{"port",   	 required_argument,  NULL, 'p'},
		{"time",  	 optional_argument,  NULL, 't'},
		{"dbName",   optional_argument,  NULL, 'a'},
		{"tbName",   optional_argument,  NULL, 'b'},
		{"help",  	 no_argument,        NULL, 'h'},
		{0, 0, 0, 0}
	};

	while( (ch=getopt_long(argc, argv, "i:p:t::a::b::h", opts, NULL)) != -1 )
	{
		switch(ch)
		{
			case 'i':
			{
				int 	getLen = strlen(optarg);
				char 	getStr[100] = {0};

				strncpy(getStr, optarg, 100);
				if( (getStr[getLen-1]>'a' && getStr[getLen-1]<'z') ||  (getStr[getLen-1]>'A' && getStr[getLen-1]<'Z') )
				{
					cli_infor_t.ip = get_ip_dns(optarg ,100); //dns
				}
				else
					cli_infor_t.ip = optarg;

				break;
			}

			case 'p':
				cli_infor_t.port = atoi(optarg);
				break;

			case 'a':
				dbName = optarg;
				break;

			case 'b':
				tbName = optarg;
				break;

			case 't':
				time = atoi(optarg);
				break;

			case 'h':
				print_usage(argv[0]);
				return 0;
		}
	}
	//----------------------------------------
  
	if( !cli_infor_t.port || !cli_infor_t.ip )
	{
		printf("The entered ip and port are incorrect!\n");
		print_usage(argv[0]);
		return INPUT_PARA_ERROR;
	}
	printf("The entered ip and port are correct!\n");
	dbg_print("%s %d %d\n", cli_infor_t.ip, cli_infor_t.port, time);


	//--------------------- open database --------------------------
	rv = -1;
	rv = sqlite3_open(dbName, &db);
	if(rv)
	{
		printf("open database %s failure: %s\n", dbName, sqlite3_errmsg(db));
		sqlite3_close(db);
		return -12;
	}
	if( sql_op(db, tbName, FIND, NULL) ) //1:The file does not exist   0:already exists
	{
		sql_op(db, tbName, CREATE, "id int, content char");
	}
	dbg_print("Open database successfully! %d\n", rv);


	//------------ connect to server -------------
	printf("Now the client tries the server...\n");
	if( (rv = client_init(&cli_infor_t) < 0) )
	{
		printf("Failed to connect to the server\n");
		link_flag = 0;
	}
	else  link_flag = 1;
	

	//signal timer
	//signal(SIGALRM, sig_alrm);
	signal(SIGPIPE, sig_pipe);


	while(1)
	{
		//if(alrm_flag)
		//{

		//----------- Acquired temperature -------------
		if ( !get_temp_str(serialNum, STR_LEN) )
		{
			printf("get temperature error!\n");
			errorFlag++;

			if(errorFlag==5)
			{
				printf("Sorry, please check your hardware device and try again\n");
				return -1; //If you can't get it, an error exit is reported
			}
			else
			{
				printf("Unable to obtain temperature, trying again...\n");
				sleep(5);
				continue;
			}
		}
		errorFlag = 0;
		dbg_print("%s\n", serialNum);
	

		//------------ Send data to the server -------------
		if( link_flag )
		{
			dbg_print("hei %d\n", link_flag);
			if( write(cli_infor_t.fd, serialNum, strlen(serialNum)) < 0 )
			{
				printf("Write %d bytes data back to client[%d] failure: %s\n", rv, cli_infor_t.fd, strerror(errno));
				close(cli_infor_t.fd);
				link_flag = 0;

				//continue;
				//goto EXIT1;
			}

			memset(ReceBuf, 0, sizeof(ReceBuf));
			if( ( rv=read(cli_infor_t.fd, ReceBuf, sizeof(ReceBuf)) )  < 0 )
			{
				printf("read data from server[%d] failure: %s\n", cli_infor_t.fd, strerror(errno));
				close(cli_infor_t.fd);
				link_flag = 0;
				//continue;
			}
			else if( rv==0 )
			{
				printf("The connection to the server[%d] is disconnected\n", cli_infor_t.fd);
				close(cli_infor_t.fd);
				link_flag = 0;
				//continue;
			}
			else	printf("read %d bytes data from server[%d] and echo it back: '%s\n'", rv, cli_infor_t.fd, ReceBuf);
		}
		

		if( !link_flag ) //Failed to connect to the server
		{
			//The maximum number of temporary data is 10
			if(dataIndex<100) dataIndex++;
			else 
			{
				dataIndex=1;
				list_clear(pList);
				sql_op(db, tbName, DROP, NULL);
				sql_op(db, tbName, CREATE, "id int, content char");
			}


			//------------ Insert a linked list from the header --------------
			list_insert_head(&pList, NULL, serialNum);
			//list_print(pList);

			//------------ Put into database ---------------
			memset(bufToDb, 0, sizeof(bufToDb));
			snprintf(bufToDb, sizeof(bufToDb), "%d, '%s'", dataIndex, serialNum);
			sql_op(db, tbName, INSERT, bufToDb);
			printf("Successfully put data into the database [%s - %s]\n", dbName, tbName);


			dbg_print("1: %d\n", dataIndex);


			//------------ reconnection --------------------
			if( (rv = client_init(&cli_infor_t) < 0) )
			{
				printf("Failed to connect to the server\n");
				link_flag = 0;
			}
			else
			{
				link_flag = 1;
			  	  
				if(dataIndex>0)
				{
					dbg_print("relink%d\n", link_flag);
					char	*buf_o=NULL;

					for(int i=dataIndex; i>0; i--)
					{
			  			//pHead = pList;
						
						memset(bufToDb, 0, STR_LEN);
						strncpy(bufToDb, list_get(pList, i), STR_LEN);

						//The data in the linked list is sent to the server
						//if( write(cli_infor_t.fd, pHead->element, strlen(serialNum)) < 0 )
						if( write(cli_infor_t.fd, bufToDb, strlen(serialNum)) < 0 )
						{
							printf("Write %d bytes data back to client[%d] failure: %s\n", rv, cli_infor_t.fd, strerror(errno));
							close(cli_infor_t.fd);
							link_flag = 0;

							break;
						}
						//list_drop_head(&pList);
						list_drop_tail(&pList);
						
						if(link_flag)
						{
							//Delete the data from the database
							memset(bufToDb, 0, sizeof(bufToDb));
							snprintf(bufToDb, sizeof(bufToDb), "id=%d", i);
							sql_op(db, tbName, DELETE, bufToDb);
						}

						if(dataIndex>0) dataIndex--;
					}

					if( link_flag )
					{
						dataIndex = 0;
						//list_clear(pList);
					}
				} 
			}

		}

		dbg_print("hei22 %d\n", link_flag);

		timer_s(time, 0);

		//signal(SIGALRM, sig_alrm);
		//alarm(time);
		//alrm_flag = 0;

		//}
	}

//EXIT1:
	sqlite3_close(db);

	return 0;
}

/* 
name: 		print_usage
function:	Description of command line parameters
parameter:	proname--program name
value:		NULL
*/
void print_usage(char *proname)
{
	printf("hello, this is %s:\n", proname);
	printf("-i(--ipaddr): ip address of server\n");
	printf("-p(--port): port of server\n");
	printf("-t(--time): sampling interval\n");
	printf("-a(--dbName): The name of the client temporary database\n");
	printf("-b(--tbName): The name of the table in the client's temporary database\n");
	printf("-h(--help): some help\n");
}

/* 
name: 		sig_alrm
function:	Timer -- signal
parameter:	sig--Signal type
value:		NULL
*/
void sig_alrm(int sig)
{
	if(sig == SIGALRM)
	{
		alrm_flag = 1;
	}
//	alarm(5);
}

void sig_pipe(int sig)
{
	link_flag = 0;
}



