/*
 * msgQueue.c
 *
 *  Created on: 2018-9-14
 *      Author: xm
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "typedef.h"

int msgQIdQuarkioe, msgQIdPlc, msgQIdRWeb;

/*
 * 消息队列初始化
 */
int msgQInit(void)
{
	key_t keyQuarkioe, keyPlc, keyRWeb;

	/*
	 * quark ioe
	 */
	keyQuarkioe = ftok("gatewayMsgQueueFile", 1);
	if(keyQuarkioe == -1)
	{
		printf("keyQuarkioe ftok error.\n");
		return -1;
	}
	else
	{
		//printf("keyQuarkioe = %x\n",keyQuarkioe);
	}
	msgQIdQuarkioe = msgget(keyQuarkioe, IPC_CREAT|0600);
	if(msgQIdQuarkioe == -1)
	{
		printf("msgQIdQuarkioe msgget error.\n");
		return -1;
	}
	else
	{
		//printf("msgQIdQuarkioe = %d\n",msgQIdQuarkioe);
	}

	//getchar();

	/*
	 * plc
	 */
	keyPlc = ftok("gatewayMsgQueueFile", 2);
	if(keyPlc == -1)
	{
		printf("keyPlc ftok error.\n");
		return -1;
	}
	msgQIdPlc = msgget(keyPlc, IPC_CREAT|0600);
	if(msgQIdPlc == -1)
	{
		printf("msgQIdPlc msgget error.\n");
		return -1;
	}

	/*
	 * web
	 */
	keyRWeb = ftok("gatewayMsgQueueFile", 3);
	if(keyRWeb == -1)
	{
		printf("keyRWeb ftok error.\n");
		return -1;
	}
	msgQIdRWeb = msgget(keyRWeb, IPC_CREAT|0600);
	if(msgQIdRWeb == -1)
	{
		printf("msgQIdRWeb msgget error.\n");
		return -1;
	}

	return 0;
}


/*
 * 消息队列发送封装
 */
void msgQSend(int msgQId, msgQueueData_t *msgData)
{
	char retryTime;
	msgQueue_t msg;

	msg.type = 1;
	memcpy((void *)&msg.data, (void *)msgData, sizeof(msgQueueData_t));

	for(retryTime = 0; retryTime < 3; retryTime++)
	{
		if(msgsnd(msgQId, (void *)&msg, sizeof(msg.data), IPC_NOWAIT) == 0)
		{
			break;
		}
		else
		{
			printf("msgsnd fail with msg cmd %d, retry %d time.\n", msg.data.cmd, retryTime+1);
		}
	}
}

/*
 * quark
 */
void msgQSendToQuarkioe(msgQueueData_t *msgData)
{
	msgQSend(msgQIdQuarkioe, msgData);
}

/*
 * plc
 */
void msgQSendToPlc(msgQueueData_t *msgData)
{
	msgQSend(msgQIdPlc, msgData);
}

/*
 * web
 */
void msgQSendToRWeb(msgQueueData_t *msgData)
{
	msgQSend(msgQIdRWeb, msgData);
}

/*
 * 消息队列接收封装
 */
void msgQReceive(int msgQId, msgQueueData_t *msgData)
{
	ssize_t msgRcvSize;
	msgQueue_t msg;

	memset((void *)&msg, 0, sizeof(msgQueue_t));
	msgRcvSize = msgrcv(msgQId, (void *)&msg, sizeof(msg.data), 0, 0);
	if(msgRcvSize == -1)
	{
		memset(msgData, 0, sizeof(msgQueueData_t));
	}
	else
	{
		memcpy(msgData, (void *)&msg.data, sizeof(msgQueueData_t));
	}
}

/*
 * quark
 */
void msgQReceiveForQuarkioe(msgQueueData_t *msgData)
{
	msgQReceive(msgQIdQuarkioe, msgData);
}

/*
 * plc
 */
void msgQReceiveForPlc(msgQueueData_t *msgData)
{
	msgQReceive(msgQIdPlc, msgData);
}

/*
 * web
 */
void msgQReceiveForRWeb(msgQueueData_t *msgData)
{
	msgQReceive(msgQIdRWeb, msgData);
}

