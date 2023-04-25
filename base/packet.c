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

//#define CONFIG_DEBUG
#include "debug.h"

int pack_data(packet_t *pack, char *buf, int size)
{
	char	str[64] = {0};

	memset(buf, 0, size);
	snprintf(str, size-1, "%s|%s|%.2f", pack->devsn, pack->time, pack->temper);
	strncpy(buf, str, size-1);

	return 0;
}

int	unpack_data(packet_t *pack, char *data, int size)
{
	char		*data_tmp=data;
	char		*ptr=NULL;
	int			n = 0;
	int			i = 0;

	memset(pack, 0, sizeof(packet_t));
	while(i<2)
	{
		ptr = strstr(data_tmp, "|");
		if( !ptr )
		{
			printf("can't find the string!\n");
			return -1;
		}

		n = ptr-data_tmp;

		if( !i )
			strncpy(pack->devsn, data_tmp, n);
		else
			strncpy(pack->time, data_tmp, n);

		data_tmp = ptr+1;  //skip '|'
		i++;
	}

	pack->temper = atof(data_tmp);

	return 0;
}

/*  
int main()
{
	packet_t	pack;
	char		buf[64]="asad2131|2005-02-21|23.894";

	unpack_data(&pack, buf, 64);

	printf("%s,%s,%f\n", pack.devsn, pack.time, pack.temper);
	return 0;
}
*/

