/*********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  client.c
 *    Description:  This file 
 *                 
 *        Version:  1.5.0(04/01/23)
 *         Author:  xiao
 *      ChangeLog:  1, Release initial version on "04/01/23 08:20:49"
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
#include "socket.h"
#include "dns.h"
#include "timer.h"
#include "sql.h"
#include "sqlite3.h"
#include "linklist_temp.h"

#define STR_LEN		256
#define NAME_LEN	64
#define TIME		5

#define INPUT_PARA_ERROR	-2
#define GET_TEMP_ERROR		-3
#define BASENAME			"cliData.db"
#define TABELNAME			"TEMP"

//#define CONFIG_DEBUG
#include "debug.h"


void print_usage(char *proname); //man -- Command line argument parsing
//void sig_alrm(int sig); 
void sig_pipe(int sig); 

//int g_alrm_flag=1; 
int	g_link_flag = 0;  //1:have connected to the server   0:unconnected


int main(int argc, char *argv[])
{
	sock_infor		cli_infor_t; 	//Basic information about the client(ip address, port...)
	int				sample_itv = TIME; 	//sample interval
	int				rv = -1;	    
	int				dataIndex = 0;
	char			serialNum[STR_LEN] = {0}; //The obtained temperature string
	int				errorFlag = 0;	//Gets the number of temperature errors
	char			ReceBuf[STR_LEN] = {0}; 
	char			bufToDb[STR_LEN] = {0}; 

	Node			*pList = NULL; //A header pointer to hold temporary database
	Node			*pHead = NULL;  

	//database
	sqlite3			*db = NULL;
	char			*errmsg = NULL;
	char			*dbName = BASENAME;
	char			*tbName = TABELNAME;

	memset(&cli_infor_t, 0, sizeof(cli_infor_t));
	cli_infor_t.ip = (char *)&rv;
	cli_infor_t.fd = -1;

	//-------------- Command line argument parsing  --------------
	int				ch = -1; //Parameter resolution return value
	struct option	opts[] = {
		{"ipaddr",		required_argument,  NULL, 'i'},
		{"port",		required_argument,  NULL, 'p'},
		{"sample_itv",	optional_argument,  NULL, 't'},
		{"dbName",		optional_argument,  NULL, 'a'},
		{"tbName",		optional_argument,  NULL, 'b'},
		{"help",		no_argument,        NULL, 'h'},
		{0, 0, 0, 0}
	};

	while( (ch=getopt_long(argc, argv, "i:p:t::a::b::h", opts, NULL)) != -1 )
	{
		switch(ch)
		{
			case 'i':
			{
				int		getlen = strlen(optarg);
				char	getstr[100] = {0};

				strncpy(getstr, optarg, 100);
				if( (getstr[getlen-1]>'a' && getstr[getlen-1]<'z') ||  (getstr[getlen-1]>'A' && getstr[getlen-1]<'Z') )
				{
					cli_infor_t.ip = get_ip_dns(optarg ,100); //dns
				}
				else
				{
					cli_infor_t.ip = optarg;
				}

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
				sample_itv = atoi(optarg);
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
	dbg_print("%s %d %d\n", cli_infor_t.ip, cli_infor_t.port, sample_itv);


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
		g_link_flag = 0;
	}
	else  g_link_flag = 1;
	

	//signal timer
	//signal(SIGALRM, sig_alrm);
	signal(SIGPIPE, sig_pipe);


	while(1)
	{
		//if(g_alrm_flag)
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
		if( g_link_flag )
		{
			dbg_print("hei %d\n", g_link_flag);
			if( write(cli_infor_t.fd, serialNum, strlen(serialNum)) < 0 )
			{
				printf("Write %d bytes data back to client[%d] failure: %s\n", rv, cli_infor_t.fd, strerror(errno));
				close(cli_infor_t.fd);
				g_link_flag = 0;

				//continue;
				//goto EXIT1;
			}

			memset(ReceBuf, 0, sizeof(ReceBuf));
			if( ( rv=read(cli_infor_t.fd, ReceBuf, sizeof(ReceBuf)) )  < 0 )
			{
				printf("read data from server[%d] failure: %s\n", cli_infor_t.fd, strerror(errno));
				close(cli_infor_t.fd);
				g_link_flag = 0;
				//continue;
			}
			else if( rv==0 )
			{
				printf("The connection to the server[%d] is disconnected\n", cli_infor_t.fd);
				close(cli_infor_t.fd);
				g_link_flag = 0;
				//continue;
			}
			else	printf("read %d bytes data from server[%d] and echo it back: '%s\n'", rv, cli_infor_t.fd, ReceBuf);
		}
		

		if( !g_link_flag ) //Failed to connect to the server
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
				g_link_flag = 0;
			}
			else
			{
				g_link_flag = 1;
			  	  
				if(dataIndex>0)
				{
					dbg_print("relink%d\n", g_link_flag);
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
							g_link_flag = 0;

							break;
						}
						//list_drop_head(&pList);
						list_drop_tail(&pList);
						
						if(g_link_flag)
						{
							//Delete the data from the database
							memset(bufToDb, 0, sizeof(bufToDb));
							snprintf(bufToDb, sizeof(bufToDb), "id=%d", i);
							sql_op(db, tbName, DELETE, bufToDb);
						}

						if(dataIndex>0) dataIndex--;
					}

					if( g_link_flag )
					{
						dataIndex = 0;
						//list_clear(pList);
					}
				} 
			}

		}

		dbg_print("hei22 %d\n", g_link_flag);

		timer_s(sample_itv, 0);

		//signal(SIGALRM, sig_alrm);
		//alarm(sample_itv);
		//g_alrm_flag = 0;

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
	printf("-t(--sample_itv): sampling interval\n");
	printf("-a(--dbName): The name of the client temporary database\n");
	printf("-b(--tbName): The name of the table in the client's temporary database\n");
	printf("-h(--help): some help\n");
}

/* 
name: 		sig_alrm
function:	timer -- signal
parameter:	sig--Signal type
value:		NULL
*/
/*
void sig_alrm(int sig)
{
	if(sig == SIGALRM)
	{
		g_alrm_flag = 1;
	}
//	alarm(5);
}
*/

void sig_pipe(int sig)
{
	g_link_flag = 0;
}



