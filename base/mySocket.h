/********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  mySocket.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(03/21/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "03/21/23 08:46:46"
 *                 
 ********************************************************************************/

#ifndef _MYSOCKET_H
#define _MYSOCKET_H

typedef struct socket_information{
	int		fd;
	int		port;
	char   *ip;
} sock_infor;

int server_init(sock_infor *serv_infor_t, int backlog); //Initialize the server
int client_init(sock_infor *cli_infor_t); //The client tries the server
void set_socket_rlimit(void); //set max open socket count

#endif

