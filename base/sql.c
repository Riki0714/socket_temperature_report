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
#include <string.h>

#include "sqlite3.h"
#include "sql.h"


#define LEN 256

int db_open(sqlite3 *db, char *dbname, char *tbName)
{
	int rv=-1;

	rv = sqlite3_open(dbname, &db);
	if(rv)
	{   
		printf("open database %s failure: %s\n", dbname, sqlite3_errmsg(db));
		sqlite3_close(db);
		return -12;
	}   

	tb_create(db, tbName, "id int, content char");
	return 0;
}

int db_close(sqlite3 *db)
{
	if( sqlite3_close(db) !=0 )
	{
		printf("close database failure!\n");
		return -1;
	}
	return 0;
}

int tb_create(sqlite3 *db, char *tbName, char *data)
{
	char    str_tmp[LEN]={0};
	char    str[LEN]={0};
	int     i=0;
	char	 *errmsg=NULL;

	snprintf(str_tmp, LEN-1, "CREATE TABLE %s", tbName);
	strncat(str_tmp, "(", sizeof(str_tmp)-strlen(str_tmp)   );  

	snprintf(str, LEN-1, "%s%s);", str_tmp, data);
	sqlite3_exec(db, str, NULL, NULL, &errmsg);

	return 0;
}

int db_remove(sqlite3 *db, char *tbName, char *data)
{
	char	str[LEN]={0};
	char	 *errmsg=NULL;

	snprintf(str, LEN-1, "DELETE FROM %s where %s", tbName, data);
	strncat(str, ";", sizeof(str)-strlen(str) );
	sqlite3_exec(db, str, NULL, NULL, &errmsg);

	return 0;
}

int db_insert(sqlite3 *db, char *tbName, char *data)
{	
	char	str[LEN]={0};
	char	 *errmsg=NULL;

	snprintf(str, LEN-1, "INSERT INTO %s VALUES(%s);", tbName, data);
	sqlite3_exec(db, str, NULL, NULL, &errmsg);

	return 0;
}


int sql_op(sqlite3 *db, char *tbName, int op, char *data)
{
	char  str[LEN]={0};
	char *errmsg=NULL;

	switch(op)
	{
		case CREATE:
		{
			char	str_tmp[LEN]={0};
			int		i=0;

			snprintf(str_tmp, LEN-1, "CREATE TABLE %s", tbName);
			strncat(  str_tmp, "(", sizeof(str_tmp)-strlen(str_tmp)   );

			snprintf(str, LEN-1, "%s%s);", str_tmp, data);
			sqlite3_exec(db, str, NULL, NULL, &errmsg);
			
			break;
		}

		case DROP:
			snprintf(str, LEN-1, "DROP TABLE %s;", tbName);
			sqlite3_exec(db, str, NULL, NULL, &errmsg);
			break;

		case INSERT:
			snprintf(str, LEN-1, "INSERT INTO %s VALUES(%s);", tbName, data);
			sqlite3_exec(db, str, NULL, NULL, &errmsg);
			break;

		case DELETE:
		{
			//char 	str_tmp[LEN]={0};

			snprintf(str, LEN-1, "DELETE FROM %s where %s", tbName, data);
			strncat(str, ";", sizeof(str)-strlen(str) );
			sqlite3_exec(db, str, NULL, NULL, &errmsg);
			break;
		}

		case SELECT:
		{
			char	  **azResult=NULL;
			int 		nrow, ncolumn;

			snprintf(str, LEN-1, "SELECT * FROM %s;", tbName);
			sqlite3_get_table(db, str, &azResult, &nrow, &ncolumn, &errmsg);
			//sqlite3_exec(db, str, NULL, NULL, &errmsg);

			printf("nrow=%d, ncolumn=%d\n", nrow, ncolumn);
			for(int i=0; i<(nrow+1)*ncolumn; i++)
			{
				printf("azResult[%d] = %s\n", i, azResult[i]);
			}
			sqlite3_free_table(azResult);

			break;
		}

		case UPDATE:
			snprintf(str, LEN-1, "UPDATE %s SET %s", tbName, data);
			sqlite3_exec(db, str, NULL, NULL, &errmsg);
			break;

		case FIND:
		{
			int rv=-1;
			char	  **azResult=NULL;
			int 		nrow, ncolumn;


			//snprintf(str, LEN-1, "SELECT COUNT(*) FROM sqlite_master wthere typr='table' and name='%s'", tbName);
			//rv = sqlite3_exec(db, str, NULL, NULL, &errmsg);
			snprintf(str, LEN-1, "SELECT * FROM %s;", tbName);
			rv = sqlite3_get_table(db, str, &azResult, &nrow, &ncolumn, &errmsg);
			//rv: 0 y   1 n
			if( rv==0 ) 
			{
				printf("This table already exists\n");
			}

			return rv;
			//printf("%d\n", rv);
			//break;
		}
	}

	return 0;
}

  /*  
int main()
{
	sqlite3 *db=NULL;
	int len=0;
	char	dbname[32]="haha.db";
	char	tbname[32]="table";
	char	str1[LEN] = {0};
	char	*errmsg=NULL;
	char    buf[32]="--haha--";
	char    buf1[32]={0};
	char	data[LEN] = "id int, name char";

	len = sqlite3_open(dbname, &db);
	printf("haha\n");

	sql_op(db, "table8", CREATE, "id int, name char");
	snprintf(buf1, sizeof(buf1), "1, '%s'", buf);
	printf("%s", buf1);
	sql_op(db, "table8", INSERT, buf1);
	//sql_op(db, "table8", INSERT, "1, '--haha--'");
	//sql_op(db, "table8", FIND, NULL);
	
	//snprintf(str1, LEN-1, "CREATE TABLE table2(%s);", data);
	//sqlite3_exec(db, str1, NULL, NULL, &errmsg);
	
	//snprintf(str1, LEN-1, "CREATE TABLE table2(%s);", data);
	//sqlite3_exec(db, str1, NULL, NULL, &errmsg);
			
	//sqlite3_exec(db, "CREATE TABLE table1(id int, name char);", NULL, NULL, &errmsg);
			
	//sql_op(db, tbname, CREATE, "ID INTEDER PRIMARY KEY, SensorID INTEGER");
	
	//sql_op(db, "table2", INSERT, "NULL, 2");
	//sql_op(db, "stu", DROP, NULL);
	//sql_op(db, "table2", DELETE, "name=2");

	sqlite3_close(db);
	return 0;
}
*/

