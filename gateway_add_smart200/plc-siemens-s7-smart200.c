/*
 * plc-siemens-s7-smart200.c
 *
 *  Created on: 2018年11月28日
 *      Author: fanxiaobin
 */

#include "plc-siemens-s7-smart200.h"
#include "plc-siemens-s7-1500.h"
#include "typedef.h"
#include "msgQueue.h"
#include "plcMain.h"

void SIEMENS_S7_SMART200_Init()
{
	Py_Initialize();

	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.path.append('./')");
	pmodule = PyImport_ImportModule("smart200");
}

void SIEMENS_S7_SMART200_Read_And_Send()
{
	int ret = 0;
	if(plc_connect_flag == disconnected)
	{
		client = create_plc_client();
		//ret = connect_plc(client,"192.168.1.2",0,1);
		ret = connect_plc(client,plcGatewayCfgInfo.plcIp,
				plcGatewayCfgInfo.plcRack,plcGatewayCfgInfo.plcSlot);
		if(ret == 0)
		{
			plc_connect_flag = connected;
			puts("plc connected");
		}
		else
		{
			plc_connect_flag = disconnected;
			destroy_plc_client(client);
			puts("plc disconnected");
			return;
		}
	}

	memset(plcPointValue_buffer,0,sizeof(plcPointValue_buffer));
	for(int i = 0; i < plcPointTable.pointTableNumber; i++)
	{
		if(i!=0)
		{
			strcat(plcPointValue_buffer,",");
		}

		char address[PLC_POINT_ADDRESS_MAX_LEN+1] = {0};
		memcpy(address,plcPointTable.pointTable[i].address,sizeof(address));

		int value = 0;
		ret = read_plc(client,plcPointTable.pointTable[i].address,&value);
		if(ret == 0)
		{
			sprintf(plcPointValue_buffer+strlen(plcPointValue_buffer),"%d",value);
		}
		else
		{
			puts("read_plc return err.");
			break;
		}
	}

	printf("plc msgQ send:%s\n",plcPointValue_buffer);

	if(ret == -1)
	{
		//disconnect_plc(client);
		destroy_plc_client(client);
		plc_connect_flag = disconnected;
		return;
	}

	msgQueueData_t msgData;
	memset(&msgData, 0, sizeof(msgQueueData_t));
	msgData.cmd = CMD_SEND_PLC_POINT_VALUE;
	msgData.len = strlen(plcPointValue_buffer)+1;
	memcpy(msgData.data, plcPointValue_buffer, msgData.len);
	msgQSendToQuarkioe(&msgData);
}

void SIEMENS_S7_SMART200_End()
{
	Py_Finalize();
}

