/*
 * database.c
 *
 *  Created on: 2018-9-14
 *      Author: xm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <unistd.h>
#include "database.h"

#define DB_NAME "gateway.db"
#define WAIT_FOR_IDEL_TIMES    1000
#define WAIT_FOR_IDLE	usleep(10000)

/*
 * 数据库初始化，创建表
 */
int databaseInit(void)
{
	sqlite3 *db;
	char *zErrMsg = 0;
	char sql[1024] = {0};
	int rc, state = 0;

	rc = sqlite3_open(DB_NAME, &db);
	if(rc)
	{
		printf("Can't open database file: %s.\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return -1;
	}

	/*
	 * 设备ID数据表
	 */
	sprintf(sql,"create table devId("
			"devId text not null);");
	rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
	if(rc)
	{
		printf("fail to create devId table: %s.\n", sqlite3_errmsg(db));
		sqlite3_free(zErrMsg);
		state = -1;
	}

	/*
	 * 设备配置数据
	 */
	sprintf(sql,"create table gatewayCfg("
			"netType int not null,"
			"plcType int not null,"
			"netIp text not null,"
			"netMask text not null,"
			"netGateway text not null,"
			"srvAddress text not null,"
			"plcIp text not null,"
			"plcRack int not null,"
			"plcSlot int not null);");
	rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
	if(rc)
	{
		printf("fail to create gatewayCfg table: %s.\n", sqlite3_errmsg(db));
		sqlite3_free(zErrMsg);
		state = -1;
	}

	/*
	 * 点表数据表
	 */
	sprintf(sql,"create table pointTable("
			"id int primary key not null,"
			"address text not null,"
			"datatype text not null,"
			"deviation text not null,"
			"name text not null,"
			"report_interval text not null);");
	rc = sqlite3_exec(db, sql, NULL, 0, &zErrMsg);
	if(rc)
	{
		printf("fail to create pointTable table: %s.\n", sqlite3_errmsg(db));
		sqlite3_free(zErrMsg);
		state = -1;
	}

	sqlite3_close(db);

	return state;
}

/*
 * 写入默认设备ID，正式生产通过数据库修改设备ID（暂行办法）
 */
int databaseDefaultDevId(void)
{
	sqlite3 *db;
	char *zErrMsg = 0;
	char sql[1024] = {0};
	int rc;
	int nrow = 0, ncolumn = 0;
	char **azResult; //二维数组存放结果

	rc = sqlite3_open(DB_NAME, &db);
	if(rc)
	{
		printf("Can't open database file: %s.\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return -1;
	}

	int n =WAIT_FOR_IDEL_TIMES;
	do
	{
		// 开始一个事务EXCLUSIVE，保证不会被别的读打断，造成commit错误
		rc = sqlite3_exec(db,"BEGIN EXCLUSIVE ", NULL,NULL, &zErrMsg);

		if( rc != SQLITE_OK )
		{
			sqlite3_free(zErrMsg);
			n--;

			if (n == 0)
			{
				printf("timeout\n");
				sqlite3_close(db);
				return -1;
			}
			if (rc == SQLITE_BUSY)
			{
				printf("write busy\n");
				WAIT_FOR_IDLE; // 睡眠10ms
				continue;
			}
		}
		else
		{
			break;
		}

	}while(1);

	//判断表是否为空
	sprintf(sql, "SELECT *FROM devId;");
	rc = sqlite3_get_table(db, sql, &azResult, &nrow, &ncolumn, &zErrMsg);

	if (rc != SQLITE_OK)
	{
		printf("zErrMsg = %s \n", zErrMsg);
		sqlite3_free(zErrMsg);
		sqlite3_free_table(azResult);
		sqlite3_close(db);
		return -0x04;
	}

	if ((nrow == 0) && (ncolumn == 0))
	{
		// 插入数据
		sprintf(sql, "INSERT INTO devId(devId)"
			   "VALUES('83776582848783780099');");
		rc = sqlite3_exec( db , sql , 0 , 0 , &zErrMsg );

		if (rc != SQLITE_OK)
		{
			printf("insert error: %s\n", zErrMsg);
			sqlite3_free(zErrMsg);
			rc = sqlite3_exec(db,"ROLLBACK", NULL,NULL, &zErrMsg);	//	插入错误则回滚

			if (rc != SQLITE_OK)
			{
				sqlite3_free(zErrMsg);
			}
		}
	}
	sqlite3_free_table(azResult);
	rc=sqlite3_exec(db,"COMMIT", NULL,NULL, &zErrMsg);

	if( rc != SQLITE_OK )
	{
		printf("SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		sqlite3_close(db); //关闭数据库
		return -1;
	}

	sqlite3_close(db); //关闭数据库

	return 0;
}

/*
 * 读设备ID号
 */
int databaseReadDevId(char *devId)
{
	sqlite3 *db;
	char *zErrMsg = 0;
	char sql[1024] = {0};
	int rc;
	int nrow = 0, ncolumn = 0;
	char **azResult; //二维数组存放结果

	if(devId == NULL)
		return -1;

	rc = sqlite3_open(DB_NAME, &db);
	if(rc)
	{
		printf("Can't open database file: %s.\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return -1;
	}

	int n =WAIT_FOR_IDEL_TIMES;
	do
	{
		// 开始一个事务EXCLUSIVE，保证不会被别的读打断，造成commit错误
		rc = sqlite3_exec(db,"BEGIN IMMEDIATE ", NULL,NULL, &zErrMsg);

		if( rc != SQLITE_OK )
		{
			sqlite3_free(zErrMsg);
			n--;

			if (n == 0)
			{
				printf("timeout\n");
				sqlite3_close(db);
				return -1;
			}
			if (rc == SQLITE_BUSY)
			{
				printf("write busy\n");
				WAIT_FOR_IDLE; // 睡眠10ms
				sleep(1);
				continue;
			}
		}
		else
		{
			break;
		}

	}while(1);

	//读取
	sprintf(sql, "SELECT *FROM devId;");
	rc = sqlite3_get_table(db, sql, &azResult, &nrow, &ncolumn, &zErrMsg);
	if (rc != SQLITE_OK)
	{
		printf("zErrMsg = %s \n", zErrMsg);
		sqlite3_free(zErrMsg);
		sqlite3_free_table(azResult);
		sqlite3_close(db);
		return -1;
	}

	if ((nrow == 0) && (ncolumn == 0))
	{
		sqlite3_close(db); //关闭数据库
		return -1;
	}

	memcpy(devId, azResult[ncolumn], strlen(azResult[ncolumn]) + 1);
//	printf("devId:%s\n", devId);

	sqlite3_free_table(azResult);
	rc=sqlite3_exec(db,"COMMIT", NULL,NULL, &zErrMsg);

	if( rc != SQLITE_OK )
	{
		printf("SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		sqlite3_close(db); //关闭数据库
		return -1;
	}

	sqlite3_close(db); //关闭数据库

	return 0;
}

/*
 * 写配置信息
 */
int databaseWriteGateWayCfg(gatewayCfgInfo_t *gcf)
{
	sqlite3 *db;
	char *zErrMsg = 0;
	char sql[1024] = {0};
	int rc;

	if(gcf == NULL)
		return -1;

	rc = sqlite3_open(DB_NAME, &db);
	if(rc)
	{
		printf("Can't open database file: %s.\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return -1;
	}

	int n =WAIT_FOR_IDEL_TIMES;
	do
	{
		// 开始一个事务EXCLUSIVE，保证不会被别的读打断，造成commit错误
		rc = sqlite3_exec(db,"BEGIN EXCLUSIVE ", NULL,NULL, &zErrMsg);

		if( rc != SQLITE_OK )
		{
			sqlite3_free(zErrMsg);
			n--;

			if (n == 0)
			{
				printf("timeout\n");
				sqlite3_close(db);
				return -1;
			}
			if (rc == SQLITE_BUSY)
			{
				printf("write busy\n");
				WAIT_FOR_IDLE; // 睡眠10ms
				sleep(1);
				continue;
			}
		}
		else
		{
			break;
		}

	}while(1);

	//删除所有数据
	sprintf(sql,"delete from gatewayCfg;");
	rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
	if(rc)
	{
		printf("fail to delete values: %s\n", sqlite3_errmsg(db));
		sqlite3_free(zErrMsg);
		sqlite3_close(db); //关闭数据库
		return -1;
	}

	sprintf(sql,"insert into gatewayCfg values(%d,%d,'%s','%s','%s','%s','%s',%d,%d);",
			gcf->netType,
			gcf->plcType,
			gcf->netIp,
			gcf->netMask,
			gcf->netGateway,
			gcf->srvAddress,
			gcf->plcIp,
			gcf->plcRack,
			gcf->plcSlot);
	puts(sql);
	rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
	if(rc)
	{
		printf("fail to insert values: %s\n", sqlite3_errmsg(db));
		sqlite3_free(zErrMsg);
		sqlite3_close(db); //关闭数据库
		return -1;
	}

	rc=sqlite3_exec(db,"COMMIT", NULL,NULL, &zErrMsg);
	if( rc != SQLITE_OK )
	{
		printf("SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		sqlite3_close(db); //关闭数据库
		return -1;
	}

	sqlite3_close(db); //关闭数据库

	return 0;
}

/*
 * 读配置信息
 */
int databaseReadGatewayCfg(gatewayCfgInfo_t *gcf)
{
	sqlite3 *db;
	char *zErrMsg = 0;
	char sql[1024] = {0};
	int rc;
	int nrow = 0, ncolumn = 0;
	char **azResult; //二维数组存放结果

	if(gcf == NULL)
		return -1;

	rc = sqlite3_open(DB_NAME, &db);
	if(rc)
	{
		printf("Can't open database file: %s.\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return -1;
	}

	int n =WAIT_FOR_IDEL_TIMES;
	do
	{
		// 开始一个事务EXCLUSIVE，保证不会被别的读打断，造成commit错误
		rc = sqlite3_exec(db,"BEGIN IMMEDIATE ", NULL,NULL, &zErrMsg);

		if( rc != SQLITE_OK )
		{
			sqlite3_free(zErrMsg);
			n--;

			if (n == 0)
			{
				printf("timeout\n");
				sqlite3_close(db);
				return -1;
			}
			if (rc == SQLITE_BUSY)
			{
				printf("write busy\n");
				WAIT_FOR_IDLE; // 睡眠10ms
				sleep(1);
				continue;
			}
		}
		else
		{
			break;
		}

	}while(1);

	//读取
	sprintf(sql, "SELECT *FROM gatewayCfg;");
	rc = sqlite3_get_table(db, sql, &azResult, &nrow, &ncolumn, &zErrMsg);
	if (rc != SQLITE_OK)
	{
		printf("zErrMsg = %s \n", zErrMsg);
		sqlite3_free(zErrMsg);
		sqlite3_free_table(azResult);
		sqlite3_close(db);
		return -1;
	}

	if ((nrow == 0) && (ncolumn == 0))
	{
		sqlite3_close(db); //关闭数据库
		return -1;
	}

	int tempData;
	sscanf(azResult[ncolumn], "%d", &tempData);
	gcf->netType = tempData&0xff;
	sscanf(azResult[ncolumn+1], "%d", &tempData);
	gcf->plcType = tempData&0xff;
	memcpy(gcf->netIp, azResult[ncolumn+2], strlen(azResult[ncolumn+2]) + 1);
	memcpy(gcf->netMask, azResult[ncolumn+3], strlen(azResult[ncolumn+3]) + 1);
	memcpy(gcf->netGateway, azResult[ncolumn+4], strlen(azResult[ncolumn+4]) + 1);
	memcpy(gcf->srvAddress, azResult[ncolumn+5], strlen(azResult[ncolumn+5]) + 1);
	memcpy(gcf->plcIp, azResult[ncolumn+6], strlen(azResult[ncolumn+6]) + 1);
	sscanf(azResult[ncolumn+7], "%d", &tempData);
	gcf->plcRack = tempData;
	sscanf(azResult[ncolumn+8], "%d", &tempData);
	gcf->plcSlot = tempData;

//	printf("gcf->netType:%d\n", gcf->netType);
//	printf("gcf->plcType:%d\n", gcf->plcType);
//	printf("gcf->netIp:%s\n", gcf->netIp);
//	printf("gcf->netMask:%s\n", gcf->netMask);
//	printf("gcf->netGateway:%s\n", gcf->netGateway);
//	printf("gcf->srvAddress:%s\n", gcf->srvAddress);
//	printf("gcf->plcIp:%s\n", gcf->plcIp);
//	printf("gcf->plcRack:%d\n", gcf->plcRack);
//	printf("gcf->plcSlot:%d\n", gcf->plcSlot);

	sqlite3_free_table(azResult);
	rc=sqlite3_exec(db,"COMMIT", NULL,NULL, &zErrMsg);

	if( rc != SQLITE_OK )
	{
		printf("SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		sqlite3_close(db); //关闭数据库
		return -1;
	}

	sqlite3_close(db); //关闭数据库

	return 0;
}

/*
 * 写点表信息
 */
int databaseWritePointTable(pointTable_t *pt)
{
	sqlite3 *db;
	char *zErrMsg = 0;
	char sql[1024] = {0};
	int rc;
	int ptIndex;

	if(pt == NULL)
		return -1;

	rc = sqlite3_open(DB_NAME, &db);
	if(rc)
	{
		printf("Can't open database file: %s.\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return -1;
	}

	int n =WAIT_FOR_IDEL_TIMES;
	do
	{
		// 开始一个事务EXCLUSIVE，保证不会被别的读打断，造成commit错误
		rc = sqlite3_exec(db,"BEGIN EXCLUSIVE ", NULL,NULL, &zErrMsg);

		if( rc != SQLITE_OK )
		{
			sqlite3_free(zErrMsg);
			n--;

			if (n == 0)
			{
				printf("timeout\n");
				sqlite3_close(db);
				return -1;
			}
			if (rc == SQLITE_BUSY)
			{
				printf("write busy\n");
				WAIT_FOR_IDLE; // 睡眠10ms
				sleep(1);
				continue;
			}
		}
		else
		{
			break;
		}

	}while(1);

	//删除所有数据
	sprintf(sql,"delete from pointTable;");
	rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
	if(rc)
	{
		printf("fail to delete values: %s\n", sqlite3_errmsg(db));
		sqlite3_free(zErrMsg);
		sqlite3_close(db); //关闭数据库
		return -1;
	}

	//写入新的点表数据
	for(ptIndex = 0; ptIndex < pt->pointTableNumber; ptIndex++)
	{
		sprintf(sql,"insert into pointTable values(%d,'%s','%s','%s','%s','%s');",
				ptIndex,
				pt->pointTable[ptIndex].address,
				pt->pointTable[ptIndex].datatype,
				pt->pointTable[ptIndex].deviation,
				pt->pointTable[ptIndex].name,
				pt->pointTable[ptIndex].readInterval);
		puts(sql);
		rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
		if(rc)
		{
			printf("fail to insert values: %s\n", sqlite3_errmsg(db));
			sqlite3_free(zErrMsg);
			sqlite3_close(db); //关闭数据库
			return -1;
		}
	}

	rc=sqlite3_exec(db,"COMMIT", NULL,NULL, &zErrMsg);
	if( rc != SQLITE_OK )
	{
		printf("SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		sqlite3_close(db); //关闭数据库
		return -1;
	}

	sqlite3_close(db); //关闭数据库

	return 0;
}

/*
 * 读点表信息
 */
int databaseReadPointTable(pointTable_t *pt)
{
	sqlite3 *db;
	char *zErrMsg = 0;
	char sql[1024] = {0};
	int rc;
	int nrow = 0, ncolumn = 0;
	char **azResult; //二维数组存放结果
	int ptIndex;

	if(pt == NULL)
		return -1;

	rc = sqlite3_open(DB_NAME, &db);
	if(rc)
	{
		printf("Can't open database file: %s.\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return -1;
	}

	int n =WAIT_FOR_IDEL_TIMES;
	do
	{
		// 开始一个事务EXCLUSIVE，保证不会被别的读打断，造成commit错误
		rc = sqlite3_exec(db,"BEGIN IMMEDIATE ", NULL,NULL, &zErrMsg);

		if( rc != SQLITE_OK )
		{
			sqlite3_free(zErrMsg);
			n--;

			if (n == 0)
			{
				printf("timeout\n");
				sqlite3_close(db);
				return -1;
			}
			if (rc == SQLITE_BUSY)
			{
				printf("write busy\n");
				WAIT_FOR_IDLE; // 睡眠10ms
				sleep(1);
				continue;
			}
		}
		else
		{
			break;
		}

	}while(1);

	//读取
	sprintf(sql, "SELECT *FROM pointTable;");
	rc = sqlite3_get_table(db, sql, &azResult, &nrow, &ncolumn, &zErrMsg);
	if (rc != SQLITE_OK)
	{
		printf("zErrMsg = %s \n", zErrMsg);
		sqlite3_free(zErrMsg);
		sqlite3_free_table(azResult);
		sqlite3_close(db);
		return -1;
	}

	if ((nrow == 0) && (ncolumn == 0))
	{
		sqlite3_close(db); //关闭数据库
		return -1;
	}

	pt->pointTableNumber = nrow;
//	printf("pt->pointTableNumber:%d\n", pt->pointTableNumber);
	for(ptIndex = 0; ptIndex < nrow; ptIndex++)
	{
		memcpy(pt->pointTable[ptIndex].address, azResult[(ptIndex+1)*6 + 1], strlen(azResult[(ptIndex+1)*6 + 1]) + 1);
		memcpy(pt->pointTable[ptIndex].datatype, azResult[(ptIndex+1)*6 + 2], strlen(azResult[(ptIndex+1)*6 + 2]) + 1);
		memcpy(pt->pointTable[ptIndex].deviation, azResult[(ptIndex+1)*6 + 3], strlen(azResult[(ptIndex+1)*6 + 3]) + 1);
		memcpy(pt->pointTable[ptIndex].name, azResult[(ptIndex+1)*6 + 4], strlen(azResult[(ptIndex+1)*6 + 4]) + 1);
		memcpy(pt->pointTable[ptIndex].readInterval, azResult[(ptIndex+1)*6 + 5], strlen(azResult[(ptIndex+1)*6 + 5]) + 1);
//		printf("pt->pointTable[%d].address:%s\n", ptIndex, pt->pointTable[ptIndex].address);
//		printf("pt->pointTable[%d].name:%s\n", ptIndex, pt->pointTable[ptIndex].name);
//		printf("pt->pointTable[%d].readInterval:%s\n", ptIndex, pt->pointTable[ptIndex].readInterval);
	}

	sqlite3_free_table(azResult);
	rc=sqlite3_exec(db,"COMMIT", NULL,NULL, &zErrMsg);

	if( rc != SQLITE_OK )
	{
		printf("SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		sqlite3_close(db); //关闭数据库
		return -1;
	}

	sqlite3_close(db); //关闭数据库

	return 0;
}


/*
 * clean点表信息
 */
int databaseCleanPointTable(pointTable_t *pt)
{
	puts("databaseCleanPointTable");

	sqlite3 *db;
	char *zErrMsg = 0;
	char sql[1024] = {0};
	int rc;

	if(pt == NULL)
		return -1;

	rc = sqlite3_open(DB_NAME, &db);
	if(rc)
	{
		printf("Can't open database file: %s.\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return -1;
	}

	int n =WAIT_FOR_IDEL_TIMES;
	do
	{
		// 开始一个事务EXCLUSIVE，保证不会被别的读打断，造成commit错误
		rc = sqlite3_exec(db,"BEGIN EXCLUSIVE ", NULL,NULL, &zErrMsg);

		if( rc != SQLITE_OK )
		{
			sqlite3_free(zErrMsg);
			n--;

			if (n == 0)
			{
				printf("timeout\n");
				sqlite3_close(db);
				return -1;
			}
			if (rc == SQLITE_BUSY)
			{
				printf("write busy\n");
				WAIT_FOR_IDLE; // 睡眠10ms
				sleep(1);
				continue;
			}
		}
		else
		{
			break;
		}

	}while(1);

	//删除所有数据
	sprintf(sql,"delete from pointTable;");
	rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
	if(rc)
	{
		printf("fail to delete values: %s\n", sqlite3_errmsg(db));
		sqlite3_free(zErrMsg);
		sqlite3_close(db); //关闭数据库
		return -1;
	}

	rc=sqlite3_exec(db,"COMMIT", NULL,NULL, &zErrMsg);
	if( rc != SQLITE_OK )
	{
		printf("SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		sqlite3_close(db); //关闭数据库
		return -1;
	}

	sqlite3_close(db); //关闭数据库

	return 0;
}
