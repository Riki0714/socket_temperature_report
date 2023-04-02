/*********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  get_temp.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(02/26/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "02/26/23 11:59:46"
 *                 
 ********************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include "get_temp.h"
#include "getTime.h"

//#define CONFIG_DEBUG
#include "myDebug.h"


#define W1_PATH 		"/sys/bus/w1/devices/"
#define PATH_OTHER  	"/w1_slave"
#define KEY_FILE_NAME   "28-"
#define KEY_CONTENT   	"t-"
#define LEN_1			32
#define LEN_2			64
#define LEN_3			128
#define LEN_4			256

#define OPEN_DIR_ERROR 	 -2
#define FOUND_ERROR      -3

//char sn_name[64];

int get_name(char *serialNum, int len)
{
	char			w1_path[64] = W1_PATH;
	char			path_other[64] = PATH_OTHER;
	DIR		 		*dirFd = NULL;
	struct dirent  	*dirRead = NULL;
	int				found = 0;

	dbg_print("%d\n", sizeof(serialNum));  //4

	dirFd = opendir(w1_path);
	if( !dirFd )
	{
		printf("open dir error!\n");
		return OPEN_DIR_ERROR;
	}

	while( NULL != ( dirRead=(readdir(dirFd)) ) ) //find file
	{
		//if( strstr(dirRead->d_name, "28-") )
		if( strstr(dirRead->d_name, KEY_FILE_NAME) )
		{
			memset(serialNum, 0, len);
			strncpy(serialNum, dirRead->d_name, len-1);
			found = 1;
		}
	}

	if( !found )   
	{
		printf("Sorry!The file was not found.\n");
		closedir(dirFd);

		return FOUND_ERROR;
	}

	closedir(dirFd); //!!!
	return 1;
}

int get_file_content(char *allPath, char *content, int conLen)
{
	int		 fd = -1;
	char     buf[128];
	char	*ptr = NULL;

	if( (fd=open(allPath, O_RDONLY)) < 0 )
	{
		printf("open file failure: %s\n", strerror(errno));
		return -4;
	}
	dbg_print("open file successfully!%d\n", fd);

	memset(buf, 0, sizeof(buf));
	if( (read(fd, buf, sizeof(buf))) < 0  )
	{
		printf("read data failure: %s\n", strerror(errno));
		return -5;
	}
	dbg_print("read file successfully!%d\n", fd);

	ptr = strstr(buf, "t=");
	if( !ptr )
	{
		printf("can't find the string!\n");
		return -6;
	}

	ptr += 2;  //skip "t="
	memset(content, 0, conLen);
	dbg_print("%s\n", ptr);

	//content = ptr;
	strncpy(content, ptr, conLen-1);
	dbg_print("%s\n", content);

	dbg_print("get content successfully!%d\n", fd);

	return 1;
}


int get_temp_str(char *buf, int len)
{
	char 	name[LEN_2] = {0};
	char	allPath[LEN_3] = W1_PATH;
	char	content[LEN_2] = {0};
	char	time[LEN_3] = {0};
	double	tempF = 0;
	char	str[LEN_4] = {0};
	int		timeLen = 0;

	//memset(name, 0, sizeof(name));
	if( get_name(name, 64)<0 )  
	{
		printf("get name error!\n");
		return -3;
	}

	//strcat(allPath, name);
	strncat(allPath, name, (sizeof(allPath)-1-strlen(allPath)) );
	strncat(allPath, PATH_OTHER, (sizeof(allPath)-1-strlen(allPath)) );
	dbg_print("%s\n", allPath);

	get_file_content(allPath, content, 64);
	dbg_print("%s\n", content);
	tempF = atof(content) / 1000;

	get_time_pipe(time, 128);
	timeLen = strlen(time);
	time[timeLen-1] = '\0';
	dbg_print("%s\n", time);

	memset(buf, 0, len);
	//snprintf(str, len-1, "--Serial number: %s\n--Sampling time: %s--Temperature value: %.2f\n", name, time, tempF);
	snprintf(str, len-1, "--%s--%s--%.2f", name, time, tempF);
	dbg_print("%s\n", str);

	strncpy(buf, str, len-1);

	return 1;
}

/*  
int main()
{
	char 	name[LEN_2] = {0};
	char	allPath[LEN_3] = W1_PATH;
	char	content[LEN_2] = {0};
	char	time[LEN_3] = {0};
	double	tempF = 0;
	char	str[LEN_4] = {0};

	//memset(name, 0, sizeof(name));
	if( getName(name, 64)<0 )  
	{
		printf("get name error!\n");
		return -3;
	}

	//strcat(allPath, name);
	strncat(allPath, name, (sizeof(allPath)-1-strlen(allPath)) );
	strncat(allPath, PATH_OTHER, (sizeof(allPath)-1-strlen(allPath)) );
	dbg_print("%s\n", allPath);

	get_file_content(allPath, content, 64);
	dbg_print("%s\n", content);
	tempF = atof(content) / 1000;

	get_time_pipe(time, 128);
	dbg_print("%s\n", time);

	//memset(buf, 0, len);
	snprintf(str, 256-1, "--Serial number: %s\n--Sampling time: %s--Temperature value: %.2f\n", name, time, tempF);
	printf("%s\n", str);

	return 0;
}
*/



