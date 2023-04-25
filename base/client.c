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

#include "ds18b20.h"
#include "packet.h"
#include "socket.h"
#include "timer.h"
#include "sql.h"
#include "sqlite3.h"

#define STR_LEN		128
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
int time_out(int *last_time, int *now_time, int sample_intv);

int	g_link_flag = 0;  //1:have connected to the server   0:unconnected


int main(int argc, char *argv[])
{
	sock_infor		cli_infor_t; 	//Basic information about the client(ip address, port...)
	packet_t		pack; //(devsn, time, temperature...)
	int				sample_intv = INTERVAL; 	//sample interval
	int				rv = -1;	    
	int				index = 0;
	int				last_time=0;
	int				now_time=0;
	int				timeout_flag=0;
	char			pack_str[STR_LEN] = {0}; //The obtained temperature string
	char			buf_rece[STR_LEN] = {0}; 
	char			buf_to_db[STR_LEN] = {0}; 

	sqlite3			*db = (sqlite3 *)&rv;
	char			*dbname = BASENAME;
	char			*tbname = TABLENAME;

	memset(&cli_infor_t, 0, sizeof(cli_infor_t));
	cli_infor_t.ip = "127.0.0.1";
	cli_infor_t.port = 7799;
	cli_infor_t.fd = -1;

	//-------------- Command line argument parsing  --------------
	int				ch = -1; 
	struct option	opts[] = {
		{"ipaddr",		optional_argument,  NULL, 'i'},
		{"port",		optional_argument,  NULL, 'p'},
		{"sample_intv",	optional_argument,  NULL, 't'},
		{"help",		no_argument,        NULL, 'h'},
		{0, 0, 0, 0}
	};

	while( (ch=getopt_long(argc, argv, "i::p::t::h", opts, NULL)) != -1 )
	{
		switch(ch)
		{
			case 'i':
			{
				cli_infor_t.ip = optarg;
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
	rv = db_open(&db, dbname, tbname);
	if( rv<0 )
	{
		//printf("open database %s failure: %s\n", dbname, sqlite3_errmsg(db));
		db_close(db);
		return -12;
	}
	dbg_print("Open database successfully! %d\n", rv);


	//------------ connect to server -------------
	printf("Now the client tries the server...\n");
	if( (rv = client_connect(&cli_infor_t) < 0) )
	{
		printf("Failed to connect to the server\n");
		g_link_flag = 0;
	}
	else  g_link_flag = 1;

	signal(SIGPIPE, sig_pipe);

	//now_time = time((time_t *)NULL);
	//last_time = now_time;

	while(1)
	{	  
		if( !timeout_flag)
		{	
			now_time = time((time_t *)NULL);

			if( (now_time-last_time) < sample_intv ) 
				continue; 
			else 
				last_time = now_time;
		}
		else timeout_flag = 0;

		//----------- Acquired temperature -------------
		if ( !sample_temperature(&pack) )
		{
			printf("get temperature error!\n");
			continue;
		}
		pack_data(&pack, pack_str, STR_LEN);
		dbg_print("%s\n", pack_str);
	

		//------------ Send data to the server -------------
		if( g_link_flag )
		{
			dbg_print("linked %d\n", g_link_flag);
			
			if( socket_write(cli_infor_t.fd, pack_str, strlen(pack_str)) < 0 )
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
			//------------ Put into database ---------------
			db_insert(db, tbname, &pack);
			printf("Successfully put data into the database [%s - %s]\n", dbname, tbname);

			//------------ reconnection --------------------
			if( !g_link_flag )
			{
				if( (rv = client_connect(&cli_infor_t) < 0) )
				{
					printf("Failed to connect to the server\n");
					g_link_flag = 0;
				}
				else
					g_link_flag = 1;
			}
			
			if( g_link_flag )
			{
				dbg_print("relink%d\n", g_link_flag);

				while(1)
				{
					now_time = time((time_t *)NULL);
					if( (now_time-last_time) > sample_intv ) 
					{
						last_time = now_time;
						timeout_flag = 1;
						break;
					}

					rv = db_query(db, tbname, &pack);
					if( rv==QUERY_ERROR )
					{
						printf("get first data from database[%s] table[%s] failure!\n", dbname, tbname);
						break;
					}
					else if( rv==QUERY_NULL )
					{
						printf("there is no data in the database[%s] table[%s]\n", dbname, tbname);
						break;
					}
					memset(buf_to_db, 0, sizeof(buf_to_db));
					pack_data(&pack, buf_to_db, sizeof(buf_to_db));

					if( socket_write(cli_infor_t.fd, buf_to_db, strlen(pack_str)) < 0 )
					{
						g_link_flag = 0;
						break;
					}
					if( db_remove(db, tbname)<0 )
					{
						printf("delete data from database[%s] table[%s] failure!\n", dbname, tbname);
						continue;
					}
				}
			}
		}

		dbg_print("again %d\n", g_link_flag);
	}

//EXIT1:
	db_close(db);

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

void sig_pipe(int sig)
{
	g_link_flag = 0;
}

int time_out(int *last_time, int *now_time, int sample_intv)
{
	*now_time = time((time_t *)NULL);
	if( (*now_time - *last_time) < sample_intv )
	{
		return 0;
	}
	else
	{
		*last_time = *now_time;
		return 1;
	}

	return -1;
}



