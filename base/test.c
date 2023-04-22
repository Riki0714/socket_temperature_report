/*********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  test.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(04/22/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "04/22/23 14:02:27"
 *                 
 ********************************************************************************/
#include <stdio.h>
#include <time.h>

 #include <sys/socket.h>
       #include <netinet/in.h>
       #include <arpa/inet.h>


int main()
{
	char *ip="110.242.68.66";
	char *buf=NULL;
	struct sockaddr_in src;
	int rv=-1;
	
	//in_addr_t addr=inet_addr("110.242.68.66");

	//rv = inet_pton(AF_INET, ip, buf);
	//inet_aton(ip, &src.sin_addr);

	//printf("%d\n", addr);
	//
	
	int s = time((time_t *)NULL);
	int s_now=s;

	printf("%d\n", s);
/*
	while(1)
	{
		s_now = time((time_t *)NULL);
		if( (s_now - s) > 5)
		{
			s = s_now;
		  	printf("333\n");
		}
	}
	return 0;*/
}

