/********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  mySql.h
 *    Description:  This file 
 *
 *        Version:  1.0.0(03/26/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "03/26/23 08:34:57"
 *                 
 ********************************************************************************/

#ifndef _MYSQL_H
#define _MYSQL_H

#include "sqlite3.h"

#define	CREATE  1
#define DROP	2
#define INSERT	3
#define DELETE	4
#define SELECT	5
#define UPDATE	6
#define FIND	7


int db_open(sqlite3 **db, char *dbname, char *tbname);
int tb_create(sqlite3 *db, char *tbName);
int db_insert(sqlite3 *db, char *tbName, packet_t *pack);
int db_query(sqlite3 *db, char *tbName, packet_t *pack);
int db_remove(sqlite3 *db, char *tbName);
int db_close(sqlite3 *db);
int sql_op(sqlite3 *db, char *tbName, int op, char *data);

#endif


