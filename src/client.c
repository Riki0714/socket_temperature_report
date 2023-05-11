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
#include "sql.h"
#include "sqlite3.h"

#define STR_LEN		128
#define NAME_LEN	64
#define INTERVAL	5

#define INPUT_PARA_ERROR	-2
#define GET_TEMP_ERROR		-3
#define BASENAME			"cli_data.db"

//#define CONFIG_DEBUG
#include "debug.h"


void print_usage(char *proname); //man -- Command line argument parsing
void sig_pipe(int sig); 

int	g_sigpipe_flag = 0;  //1:have connected to the server   0:unconnected

int main(int argc, char *argv[])
{
	socket_t		sock; //Basic information about the client(ip address, port...)
	packet_t		pack; //(devsn, time, temperature...)
	int				sample_intv = INTERVAL; 	//sample interval
	int				sample_flag = 0; 	//sample interval
	int				rv = -1;	    
	int				index = 0;
	int				last_time=0;
	int				now_time=0;
	int				timeout_flag=0;
	char			pack_str[STR_LEN] = {0}; //The obtained temperature string
	char			buf_rece[STR_LEN] = {0}; 
	char			buf_to_db[STR_LEN] = {0}; 
	char			*dbname = BASENAME;

	int				port = 7799;
	char			*hostname = "127.0.0.1";


	//-------------- Command line argument parsing  --------------
	int				ch = -1; 
	struct option	opts[] = {
		{"hostname",	optional_argument,  NULL, 'h'},
		{"port",		optional_argument,  NULL, 'p'},
		{"sample_intv",	optional_argument,  NULL, 't'},
		{"help",		no_argument,        NULL, 'H'},
		{0, 0, 0, 0}
	};

	while( (ch=getopt_long(argc, argv, "h::p::t::H", opts, NULL)) != -1 )
	{
		switch(ch)
		{
			case 'h':
				hostname = optarg;
				break;

			case 'p':
				port = atoi(optarg);
				break;

			case 't':
				sample_intv = atoi(optarg);
				break;

			case 'H':
				print_usage(argv[0]);
				return 0;
		}
	}

	//--------------------- socket init------------------------
	memset(&sock, 0, sizeof(sock));
	if( (rv=socket_init(&sock, hostname, port)) < 0 )
	{
		printf("socket init failure!\n");
		return -1;
	}
	printf("socket init successfully! %d\n", rv);

	//--------------------- open database --------------------------
	rv = db_open(dbname);
	if( rv<0 )
	{
		printf("open database %s failure\n", dbname);
		db_close();
		return -2;
	}
	printf("Open database successfully! %d\n", rv);
		
	while(1)
	{	  
		//----------- Acquired temperature -------------
		now_time = time((time_t *)NULL);
		if( (now_time-last_time) >= sample_intv ) 
		{
			last_time = now_time;
			if( !sample_temperature(&pack) )
			{
				printf("get temperature faliure!\n");
				continue;
			}
			sample_flag = 1;
		}
		
		//------------ Connect or not --------------
		rv = socket_diag(&sock);
		if( rv < 0  )
		{
			dbg_print("Can't determine whether socket is connected %d\n", rv);
			//return -3;
		}

		//------------ reconnect ------------
		if( !sock.connected ) 
		{
			if( (rv=socket_connect(&sock)) < 0 )
			{
				dbg_print("Failed to connect to the server %d\n", rv);
				//return -4;
			}
		}

		//------------ disconnected  ----------
		if( !sock.connected )
		{
			if( sample_flag )
			{
				if( db_insert(&pack) < 0 )
				{
					printf("Put data into database failure!\n");
					return -5;
				}
				dbg_print("Successfully put data into the database [%s]\n", dbname);
				sample_flag = 0;
			}
			continue;
		}

		//------------- connected ---------------
		if( sample_flag )
		{
			sample_flag = 0;
			if( socket_send_packet(&sock, &pack) < 0)
			{
				printf("Faild to send data to the server\n");
				if( db_insert(&pack) < 0 )
				{
					printf("Put data into database failure!\n");
					return -6;
				}
			}
		}

		memset(&pack, 0, sizeof(pack));
		rv = db_query(&pack); //Gets the first data in the database
		if( rv==QUERY_ERROR )
		{
			dbg_print("get first data from database[%s] failure\n", dbname);
			continue;
		}
		else if( rv==QUERY_NULL )
		{
			dbg_print("database[%s] is null\n", dbname);
			continue;
		}

		if( socket_send_packet(&sock, &pack) < 0 )
		{
			printf("Faild to send database data to the server\n");
			continue;
		}
		if( db_remove()<0 )
		{
			printf("delete data from database[%s] failure\n", dbname);
		}
	}

	db_close();
	return 0;
}

void print_usage(char *proname)
{
	printf("hello, this is %s:\n", proname);
	printf("-i(--ipaddr): ip address of server\n");
	printf("-p(--port): port of server\n");
	printf("-t(--sample_intv): sampling interval\n");
	printf("-h(--help): some help\n");
}

void sig_pipe(int sig)
{
	g_sigpipe_flag = 1;
}





