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

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

int get_time_func(char *buf1, int len1, char *buf2, int len2)
{
	time_t t;
    struct tm *tmp,*tmp2;
    //char buf1[64], buf2[64];

    time(&t);
					      
	tmp = localtime(&t);
	strftime(buf1, 64, "time and date: %F %T", tmp);
	//printf("%s\n", buf1);

	tmp2 = gmtime(&t);
	strftime(buf2, 64, "time and date: %F %T", tmp2);
	//printf("%s\n", buf2);

	return 1;
}

int get_time_pipe(char *buf, int len)
{
	FILE 		*fp = NULL;

	if ( !(fp = popen("date","r")) )
	{
		printf("popen error!\n");
		return -1;
	}

	//fgets(buf, sizeof(buf), fp);
	fgets(buf, len, fp);
	//printf("%s", buf);

	return 1;
}


