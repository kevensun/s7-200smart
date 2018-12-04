/*
 * createSrTemplate.c
 *
 *  Created on: 2018-9-14
 *      Author: xm
 */

/*
 * create_srTemplate.c
 *
 *  Created on: 2018年9月9日
 *      Author: fanxiaobin
 */

#include <stdio.h>
#include <time.h>
#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>
#include "typedef.h"

extern pointTable_t quarkPointTable;
extern gatewayCfgInfo_t quarkGatewayCfgInfo;

int set_srTemplate_version(const char *deviceID,char *version,int size);
char *get_uploadMeasure_Template();
char *get_uploadMeasure_Template_Part_1();
char *get_uploadMeasure_Template_Part_2();
char *get_uploadMeasure_Template_Part_2_1();

int create_srTemplate(void)
{
	int ret;
	char version[1024] = {0};
	set_srTemplate_version(quarkGatewayCfgInfo.devId, version, sizeof(version));

//	int ret = read_pointTable_from_db();
//	if(ret == -1)
//	{
//		puts("fail to read_pointTable_from_db.");
//		return -1;
//	}

	char *uploadMeasure_Template = get_uploadMeasure_Template();
	if(uploadMeasure_Template == NULL)
	{
		puts("fail to get_uploadMeasure_Template.");
		return -1;
	}

	const char srtemplate_base[] = "srtemplate_base.txt";
	const char srtemplate[] = "srtemplate.txt";

	FILE *fp_r = fopen(srtemplate_base,"r");
	if(fp_r == NULL)
	{
		fprintf(stderr,"%d open to read err.\n",__LINE__);
		perror("fopen");
		free(uploadMeasure_Template);
		return -1;
	}
	else
	{
		FILE *fp_w = fopen(srtemplate,"w");

		fwrite(version,sizeof(char),strlen(version),fp_w);

		char readbuf[1024] = {0};
		do
		{
			ret = fread(readbuf,sizeof(char),sizeof(readbuf),fp_r);
			if(ret > 0)
			{
				fwrite(readbuf,sizeof(char),ret,fp_w);
			}
		}while(ret>0);

		fwrite(uploadMeasure_Template,sizeof(char),strlen(uploadMeasure_Template),fp_w);

		fclose(fp_w);
	}

	fclose(fp_r);
	free(uploadMeasure_Template);

	return 0;
}

int set_srTemplate_version(const char *deviceID,char *version,int size)
{
	time_t rawtime;
	time(&rawtime);
	struct tm *tm_time = localtime(&rawtime);

	char buf[1024] = {0};
	sprintf(buf, "wsn");
	strcat(buf,deviceID);
	sprintf(buf+strlen(buf),"%04d%02d%02d%02d%02d%02d\n\n",tm_time->tm_year+1900,tm_time->tm_mon+1,tm_time->tm_mday,
			tm_time->tm_hour,tm_time->tm_min,tm_time->tm_sec);
//	puts(buf);

	int len = strlen(buf);
	if(len+1 > size)
	{
		fprintf(stderr,"%d size is small.\n",__LINE__);
		return -1;
	}
	else
	{
		memcpy(version,buf,strlen(buf)+1);
	}

	return 0;
}

//10,200,POST,/measurement/measurements,application/json,,%%,#Variable,#Variable
char *get_uploadMeasure_Template()
{
	char *param_1 = get_uploadMeasure_Template_Part_1();
	char *param_2 = get_uploadMeasure_Template_Part_2();

	if(param_1 == NULL || param_2 == NULL)
	{
		if(param_1 != NULL)
		{
			free(param_1);
		}
		else if(param_2 != NULL)
		{
			free(param_2);
		}
		return NULL;
	}

	char str[] = "10,200,POST,/measurement/measurements,application/json,,%%,,";
	int num = strlen(str) + strlen(param_1) + strlen(param_2) + 1;
	char *p = (char *)malloc(num * sizeof(char));
	if(p == NULL)
	{
		fprintf(stderr,"%d malloc fail.\n",__LINE__);
		free(param_1);
		free(param_2);
		return NULL;
	}
	else
	{
//		printf("%d malloc success : %d\n",__LINE__,num);
		memset(p,0,num);
	}

	sprintf(p,"10,200,POST,/measurement/measurements,application/json,,%%%%,%s,%s",
			param_1,param_2);

	free(param_1);
	free(param_2);

//	printf("%d len = %lu\n",__LINE__,strlen(p));

	return p;
}

//NOW UNSIGNED NUMBER NUMBER NUMBER NUMBER NUMBER
char *get_uploadMeasure_Template_Part_1()
{
	int i;
	if(quarkPointTable.pointTableNumber < 1)
	{
		fprintf(stderr,"%d pointTable pointTableNumber < 1.\n",__LINE__);
		return NULL;
	}

	int num = strlen("NOW ") + strlen("UNSIGNED ") + strlen("NUMBER ")*quarkPointTable.pointTableNumber;
	char *p = (char *)malloc(num * sizeof(char));
	if(p == NULL)
	{
		fprintf(stderr,"%d malloc fail.\n",__LINE__);
		return NULL;
	}
	else
	{
	//	printf("%d malloc success : %d\n",__LINE__,num);
		memset(p,0,num);
	}

	strcat(p,"NOW UNSIGNED");

	for(i=0;i<quarkPointTable.pointTableNumber;i++)
	{
		strcat(p," NUMBER");
	}

//	printf("%d %lu\n",__LINE__,strlen(p));

	return p;
}

//"{""time"":""%%"",""source"":{""id"":""%%""},""type"":""OBJ"",""OBJ"":{#Variable...}}"
char *get_uploadMeasure_Template_Part_2()
{
	char *param = get_uploadMeasure_Template_Part_2_1();
	if(param == NULL)
	{
		return NULL;
	}

	//"{""time"":""%%"",""source"":{""id"":""%%""},""type"":""OBJ"",""OBJ"":}"
	char str[] = "\"{\"\"time\"\":\"\"%%\"\",\"\"source\"\":{\"\"id\"\":\"\"%%\"\"},\"\"type\"\":\"\"OBJ\"\",\"\"OBJ\"\":}\"";

	int num = strlen(str) + strlen(param) + 1;
	char *p = (char *)malloc(num * sizeof(char));
	if(p == NULL)
	{
		fprintf(stderr,"%d malloc fail.\n",__LINE__);
		free(param);
		return NULL;
	}
	else
	{
//		printf("%d malloc success : %d\n",__LINE__,num);
		memset(p,0,num);
	}

	//"{""time"":""%%"",""source"":{""id"":""%%""},""type"":""OBJ"",""OBJ"":%s}"
	sprintf(p,"\"{\"\"time\"\":\"\"%%%%\"\",\"\"source\"\":{\"\"id\"\":\"\"%%%%\"\"},\"\"type\"\":\"\"OBJ\"\",\"\"OBJ\"\":%s}\"",
			param);

	free(param);

//	printf("%d %lu\n",__LINE__,strlen(p));

	return p;
}

//{""Attr1"":{""value"":%%,""unit"":null},""Attr2"":{""value"":%%,""unit"":null}...}
//""%s"":{""value"":%%,""unit"":null}
char *get_uploadMeasure_Template_Part_2_1()
{
	int i;
	if(quarkPointTable.pointTableNumber < 1)
	{
		fprintf(stderr,"%d pointTable pointTableNumber < 1.\n",__LINE__);
		return NULL;
	}

	//"""":{""value"":%%,""unit"":null}
	char str[] = "\"\"\"\":{\"\"value\"\":%%,\"\"unit\"\":null}";

	int num = 2 + (sizeof(quarkPointTable.pointTable[0].name) + strlen(str) + 1)*quarkPointTable.pointTableNumber;

	char *p = (char *)malloc(num * sizeof(char));
	if(p == NULL)
	{
		fprintf(stderr,"%d malloc fail.\n",__LINE__);
		return NULL;
	}
	else
	{
//		printf("%d malloc success : %d\n",__LINE__,num);
		memset(p,0,num);
	}

	strcat(p,"{");

	for(i=0;i<quarkPointTable.pointTableNumber;i++)
	{
		if(i>0)
		{
			strcat(p,",");
		}

		strcat(p,"\"\"");
//		puts(quarkPointTable.pointTable[i].name);
		strcat(p,quarkPointTable.pointTable[i].name);
		strcat(p,"\"\"");
		strcat(p,":{\"\"value\"\":%%,\"\"unit\"\":null}");
//		puts(p);
	}

	strcat(p,"}");

//	printf("%d %lu\n",__LINE__,strlen(p));

	return p;
}


