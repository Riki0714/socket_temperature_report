/*********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  myTimer.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(03/23/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "03/23/23 14:45:43"
 *                 
 ********************************************************************************/

#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

void timer_s(unsigned long   s, unsigned long   us)
{
	struct timeval		tv;

	tv.tv_sec = s;
	tv.tv_usec = us;

	select(0, NULL, NULL, NULL, &tv);
}


//static inline void msleep(unsiged long   ms)
void msleep(unsigned long   ms)
{
	struct timeval		tv;

	tv.tv_sec = ms/1000;
	tv.tv_usec = (ms%1000)*1000;

	select(0, NULL, NULL, NULL, &tv);
}
