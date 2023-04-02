/*********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  alarm_test_back.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(03/22/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "03/22/23 14:20:16"
 *                 
 ********************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

void sig_alarm(int sig)
{
	printf("haha!\n");
	alarm(1);
}

int main()
{
	signal(SIGALRM, sig_alarm);  
	alarm(5);
	getchar();
	sleep(3);
	printf("heihei\n");

	return 0;
}

