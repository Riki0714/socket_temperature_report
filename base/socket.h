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
	int		backlog;
	char   *ip;
} sock_infor;

int  server_init(sock_infor *infor_t, struct sockaddr_in *addr);
int  socket_connect(int fd, struct sockaddr *addr, int len);
int  socket_listen(int fd, int backlog);
int  socket_bind(int fd, struct sockaddr *addr, int len);
int  socket_write(int fd, char *buf, int bytes);
int  socket_read(int fd, char *buf, int bytes);
int  server_connect(sock_infor *serv_infor_t); //Initialize the server
int  client_connect(sock_infor *cli_infor_t); //The client tries the server
void set_socket_rlimit(void); //set max open socket count
char *socket_dns(char *doname, int size);

#endif

