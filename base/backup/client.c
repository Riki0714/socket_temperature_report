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

#include "get_temp.h"


#define STR_LEN		128
#define TIME 		30

#define INPUT_PARA_ERROR  -2
#define GET_TEMP_ERROR 	  -3

#define CONFIG_DEBUG
#ifdef  CONFIG_DEBUG
#define dbg_print(format, args...) printf(format, args)
#else
#define dbg_print(format, args...) do{} while(0)
#endif

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
	printf("-h(--help): some help\n");
}

#define SER_PORT	  9999
#define SER_IP		  "127.0.0.1"

int main(int argc, char *argv[])
{
	sock_infor	cli_infor_t;
	//int 	cli_fd = -1;
	int		file_fd = -1;
	//char    *ipaddress = NULL;
	//int		port = 0;
	int		sampleInt = TIME; //sample interval
	int		ch = -1; //Parameter resolution return value
	int 	rv = -1; //return value
	char 	serialNum[STR_LEN] = {0}; //The obtained temperature string
	int		errorFlag = 0;

	memset(&cli_infor_t, 0, sizeof(cli_infor_t));
	cli_infor_t.ip = &rv;
	cli_infor_t.fd = -1;

	struct option	opts[] = {
		{"ipaddr", required_argument,  NULL, 'i'},
		{"port",   required_argument,  NULL, 'p'},
		{"time",   optional_argument, NULL, 't'},
		{"help",   no_argument,        NULL, 'h'},
		{0, 0, 0, 0}
	};

	while( (ch=getopt_long(argc, argv, "i:p:t::h", opts, NULL)) != -1 )
	{
		switch(ch)
		{
			case 'i':
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

			case 'p':
				port = atoi(optarg);
				break;

			case 't':
				sampleInt = atoi(optarg);
				break;

			case 'h':
				print_usage(argv[0]);
				return 0;
		}
	}
  
	if( !port || !ipaddress )
	{
		printf("The entered ip and port are incorrect!\n");
		print_usage(argv[0]);
		return INPUT_PARA_ERROR;
	}
	dbg_print("The entered ip and port are correct!\n");

	/*  
	while(1)
	{
		dbg_print("%s %d %d\n", ipaddress, port, time);

		//Acquired temperature
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
		dbg_print("%s", serialNum);


		//connect to server
		rv = client_init(cli_infor_t);
		if()
		{

		}
		
	}
*/

	return 0;
}





