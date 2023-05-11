/********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  ds18b20.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(02/26/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "02/26/23 16:40:58"
 *                 
 ********************************************************************************/

#ifndef _DS18B20_H
#define _DS18B20_H

#include "packet.h"

int sample_temperature(packet_t *pack);
int ds18b20_get_temperature(char *all_path, float *temperature, int size);
int get_devsn(char *serial_number, int size);

#endif

