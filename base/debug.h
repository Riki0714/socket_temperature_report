/********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  myDebug.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(02/28/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "02/28/23 08:55:22"
 *                 
 ********************************************************************************/

#ifndef _MYDEBUG_H
#define _MYDEBUG_H

//#define CONFIG_DEBUG
#ifdef  CONFIG_DEBUG
#define dbg_print(format, args...) printf(format, args)
#else
#define dbg_print(format, args...) do{} while(0)
#endif


#endif

