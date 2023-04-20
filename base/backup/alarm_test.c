/*********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  alarm_test.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(03/22/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "03/22/23 14:04:13"
 *                 
 ********************************************************************************/

#include <stdio.h>
#include <signal.h>
#include <unistd.h>

void timer(int sig)
{
	if(SIGALRM == sig)
	{
		printf("hello\n");
		alarm(3);
		printf("xixi\n");
	}
	return ;
}

int main()
{
	signal(SIGALRM, timer);

	alarm(1);
	printf("bye\n");

	getchar();

	return 0;
}


