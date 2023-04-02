/*********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  myDns.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(03/19/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "03/19/23 09:32:55"
 *                 
 ********************************************************************************/

#include <sys/socket.h>
#include <stddef.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "myDns.h"

#define IP_SIZE 64

char *get_ip_dns(char *doname, int len)
{
	struct addrinfo 	hints, *listp, *p;
	//char 				*buf;
	char *buf;
	int 				rv=-1;
	int					falgs = NI_NUMERICHOST;
	//struct sockaddr_in		*sear;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = 0;
	hints.ai_protocol = 0;
	hints.ai_flags = 0;

	if( ( rv=(getaddrinfo(doname, NULL, &hints, &listp)) ) != 0 )
	{
		printf("getaddrinfo error: %s", gai_strerror(rv));
	}

	p = listp;
	//for( p=listp; NULL!=p; p=p->ai_next )
	{
		getnameinfo(p->ai_addr, p->ai_addrlen, buf, len, NULL, 0, falgs);
		//printf("%s\n", buf);

		//sear = (struct sockaddr_in *)p->ai_addr;
		//printf("%s\n", inet_ntoa(sear->sin_addr));

		//printf("%s\n", *(listp->ai_addr)->sa_data);

	}

	//printf("%s\n", buf[1]);

	freeaddrinfo(listp);
	return  buf;
}

/*  
int get_ip_dns(char *doname, int len)
{
	struct addrinfo 	hints, *listp, *p;
	//char 				*buf;
	//char * buf[10]={0};
	char *buf;
	int 				rv=-1;
	int					falgs = NI_NUMERICHOST;
	//struct sockaddr_in		*sear;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = 0;
	hints.ai_protocol = 0;
	hints.ai_flags = 0;

	if( ( rv=(getaddrinfo(doname, NULL, &hints, &listp)) ) != 0 )
	{
		printf("getaddrinfo error: %s", gai_strerror(rv));
	}

	int i=0;
	//p = listp;
	for( p=listp; NULL!=p; p=p->ai_next )
	{
		getnameinfo(p->ai_addr, p->ai_addrlen, buf, len, NULL, 0, falgs);
		i++;
		printf("%s\n", buf);

		//sear = (struct sockaddr_in *)p->ai_addr;
		//printf("%s\n", inet_ntoa(sear->sin_addr));

		//printf("%s\n", *(listp->ai_addr)->sa_data);

	}

	//printf("%s\n", buf[1]);

	freeaddrinfo(listp);
	return  0;
}
*/

/*  
int main()
{
	char *buf;
	int len=64;

	buf = get_ip_dns("baidu.com", len);
	printf("%s\n", buf);

	return 0;
}
*/
