/*
 * typedef.h
 *
 *  Created on: 2018-9-14
 *      Author: xm
 */

#ifndef TYPEDEF_H_
#define TYPEDEF_H_

#ifdef __cplusplus    //__cplusplus是cpp中自定义的一个宏
extern "C" {          //告诉编译器，这部分代码按C语言的格式进行编译，而不是C++的
#endif

//#define PC

#define PORT 6789
#define IP "120.76.210.84"
//#define IP "192.168.1.118"
#define GATEWAY_SOFTWARE_VERSION "1.0.4"//1.0.3
/*
 * 类型定义
 */
typedef char int8;
typedef short int16;
typedef int int32;
typedef long int64;

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long uint64;

/*
 * PLC型号
 */
#define PLC_SIEMENS_S7_200		0
#define PLC_MITSUBISHI_FX_3U	1
#define PLC_SIEMENS_S7_1500		2
#define PLC_SIEMENS_S7_SMART200 3

/*
 * 可用不可用
 */
#define NOT_AVAILABLE					0
#define AVAILABLE						1

/*
 * 消息队列结构体
 */
#define CMD_START_PLC_POINT_READ_ONCE	11
#define CMD_SEND_PLC_POINT_VALUE		21

#define GATEWAY_MSG_Q_DATA_MAX_LEN (1024-4)

typedef struct
{
	short cmd;
	short len;
	char data[GATEWAY_MSG_Q_DATA_MAX_LEN];
}msgQueueData_t;

//typedef struct
//{
//	short cmd;
//	short len;
//	char data[0];
//}msgQueueData_t;

typedef struct
{
	long type;
	msgQueueData_t data;
}msgQueue_t;

/*
 * 结构体类型定义
 */
#define PLC_POINT_ADDRESS_MAX_LEN (16)
#define PLC_POINT_NAME_MAX_LEN (64)
#define PLC_POINT_INTERVAL_MAX_LEN (8)
#define PLC_POINT_MAX_NUMBER (100)
#define PLC_POINT_DATATYPE_MAX_LEN (10)
#define PLC_POINT_DEVIATION_MAX_LEN (20)

#define GATEWAY_DEV_ID_MAX_LEN (32)
#define GATEWAY_MAC_INFO_MAX_LEN (32)
#define GATEWAY_IP_INFO_MAX_LEN (16)
#define GATEWAY_VER_INFO_MAX_LEN (16)
#define GATEWAY_SERVER_ADDRESS_INFO_MAX_LEN (256)

typedef struct
{
	char address[PLC_POINT_ADDRESS_MAX_LEN+1];
	char name[PLC_POINT_NAME_MAX_LEN+1];
	char readInterval[PLC_POINT_INTERVAL_MAX_LEN+1];
	int value;
	char datatype[PLC_POINT_DATATYPE_MAX_LEN+1];
	char deviation[PLC_POINT_DEVIATION_MAX_LEN+1];
}point_t;

typedef struct
{
	char pointTableNumber;
	point_t pointTable[PLC_POINT_MAX_NUMBER];
}pointTable_t;

typedef struct
{
	int value[PLC_POINT_MAX_NUMBER];
}pointValue_t;

typedef struct
{
	char netType;
	char plcType;
	char devId[GATEWAY_DEV_ID_MAX_LEN+1];
	char netIp[GATEWAY_IP_INFO_MAX_LEN+1];
	char netMask[GATEWAY_IP_INFO_MAX_LEN+1];
	char netGateway[GATEWAY_IP_INFO_MAX_LEN+1];
	char srvAddress[GATEWAY_SERVER_ADDRESS_INFO_MAX_LEN+1];
	char plcIp[GATEWAY_IP_INFO_MAX_LEN+1];
	int plcRack;
	int plcSlot;
}gatewayCfgInfo_t;

typedef struct
{
	char isPointTableAvailable;
	char isSrvAddressAvailable;
}sysStatus_t;

#ifdef __cplusplus
}
#endif

#endif /* TYPEDEF_H_ */
