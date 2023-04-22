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

#include "get_temp.h"
#include "get_time.h"

//#define CONFIG_DEBUG
#include "debug.h"


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

	dbg_print("get temperature successfully!%d\n", fd);

	return 1;
}

int sample_temperature(char *buf, int size)
{
	char 		devsn[LEN_2] = {0};
	char		all_path[LEN_3] = W1_PATH;
	char		time[LEN_3] = {0};
	float		temperature = 0;
	char		str[LEN_4] = {0};
	int			timeLen = 0;
	packet_t	pack;

	if( get_devsn(devsn, 64)<0 )  
	{
		printf("get device serial number error!\n");
		return -3;
	}
	strncat(all_path, devsn, (sizeof(all_path)-1-strlen(all_path)) );
	strncat(all_path, PATH_OTHER, (sizeof(all_path)-1-strlen(all_path)) );

	if(ds18b20_get_temperature(all_path, &temperature, 64)<0)
	{
		printf("get temperature failure\n");
		return -4;
	}

	if( get_time(time, LEN_1) < 0 )
	{
		printf("get time failure!\n");
		return -5;
	}

	pack_data(devsn, time, temperature, buf, size);

	return 1;
}

int pack_data(char *devsn, char *time, float temper, char *buf, int size)
{
	char	str[LEN_4] = {0};

	memset(buf, 0, size);
	snprintf(str, size-1, "--%s--%s--%.2f", pack->devsn, pack->time, pack->temper);
	strncpy(buf, str, size-1);

	return 0;
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



