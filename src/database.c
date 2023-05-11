/*********************************************************************************
 *      Copyright:  (C) 2023 iot<iot@email.com>
 *                  All rights reserved.
 *
 *       Filename:  mySql.c
 *    Description:  This file 
 *                 
 *        Version:  1.0.0(03/26/23)
 *         Author:  iot <iot@email.com>
 *      ChangeLog:  1, Release initial version on "03/26/23 08:31:54"
 *                 
 ********************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sqlite3.h"
#include "packet.h"
#include "database.h"


#define LEN 			256
#define TABLE_NAME		"TEMP"

static sqlite3 *db;

int db_open(char *dbname)
{
	int		rv=-1;
	char	str_tmp[LEN] = {0};
	char	str[LEN] = {0};
	char	*errmsg = NULL;


	rv = sqlite3_open(dbname, &db);
	if( rv )
	{   
		printf("open database[%s] failure: %s\n", dbname, sqlite3_errmsg(db));
		sqlite3_close(db);
		return -1;
	}   
	
	snprintf(str_tmp, LEN-1, "CREATE TABLE IF NOT EXISTS %s(DEVSN CHAR, TIME CHAR, TEMPER FLOAT);", TABLE_NAME);
	rv = sqlite3_exec(db, str_tmp, NULL, NULL, &errmsg);
	if( rv )
	{
		printf("create or open table failure: %s\n", errmsg);
		sqlite3_free(errmsg);
		return -2;
	}

	return 0;
}

int db_insert(packet_t *pack)
{
	char	str_tmp[LEN] = {0};
	char	*errmsg = NULL;
	int 	rv = -1;

	snprintf(str_tmp, LEN-1, "INSERT INTO %s(DEVSN, TIME, TEMPER) VALUES('%s', '%s', '%.2f');", TABLE_NAME, pack->devsn, pack->time, pack->temper);
	
	rv = sqlite3_exec(db, str_tmp, NULL, NULL, &errmsg);
	if( rv )
	{
		printf("insert data into table[%s] failure: %s\n", TABLE_NAME, errmsg);
		sqlite3_free(errmsg);
		return -1;
	}
			
	return 0;
}

int db_query(packet_t *pack)
{
	char	str_tmp[LEN] = {0};
	char	*errmsg = NULL;
	char	**query_result = NULL;
	int		nrow = 0;
	int 	ncolumn = 0;
	int 	rv = -1;

	snprintf(str_tmp, LEN-1, "SELECT * FROM  %s WHERE TIME IN (SELECT TIME FROM %s ORDER BY TIME ASC LIMIT 1);", TABLE_NAME, TABLE_NAME);
	rv = sqlite3_get_table(db, str_tmp, &query_result, &nrow, &ncolumn, &errmsg);

	if( rv!=0 )
	{
		printf("insert data into table[%s] failure: %s\n", TABLE_NAME, errmsg);
		sqlite3_free(errmsg);
		return QUERY_ERROR;
	}
	if( nrow==0 )
	{
		printf("there is no data in the database!\n");
		return QUERY_NULL;
	}

	//for(int i=3; i<(nrow+1)*ncolumn; i++)
	strncpy(pack->devsn, query_result[3], 32);
	strncpy(pack->time, query_result[4], 32);
	pack->temper = atof(query_result[5]);

	return 0;
}

int db_close()
{
	int rv = -1;

	while(1)
	{
		rv = sqlite3_close(db);
		if( rv==SQLITE_BUSY )
		{
			printf("There are still queries not completed.Disable closing!\n");
			continue;
		}
		else if( rv )
		{
			printf("close database[%s] failure: %s\n", sqlite3_errmsg(db));
			return -1;
		}
		else 
			break;
	}
	return 0;
}

int db_remove()
{
	char	str_tmp[LEN] = {0};
	char	*errmsg = NULL;
	int		rv = -1;

	snprintf(str_tmp, LEN-1, "DELETE FROM %s WHERE TIME IN (SELECT TIME FROM %s ORDER BY TIME ASC limit 1);", TABLE_NAME, TABLE_NAME);
	
	rv = sqlite3_exec(db, str_tmp, NULL, NULL, &errmsg);
	if( rv )
	{
		printf("delete data from database table[%s] failure: %s\n", TABLE_NAME, errmsg);
		sqlite3_free(errmsg);
		return -1;
	}

	return 0;
}

/*  
int main()
{
	char		*dbname="test.db";
	char		*tbname="TEMP";
	sqlite3 	*db=NULL;
	packet_t	pack;
	int			rv=-1;
	char		str[LEN]={0};

	memset(&pack, 0, sizeof(packet_t));
	strncpy(pack.devsn, "d20211", TEMP_STR_LEN);
	strncpy(pack.time, "2002", TEMP_STR_LEN);
	pack.temper = 22.99;

	rv = db_open(&db, dbname, tbname);
	//sqlite3_open(dbname, &db);
	//tb_create(db, tbname);
	//sql_op(db, tbname, CREATE, "DEVSN char, TIME char, TEMPER float");
	printf("sa\n");
	db_insert(db, tbname, &pack);

	strncpy(pack.time, "2005", TEMP_STR_LEN);
	pack.temper = 30.24;
	db_insert(db, tbname, &pack);

	db_query(db, tbname, &pack);
	//db_remove(db, tbname);

	return 0;
}
*/
