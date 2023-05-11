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

#include "packet.h"

typedef struct socket_information{
	int		fd;
	char	host[64];
	int		port;
	int		connected;
} socket_t;

int socket_init(socket_t *sock, char *host, int port);
int socket_close(socket_t *sock);
int socket_diag(socket_t *sock);
int socket_write(socket_t *sock, char *data, int bytes);
int socket_read(socket_t *sock, char *data, int bytes);
int socket_send_packet(socket_t *sock, packet_t *pack);

int socket_keepalive(socket_t *sock, int keep_idle, int keep_intv, int keep_count);
int socket_listen(socket_t *sock); //Initialize the server
int socket_connect(socket_t *sock); //The client tries the server
void set_socket_rlimit(void); //set max open socket count

#endif

