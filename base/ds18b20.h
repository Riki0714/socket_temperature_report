/********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  get_temp.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(02/26/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "02/26/23 16:40:58"
 *                 
 ********************************************************************************/

#ifndef _GET_TEMP_H
#define _GET_TEMP_H

#define TEMP_STR_LEN  32

typedef struct		packet_s{
	char		devsn[TEMP_STR_LEN];
	char		time[TEMP_STR_LEN];
	float		temper;
} packet_t;

int sample_temperature(packet_t *pack);
int ds18b20_get_temperature(char *all_path, float *temperature, int size);
int get_devsn(char *serial_number, int size);

#endif

