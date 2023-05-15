/********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  packet.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(02/26/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "02/26/23 16:40:58"
 *                 
 ********************************************************************************/

#ifndef _PACKET_H
#define _PACKET_H

#define DEVSN_SIZE	16
#define TIME_SIZE	10

typedef struct  packet_s{
	char        devsn[DEVSN_SIZE];
	char        time[TIME_SIZE];
	float       temper;
} packet_t;

int pack_data(packet_t *pack, char *buf, int size);
int unpack_data(packet_t *pack, char *data, int size);

#endif

