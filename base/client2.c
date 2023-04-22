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
#include <time.h>

#include "get_temp.h"
#include "socket.h"
#include "timer.h"
#include "sql.h"
#include "sqlite3.h"
#include "linklist_temp.h"

#define STR_LEN		256
#define NAME_LEN	64
#define INTERVAL	5

#define INPUT_PARA_ERROR	-2
#define GET_TEMP_ERROR		-3
#define BASENAME			"cli_data.db"
#define TABLENAME			"TEMP"

//#define CONFIG_DEBUG
#include "debug.h"


void print_usage(char *proname); //man -- Command line argument parsing
void sig_pipe(int sig); 

int	g_link_flag = 0;  //1:have connected to the server   0:unconnected


int main(int argc, char *argv[])
{
	sock_infor		cli_infor_t; 	//Basic information about the client(ip address, port...)
	int				sample_intv = INTERVAL; 	//sample interval
	int				rv = -1;	    
	int				index = 0;
	int				last_time=0;
	int				now_time=0;
	char			cont_str[STR_LEN] = {0}; //The obtained temperature string
	int				error_flag = 0;	//Gets the number of temperature errors
	char			buf_rece[STR_LEN] = {0}; 
	char			buf_to_db[STR_LEN] = {0}; 

	Node			*plist = NULL; //A header pointer to hold temporary database
	Node			*phead = NULL;  

	sqlite3			*db = NULL;
	char			*errmsg = NULL;
	char			*dbname = BASENAME;
	char			*tbname = TABLENAME;

	memset(&cli_infor_t, 0, sizeof(cli_infor_t));
	cli_infor_t.ip = (char *)&rv;
	cli_infor_t.fd = -1;

	//-------------- Command line argument parsing  --------------
	int				ch = -1; 
	struct option	opts[] = {
		{"ipaddr",		required_argument,  NULL, 'i'},
		{"port",		required_argument,  NULL, 'p'},
		{"sample_intv",	optional_argument,  NULL, 't'},
		{"help",		no_argument,        NULL, 'h'},
		{0, 0, 0, 0}
	};

	while( (ch=getopt_long(argc, argv, "i:p:t::h", opts, NULL)) != -1 )
	{
		switch(ch)
		{
			case 'i':
			{
				in_addr_t	addr=inet_addr(optarg);
				if( addr<0 )
				{
					cli_infor_t.ip = socket_dns(optarg ,100); //dns
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

			case 't':
				sample_intv = atoi(optarg);
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
	dbg_print("%s %d %d\n", cli_infor_t.ip, cli_infor_t.port, sample_intv);


	//--------------------- open database --------------------------
	rv = -1;
	rv = sqlite3_open(dbname, &db);
	if(rv)
	{
		printf("open database %s failure: %s\n", dbname, sqlite3_errmsg(db));
		sqlite3_close(db);
		return -12;
	}
	if( sql_op(db, tbname, FIND, NULL) ) //1:The file does not exist   0:already exists
	{
		sql_op(db, tbname, CREATE, "id int, content char");
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

	signal(SIGPIPE, sig_pipe);

	now_time = time((time_t *)NULL);
	last_time = now_time;

	while(1)
	{
		now_time = time((time_t *)NULL);
		if( (now_time-last_time) < sample_intv )
		{
			continue;
		}
		else
		{
			last_time = now_time;
		}

		//----------- Acquired temperature -------------
		if ( !get_temp_str(cont_str, STR_LEN) )
		{
			printf("get temperature error!\n");
			error_flag++;

			if(error_flag==5)
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
		error_flag = 0;
		dbg_print("%s\n", cont_str);
	

		//------------ Send data to the server -------------
		if( g_link_flag )
		{
			dbg_print("linked %d\n", g_link_flag);
			
			if( socket_write(cli_infor_t.fd, cont_str, strlen(cont_str)) < 0 )
			{
				g_link_flag = 0;
			}

			rv = socket_read(cli_infor_t.fd, buf_rece, sizeof(buf_rece));
			if( rv<0 || rv==0 )
			{
				g_link_flag = 0;
			}
		}
		

		if( !g_link_flag ) //Failed to connect to the server
		{
			//------------ Insert a linked list from the header --------------
			list_insert_head(&plist, NULL, cont_str);

			//------------ Put into database ---------------
			memset(buf_to_db, 0, sizeof(buf_to_db));
			snprintf(buf_to_db, sizeof(buf_to_db), "%d, '%s'", index, cont_str);
			sql_op(db, tbname, INSERT, buf_to_db);
			printf("Successfully put data into the database [%s - %s]\n", dbname, tbname);


			dbg_print("1: %d\n", index);


			//------------ reconnection --------------------
			if( !g_link_flag )
			{
				rv = -1;
				if( (rv = client_init(&cli_infor_t) < 0) )
				{
					printf("Failed to connect to the server\n");
					g_link_flag = 0;
				}
				g_link_flag = 1;
			}
			
			if( g_link_flag )
			{
				if(index>0)
				{
					dbg_print("relink%d\n", g_link_flag);

					for(int i=index; i>0; i--)
					{
						memset(buf_to_db, 0, STR_LEN);
						strncpy(buf_to_db, list_get(plist, index), STR_LEN);

						if( socket_write(cli_infor_t.fd, buf_to_db, strlen(cont_str)) < 0 )
						{
							g_link_flag = 0;
							break;
						}

						list_drop_tail(&plist);
						
						if(g_link_flag)
						{
							//Delete the data from the database
							memset(buf_to_db, 0, sizeof(buf_to_db));
							snprintf(buf_to_db, sizeof(buf_to_db), "id=%d", index);
							sql_op(db, tbname, DELETE, buf_to_db);
						}

						if(index>0)  index--;
						if(index==0)  break;
					}
				} 
			}

		}

		dbg_print("hei22 %d\n", g_link_flag);
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
	printf("-t(--sample_intv): sampling interval\n");
	printf("-a(--dbname): The name of the client temporary database\n");
	printf("-b(--tbname): The name of the table in the client's temporary database\n");
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



