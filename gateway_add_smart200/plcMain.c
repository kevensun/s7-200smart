/*
 * plcMain.c
 *
 *  Created on: 2018-9-14
 *      Author: xm
 */

#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include "msgQueue.h"
#include "database.h"
#include "typedef.h"
#include "plc-siemens-s7-200.h"
#include "plc-mitsubishi-fx-3u.h"
#include "plc-siemens-s7-1500.h"
#include "plc-siemens-s7-smart200.h"

sysStatus_t plcSysStatus;
pointTable_t plcPointTable;
gatewayCfgInfo_t plcGatewayCfgInfo;
pointValue_t plcPointValue;
char readPlcPointDataIsGoing = 0;

//char *plcPointValue_buffer = NULL;
char plcPointValue_buffer[1024] = {0};

/*
 * 初始化系统所需环境
 */
void plcInit(void)
{
	switch(plcGatewayCfgInfo.plcType)
	{
		case PLC_SIEMENS_S7_200:
			SIEMENS_S7_200_Init();
			break;
		case PLC_MITSUBISHI_FX_3U:
			break;
		case PLC_SIEMENS_S7_1500:
			SIEMENS_S7_1500_Init();
			break;
		case PLC_SIEMENS_S7_SMART200:
			SIEMENS_S7_SMART200_Init();
			break;
		default:
			break;
	}
}

/*
 * 读plc点表对应的值
 */
void plcPointTableRead(void)
{
	switch(plcGatewayCfgInfo.plcType)
	{
		case PLC_SIEMENS_S7_200:
			SIEMENS_S7_200_Read_And_Send();
			break;
		case PLC_MITSUBISHI_FX_3U:
			MITSUBISHI_FX_3U_Read_And_Send();
			break;
		case PLC_SIEMENS_S7_1500:
			SIEMENS_S7_1500_Read_And_Send();
			break;
		case PLC_SIEMENS_S7_SMART200:
			SIEMENS_S7_SMART200_Read_And_Send();
			break;
		default:
			break;
	}
}

/*
 * 清理系统使用过的环境
 */
void plcEnd(void)
{
	switch(plcGatewayCfgInfo.plcType)
	{
		case PLC_SIEMENS_S7_200:
			SIEMENS_S7_200_End();
			break;
		case PLC_MITSUBISHI_FX_3U:
			break;
		case PLC_SIEMENS_S7_1500:
			SIEMENS_S7_1500_End();
			break;
		case PLC_SIEMENS_S7_SMART200:
			SIEMENS_S7_SMART200_End();
			break;
		default:
			break;
	}
}

/*
 * 定时器中断
 */
void plcTimerHandler(int sigNo)
{
	msgQueueData_t msgQueueData;

	msgQueueData.cmd = CMD_START_PLC_POINT_READ_ONCE;
	msgQueueData.len = 0;
	memset(msgQueueData.data, 0, sizeof(msgQueueData.data));

	switch(sigNo)
	{
		case SIGALRM:
			if(readPlcPointDataIsGoing == 0)
			{
				msgQSendToPlc(&msgQueueData);
			}
			break;
		default:
			break;
	}
}

/*
 * 定时器初始化
 */
void plcTimerInit(void)
{
	signal(SIGALRM, plcTimerHandler);
	struct itimerval timerVal;

	/*
	 * 定时1秒
	 */
	timerVal.it_value.tv_sec = 1;		//1秒后触发信号，之后根据定时周期来触发信号
	timerVal.it_value.tv_usec = 0;

	timerVal.it_interval.tv_sec = 1;	//定时周期1秒
	timerVal.it_interval.tv_usec = 0;

	setitimer(ITIMER_REAL, &timerVal, NULL);
}

/*
 * plc进程主循环
 */
void plcMain(void)
{
	/*
	 * 清零数据
	 */
	memset(&plcSysStatus, 0, sizeof(sysStatus_t));
	memset(&plcPointTable, 0, sizeof(pointTable_t));
	memset(&plcGatewayCfgInfo, 0, sizeof(gatewayCfgInfo_t));

	/*
	 * 尽可能避免同时读数据库
	 */
	sleep(1);


    /*
     * 读取数据库，检查server地址是否已经配置，未配置则等待配置成功
     */
    if(databaseReadGatewayCfg(&plcGatewayCfgInfo) == -1)
    {
    	plcSysStatus.isSrvAddressAvailable = NOT_AVAILABLE;
    }
    else
    {
    	plcGatewayCfgInfo.srvAddress[GATEWAY_SERVER_ADDRESS_INFO_MAX_LEN -1] = '\0';
    	if(strlen(plcGatewayCfgInfo.srvAddress) < 1)
    	{
    		plcSysStatus.isSrvAddressAvailable = NOT_AVAILABLE;
    	}
    	else
    	{
    		plcSysStatus.isSrvAddressAvailable = AVAILABLE;
    	}
    }

    while(1)
    {
        if(plcSysStatus.isSrvAddressAvailable == AVAILABLE)
        	break;

        sleep(5);

        if(databaseReadGatewayCfg(&plcGatewayCfgInfo) == -1)
        {
        	plcSysStatus.isSrvAddressAvailable = NOT_AVAILABLE;
        	printf("plcSysStatus.isSrvAddressAvailable = NOT_AVAILABLE\n");
        }
        else
        {
        	plcGatewayCfgInfo.srvAddress[GATEWAY_SERVER_ADDRESS_INFO_MAX_LEN -1] = '\0';
        	if(strlen(plcGatewayCfgInfo.srvAddress) < 1)
        	{
        		plcSysStatus.isSrvAddressAvailable = NOT_AVAILABLE;
            	printf("plcSysStatus.isSrvAddressAvailable = NOT_AVAILABLE\n");
        	}
        	else
        	{
        		plcSysStatus.isSrvAddressAvailable = AVAILABLE;
            	printf("plcSysStatus.isSrvAddressAvailable = AVAILABLE\n");
        	}
        }
    }

	/*
	 * 判断点表信息是否可用
	 */
	if(databaseReadPointTable(&plcPointTable) == -1)
		plcSysStatus.isPointTableAvailable = NOT_AVAILABLE;
	else
		plcSysStatus.isPointTableAvailable = AVAILABLE;

	/*
	 * 等待点表信息可用
	 */
	while(1)
	{
		if(plcSysStatus.isPointTableAvailable == AVAILABLE)
		{
			databaseReadGatewayCfg(&plcGatewayCfgInfo);
			break;
		}

		sleep(5);

		if(databaseReadPointTable(&plcPointTable) == -1)
		{
			printf("plcSysStatus.isPointTableAvailable = NOT_AVAILABLE\n");
			plcSysStatus.isPointTableAvailable = NOT_AVAILABLE;
		}
		else
		{
			printf("plcSysStatus.isPointTableAvailable = AVAILABLE;\n");
			plcSysStatus.isPointTableAvailable = AVAILABLE;
		}
	}

	/*
	 * plc所需环境初始化
	 */
	plcInit();

	/*
	 * 启动定时器
	 */
	plcTimerInit();

	/*
	 * 主循环
	 */
	while(1)
	{
		msgQueueData_t msgQueueData;

		memset((void *)&msgQueueData, 0, sizeof(msgQueueData_t));
		msgQReceiveForPlc(&msgQueueData);
		switch(msgQueueData.cmd)
		{
			case CMD_START_PLC_POINT_READ_ONCE:
				readPlcPointDataIsGoing = 1;
				plcPointTableRead();
				readPlcPointDataIsGoing = 0;
				break;
			default:
				break;
		}
	}

	/*
	 * 清理系统使用过的环境
	 */
	plcEnd();
}
