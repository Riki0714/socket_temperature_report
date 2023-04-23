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
	snprintf(str, size-1, "--%s--%s--%.2f", pack->devsn, pack->time, pack->temper);
	strncpy(buf, str, size-1);

	return 0;
}


