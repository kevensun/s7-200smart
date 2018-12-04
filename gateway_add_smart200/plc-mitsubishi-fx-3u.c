/*
 * plc-mitsubishi-fx-3u.c
 *
 *  Created on: 2018-9-17
 *      Author: xm
 */

#include "typedef.h"
#include "msgQueue.h"
#include <string.h>
#include <stdio.h>
#include "fx-serial.h"
#include <unistd.h>
#include "plcMain.h"

struct fx_serial* ss = NULL;

int get_bitValue(int data,int bit,int *value)
{
	//printf("data = %x\n",data);
	if(bit>=0 && bit<=7)
	{
		data >>= 8;
		*value = data>>bit & 0x01;
	}
	else
	{
		puts("point table's address err:2-");
		return -1;
	}

	return 0;
}

int get_pointValue(char *address,int *value)
{
//	printf("address = %s\n",address);
	int x = -1;
	int data = 0;
	int ret = -1;

	if(address[0] == 'Y' || address[0] == 'y')
	{
		sscanf(&address[2],"%d",&x);
//		printf("x = %d",x);

		if(address[1] == '0')
		{
			ret = read_y0(ss,&data);
			if(ret == 0)
			{
				ret = get_bitValue(data,x,value);
			}
		}
		else if(address[1] == '1')
		{
			ret = read_y1(ss,&data);
			if(ret == 0)
			{
				ret = get_bitValue(data,x,value);
			}
		}
		else if(address[1] == '2')
		{
			ret = read_y2(ss,&data);
			if(ret == 0)
			{
				ret = get_bitValue(data,x,value);
			}
		}
		else if(address[1] == '3')
		{
			ret = read_y3(ss,&data);
			if(ret == 0)
			{
				ret = get_bitValue(data,x,value);
			}
		}
		else
		{
			puts("point table's address err:1");
			return -1;
		}
	}
	else if(address[0] == 'X' || address[0] == 'x')
	{
		sscanf(&address[2],"%d",&x);
		//printf("x = %d",x);

		if(address[1] == '0')
		{
			ret = read_x0(ss,&data);
			if(ret == 0)
			{
				ret = get_bitValue(data,x,value);
			}
		}
		else if(address[1] == '1')
		{
			ret = read_x1(ss,&data);
			if(ret == 0)
			{
				ret = get_bitValue(data,x,value);
			}
		}
		else if(address[1] == '2')
		{
			ret = read_x2(ss,&data);
			if(ret == 0)
			{
				ret = get_bitValue(data,x,value);
			}
		}
		else if(address[1] == '3')
		{
			ret = read_x3(ss,&data);
			if(ret == 0)
			{
				ret = get_bitValue(data,x,value);
			}
		}
		else
		{
			puts("point table's address err:1");
			return -1;
		}
	}
	else if(address[0] == 'D' || address[0] == 'd')
	{
		sscanf(&address[1],"%d",&x);
		//printf("x = %d",x);

		ret = read_registerD(ss,x,&data);
		if(ret == 0)
		{
			*value = data;
		}
	}
	else
	{
		puts("point table's address err:0");
		return -1;
	}

	return ret;
}

void MITSUBISHI_FX_3U_Read_And_Send(void)
{
	if(ss == NULL)
	{
		//ss = fx_serial_start("/dev/ttyS1", 9600, '7', 'E', '1');//for ubuntu
		ss = fx_serial_start("/dev/ttymxc1", 9600, '7', 'E', '1');//for develop board
	}

	memset(plcPointValue_buffer,0,sizeof(plcPointValue_buffer));
	for(int i=0;i<plcPointTable.pointTableNumber;i++)
	{
		if(i!=0)
		{
			strcat(plcPointValue_buffer,",");
		}

		int value = 0;
		int ret = get_pointValue(plcPointTable.pointTable[i].address,&value);
	//	printf("ret = %d,value = %x\n",ret,value);
		if(ret == 0)
		{
			sprintf(plcPointValue_buffer+strlen(plcPointValue_buffer),"%d",value);
		}
		else
		{
			return;
		}
	}

	msgQueueData_t msgData;
	memset(&msgData, 0, sizeof(msgQueueData_t));
	msgData.cmd = CMD_SEND_PLC_POINT_VALUE;
	msgData.len = strlen(plcPointValue_buffer)+1;
	memcpy(msgData.data, plcPointValue_buffer, msgData.len);
	msgQSendToQuarkioe(&msgData);

	printf("msgQ send:%s\n",plcPointValue_buffer);
}

/*
void MITSUBISHI_FX_3U_Read_And_Send(void)
{
	int i;
	msgQueueData_t msgData;

	for(i = 0; i < plcPointTable.pointTableNumber; i++)
	{
		plcPointValue.value[i] = (i+1)*20;
	}


	 * 发送读到的数据

	memset(&msgData, 0, sizeof(msgQueueData_t));

	msgData.cmd = CMD_SEND_PLC_POINT_VALUE;
	msgData.len = sizeof(pointValue_t);
	memcpy(msgData.data, &plcPointValue, sizeof(pointValue_t));

	msgQSendToQuarkioe(&msgData);
}
*/


