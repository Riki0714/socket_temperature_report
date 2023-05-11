/*********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  get_temp.c
 *    Description:  Gets the temperature and passes it back as a structure or string 
 *                 
 *        Version:  1.2.3(03/15/23)
 *         Author:  xiao
 *      ChangeLog:  1, Release initial version on "03/15/23 11:59:46"
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

#include "ds18b20.h"
#include "packet.h"
#include "get_time.h"

//#define CONFIG_DEBUG
#include "debug.h"


#define W1_PATH 		"/sys/bus/w1/devices/"
#define PATH_OTHER  	"/w1_slave"
#define KEY_FILE_NAME   "28-"
#define KEY_CONTENT   	"t-"

#define OPEN_DIR_ERROR 	 -2
#define FOUND_ERROR      -3


int get_devsn(char *serial_number, int size)
{
	char			w1_path[64] = W1_PATH;
	char			path_other[64] = PATH_OTHER;
	DIR		 		*dirFd = NULL;
	struct dirent  	*dirRead = NULL;
	int				found = 0;

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
			memset(serial_number, 0, size);
			strncpy(serial_number, dirRead->d_name, size-1);
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

int ds18b20_get_temperature(char *all_path, float *temperature, int size)
{
	int		 fd = -1;
	char     buf[128];
	char	*ptr = NULL;

	if( (fd=open(all_path, O_RDONLY)) < 0 )
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
	dbg_print("%s\n", ptr);

	*temperature = atof(ptr)/1000;
	dbg_print("%lf\n", temperature);

	return 1;
}

int sample_temperature(packet_t *pack)
{
	char		all_path[128] = W1_PATH;

	memset(pack, 0, sizeof(packet_t));
	if( get_devsn(pack->devsn, TEMP_STR_LEN)<0 )  
	{
		printf("get device serial number error!\n");
		return -3;
	}

	strncat(all_path, pack->devsn, (sizeof(all_path)-1-strlen(all_path)) );
	strncat(all_path, PATH_OTHER, (sizeof(all_path)-1-strlen(all_path)) );
	if(ds18b20_get_temperature(all_path, &(pack->temper), TEMP_STR_LEN)<0)
	{
		printf("get temperature failure\n");
		return -4;
	}

	if( get_time(pack->time, TEMP_STR_LEN) < 0 )
	{
		printf("get time failure!\n");
		return -5;
	}

	return 1;
}


