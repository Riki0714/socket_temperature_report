/*********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  getTime.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(02/28/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "02/28/23 13:56:56"
 *                 
 ********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "get_time.h"

int get_time(char *time_str, int size)
{
	time_t 			time_stamp;
    struct tm  		*tm = NULL;

    time_stamp = time((time_t *)NULL);
	//time(&time_stamp);
	tm = localtime(&time_stamp); 
					
	if( tm!=NULL )
		strftime(time_str, size, "%Y-%m-%d-%X", tm);
		//strftime(time_str, size, "%Y-%m-%x-%X", tm);
	else
		return -1;

	return 1;
}

int get_time_pipe(char *buf, int size)
{
	FILE 		*fp = NULL;

	if ( !(fp = popen("date","r")) )
	{
		printf("popen error!\n");
		return -1;
	}

	//fgets(buf, sizeof(buf), fp);
	fgets(buf, size, fp);
	//printf("%s", buf);

	return 1;
}

/*
#include <unistd.h>
int main()
{
	char	time[32]={0};

//	while(1)
//	{
		get_time(time, 32);
		printf("%s", time);

//		sleep(2);
//	}
	return 0;
}
*/
