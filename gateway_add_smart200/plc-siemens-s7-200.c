/*
 * plc-siemens-s7-200.c
 *
 *  Created on: 2018-9-17
 *      Author: xm
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "typedef.h"
#include "msgQueue.h"
#include "plc-siemens-s7-200-driver.h"

#define WAITTIME 100000

#define PPI_UNKNOWN_REG		0
#define PPI_INPUT_REG		1
#define PPI_OUTPUT_REG		2
#define PPI_VD_REG			3
#define PPI_VW_REG			4
#define PPI_VB_REG			5

#define PPI_IO_REG_OK		0x55

char confirm[] = {0x10,0x02,0x00,0x5C,0x5E,0x16};
char hello[] = {0x10,0x02,0x00,0x49,0x4B,0x16};
char readi[] = {0x68,0x1B,0x1B,0x68,0x02,0x00,0x6C,0x32,0x01,0x00,
		0x00,0x00,0x00,0x00,0x0E,0x00,0x00,0x04,0x01,0x12,
		0x0A,0x10,0x02,0x00,0x03,0x00,0x00,0x81,0x00,0x00,
		0x00,0x66,0x16};
char readq[] = {0x68,0x1B,0x1B,0x68,0x02,0x00,0x6C,0x32,0x01,0x00,
		0x00,0x00,0x00,0x00,0x0E,0x00,0x00,0x04,0x01,0x12,
		0x0A,0x10,0x02,0x00,0x02,0x00,0x00,0x82,0x00,0x00,
		0x00,0x66,0x16};
char i0[8]={0};
char i1[8]={0};
char i2[8]={0};
char q0[8]={0};
char q1[8]={0};
int nread;
char buff[512];

extern pointTable_t plcPointTable;
extern pointValue_t plcPointValue;
extern char plcPointValue_buffer[1024];
int fd = -1;

void identify_and_read(void);

/*
 *初始化
 */
void SIEMENS_S7_200_Init(void)
{
	char *file = "/dev/ttymxc2";
	struct termios old;
	struct termios termiosnew;

	fd = open(file, O_RDWR|O_NOCTTY);
	if(fd<0)
		printf("open error\n");

	//ioctl(fd, TIOCSERMODE, U_MODE_FULL_232);

	get_termios(fd, &old);
	//show_termios(&old);
	uart_config(fd, 9600, 'e', 8, 1);
	get_termios(fd, &termiosnew);
	//show_termios(&new);
	tcflush(fd,TCIFLUSH);
	fcntl(fd,F_SETFL,0);
}

/*
 * 根据点表读取对应点表的值并发送
 */
void SIEMENS_S7_200_Read_And_Send(void)
{


	identify_and_read();

	/*
	 * 发送读到的数据
	 */
//	msgQueueData_t msgData;
//	memset(&msgData, 0, sizeof(msgQueueData_t));
//	msgData.cmd = CMD_SEND_PLC_POINT_VALUE;
//	msgData.len = sizeof(pointValue_t);
//	memcpy(msgData.data, &plcPointValue, sizeof(pointValue_t));
//	msgQSendToQuarkioe(&msgData);

	msgQueueData_t msgData;
	memset(&msgData, 0, sizeof(msgQueueData_t));
	msgData.cmd = CMD_SEND_PLC_POINT_VALUE;
	msgData.len = strlen(plcPointValue_buffer)+1;
	memcpy(msgData.data, plcPointValue_buffer, msgData.len);
	msgQSendToQuarkioe(&msgData);

	printf("msgQ send:%s\n",plcPointValue_buffer);
}

/*
 * 清理系统使用过的环境
 */
void SIEMENS_S7_200_End(void)
{
	close(fd);
}
/*
 * 读取输入口
 */
int read_input(int fd)
{
	//下面开始读取输入I0.0-I2.7
	int i;
	write(fd,readi,33);
	usleep(WAITTIME);
	nread=read(fd, buff, 512);
	if(nread>0)
	{
		if(buff[0]==0xE5)
		{
			//printf("receive E5\n");
			write(fd,confirm,6);
			usleep(WAITTIME);
		}
	}
	else
	{
		printf("not receive E5\n");
		return -5;
	}
	nread=read(fd, buff, 512);
	//printf("\nlen:%d\n", nread);
	if(nread>0)
	{
//		for(i=0;i<nread;i++)
//			printf("%02x ",buff[i]);
//		printf("\n");
//		printf("I is :%02x %02x %02x\n",buff[25],buff[26],buff[27]);
		for(i=0;i<8;i++)
		{
			i0[i]=(buff[25]>>i)&1;
//			printf("I0.%d=%d\n",i,i0[i]);
		}

		for(i=0;i<8;i++)
		{
			i1[i]=(buff[26]>>i)&1;
//			printf("I1.%d=%d\n",i,i1[i]);
		}

		for(i=0;i<8;i++)
		{
			i2[i]=(buff[27]>>i)&1;
//			printf("I2.%d=%d\n",i,i2[i]);
		}
	}
	write(fd,hello,6);
	usleep(WAITTIME);
	nread=read(fd, buff, 512);
	return 0;
}

/*
 * 读取输出口
 */
int  read_output(int fd)
{
	//开始读取输出Q0.0-Q1.7
	int i;
	write(fd,readq,33);
	usleep(WAITTIME);
	nread=read(fd, buff, 512);
	if(nread>0)
	{
		if(buff[0]==0xE5)
		{
			//printf("receive E5\n");
			write(fd,confirm,6);
			usleep(WAITTIME);
		}
	}
	else
	{
		printf("not receive E5\n");
		return -5;
	}
	nread=read(fd, buff, 512);
	//printf("\nlen:%d\n", nread);
	if(nread>0)
	{
//		for(i=0;i<nread;i++)
//			printf("%02x ",buff[i]);
//		printf("\n");
//		printf("Q is :%02x %02x\n",buff[25],buff[26]);
		for(i=0;i<8;i++)
		{
			q0[i]=(buff[25]>>i)&1;
//			printf("Q0.%d=%d\n",i,q0[i]);
		}

		for(i=0;i<8;i++)
		{
			q1[i]=(buff[26]>>i)&1;
//			printf("Q1.%d=%d\n",i,q1[i]);
		}
	}
	write(fd,hello,6);
	usleep(WAITTIME);
	nread=read(fd, buff, 512);
	return 0;
}

/*
 * 读取vd区
 */
int readvd(int fd,char vdx[])
{
	char buf[512];
	int nread=0,x=0;
	//int i=0;
	write(fd,vdx,33);
	usleep(WAITTIME);
	nread=read(fd, buf, 512);
	if(nread>0)
	{
		if(buf[0]==0xE5)
		{
			//printf("receive E5\n");
			write(fd,confirm,6);
			usleep(WAITTIME);
		}
	}
	else
	{
		printf("not receive E5\n");
		return -5;
	}
	nread=read(fd, buf, 512);
	//printf("\nlen:%d\n", nread);
	if(nread>0)
	{
//		for(i=0;i<nread;i++)
//			printf("%02x ",buf[i]);
//		printf("\n");
		x=(buf[25]<<24)+(buf[26]<<16)+(buf[27]<<8)+buf[28];
		//printf("result is 0x%x,",x);
	}
	else
	{
		return -1;
	}
	write(fd,hello,6);
	usleep(WAITTIME);
	nread=read(fd, buf, 512);
	return x;
}

/*
 * 读取vw区
 */
int readvw(int fd,char vwx[])
{
	char buf[512];
	int nread=0,x=0;
	//int i=0;
	write(fd,vwx,33);
	usleep(WAITTIME);
	nread=read(fd, buf, 512);
	if(nread>0)
	{
		if(buf[0]==0xE5)
		{
			//printf("receive E5\n");
			write(fd,confirm,6);
			usleep(WAITTIME);
		}
	}
	else
	{
		printf("not receive E5\n");
		return -5;
	}
	nread=read(fd, buf, 512);
	//printf("\nlen:%d\n", nread);
	if(nread>0)
	{
//		for(i=0;i<nread;i++)
//			printf("%02x ",buf[i]);
//		printf("\n");
		x=(buf[25]<<8)+buf[26];
		//printf("result is 0x%x ,",x);
	}
	else
	{
		return -1;
	}
	write(fd,hello,6);
	usleep(WAITTIME);
	nread=read(fd, buf, 512);
	return x;
}

/*
 * 读取vb区
 */
int readvb(int fd,char vbx[])
{
	char buf[512];
	int nread=0,x=0;
	//int i=0;
	write(fd,vbx,33);
	usleep(WAITTIME);
	nread=read(fd, buf, 512);
	if(nread>0)
	{
		if(buf[0]==0xE5)
		{
			//printf("receive E5\n");
			write(fd,confirm,6);
			usleep(WAITTIME);
		}
	}
	else
	{
		printf("not receive E5\n");
		return -5;
	}
	nread=read(fd, buf, 512);
	//printf("\nlen:%d\n", nread);
	if(nread>0)
	{
//		for(i=0;i<nread;i++)
//			printf("%02x ",buf[i]);
//		printf("\n");
		x=buf[25];
		//printf("result is 0x%x ,",x);
	}
	else
	{
		return -1;
	}
	write(fd,hello,6);
	usleep(WAITTIME);
	nread=read(fd, buf, 512);
	return x;
}

char PPI_CheckSum(char  *buf, int len)
{
	char	CheckSum_Value = 0;
	int		i;

	for(i=0;i<len;i++)
	{
		CheckSum_Value += buf[i];
	}

	return CheckSum_Value;
}

int getOffset(char *address)
{
	int i, offset = 0;

	for(i = 0; i < strlen(address); i++)
	{
		if(address[i] >= '0' && address[i] <= '9')
		{
			offset = offset*10 + (address[i] - '0');
		}
		else
		{
			break;
		}
	}
	offset *= 8;

	return offset;
}

void getReadCommandFrame(int *regType, char *add, char *cmd)
{
	int i, offset;
	char address[PLC_POINT_ADDRESS_MAX_LEN] = {0};

	/*
	 * cmd buffer
	 */
	cmd[0] = 0x68;
	cmd[1] = 0x1B;
	cmd[2] = 0x1B;
	cmd[3] = 0x68;
	cmd[4] = 0x02;
	cmd[5] = 0x00;
	cmd[6] = 0x6C;
	cmd[7] = 0x32;
	cmd[8] = 0x01;
	cmd[9] = 0x00;
	cmd[10] = 0x00;
	cmd[11] = 0x00;
	cmd[12] = 0x00;
	cmd[13] = 0x00;
	cmd[14] = 0x0E;
	cmd[15] = 0x00;
	cmd[16] = 0x00;
	cmd[17] = 0x04;
	cmd[18] = 0x01;
	cmd[19] = 0x12;
	cmd[20] = 0x0A;
	cmd[21] = 0x10;

	//从数据库拿出原始字符串形式地址
	memcpy(address, add, PLC_POINT_ADDRESS_MAX_LEN);

	//强制截断
	address[PLC_POINT_ADDRESS_MAX_LEN-1] = '\0';

	//全部转小写
	for(i = 0; i < strlen(address); i++)
	{
		if(address[i] >= 'A' && address[i] <= 'Z')
		{
			address[i] += 32;
		}
	}

	//地址鉴别
	if(strncmp(address, "i", 1) == 0)
	{
		*regType = PPI_INPUT_REG;
		if(address[1] >= '0' && address[1] <= '2')
		{
			if(address[2] == '.')
			{
				if(address[3] >= '0' && address[3] <= '7')
				{
					cmd[0] = PPI_IO_REG_OK;
					cmd[1] = address[1] - '0';
					cmd[2] = address[3] - '0';
					return;
				}
			}
		}

		cmd[0] = 0;
		return;
	}
	else if(strncmp(address, "q", 1) == 0)
	{
		*regType = PPI_OUTPUT_REG;
		if(address[1] >= '0' && address[1] <= '1')
		{
			if(address[2] == '.')
			{
				if(address[3] >= '0' && address[3] <= '7')
				{
					cmd[0] = PPI_IO_REG_OK;
					cmd[1] = address[1] - '0';
					cmd[2] = address[3] - '0';
					return;
				}
			}
		}

		cmd[0] = 0;
		return;
	}
	else if(strncmp(address, "vd", 2) == 0)
	{
		*regType = PPI_VD_REG;
		cmd[22] = 0x06;
		cmd[23] = 0x00;
		cmd[24] = 0x01;
		cmd[25] = 0x00;
		cmd[26] = 0x01;
		cmd[27] = 0x84;
	}
	else if(strncmp(address, "vw", 2) == 0)
	{
		*regType = PPI_VW_REG;
		cmd[22] = 0x04;
		cmd[23] = 0x00;
		cmd[24] = 0x01;
		cmd[25] = 0x00;
		cmd[26] = 0x01;
		cmd[27] = 0x84;
	}
	else if(strncmp(address, "vb", 2) == 0)
	{
		*regType = PPI_VB_REG;
		cmd[22] = 0x02;
		cmd[23] = 0x00;
		cmd[24] = 0x01;
		cmd[25] = 0x00;
		cmd[26] = 0x01;
		cmd[27] = 0x84;
	}
	else
	{
		*regType = PPI_UNKNOWN_REG;
		return;
	}

	switch(*regType)
	{
		case PPI_VD_REG:
		case PPI_VW_REG:
		case PPI_VB_REG:
			offset = getOffset(address+2);
			cmd[28] = offset >> 16;
			cmd[29] = offset >> 8;
			cmd[30] = offset;
			//FCS,校验码
			cmd[31] = PPI_CheckSum(cmd + 4, 27);
			//DE,结束符
			cmd[32] = 0x16;
			break;
		default:
			break;
	}
}

void identify_and_read(void)
{
	int i, regType;
	char ppi_cmd_buf[33] = {0};

	memset(&plcPointValue, 0, sizeof(pointValue_t));
	memset(plcPointValue_buffer,0,sizeof(plcPointValue_buffer));

	//读取输入输出，通用
	read_input(fd);
	read_output(fd);

	for(i = 0; i < plcPointTable.pointTableNumber; i++)
	{
		if(i!=0)
		{
			strcat(plcPointValue_buffer,",");
		}

		getReadCommandFrame(&regType, plcPointTable.pointTable[i].address, ppi_cmd_buf);
		switch(regType)
		{
		case PPI_INPUT_REG:
			if(ppi_cmd_buf[0] == PPI_IO_REG_OK)
			{
				switch(ppi_cmd_buf[1])
				{
					case 0:
						plcPointValue.value[i] = (i0[(int)ppi_cmd_buf[2]]);
						break;
					case 1:
						plcPointValue.value[i] = i1[(int)ppi_cmd_buf[2]];
						break;
					case 2:
						plcPointValue.value[i] = i2[(int)ppi_cmd_buf[2]];
						break;
					default:
						break;
				}
			}
			else
			{
				plcPointValue.value[i] = 0;
			}
			break;
		case PPI_OUTPUT_REG:
			if(ppi_cmd_buf[0] == PPI_IO_REG_OK)
			{
				switch(ppi_cmd_buf[1])
				{
					case 0:
						plcPointValue.value[i] = (int)q0[(int)ppi_cmd_buf[2]];
						break;
					case 1:
						plcPointValue.value[i] = (int)q1[(int)ppi_cmd_buf[2]];
						break;
					default:
						break;
				}
			}
			else
			{
				plcPointValue.value[i] = 0;
			}
			break;
		case PPI_VD_REG:
			plcPointValue.value[i] = readvd(fd, ppi_cmd_buf);
			break;
		case PPI_VW_REG:
			plcPointValue.value[i] = readvw(fd, ppi_cmd_buf);
			break;
		case PPI_VB_REG:
			plcPointValue.value[i] = readvb(fd, ppi_cmd_buf);
			break;
		case PPI_UNKNOWN_REG:
			plcPointValue.value[i] = 0;
			break;
		default:
			plcPointValue.value[i] = 0;
			break;
		}

		sprintf(plcPointValue_buffer+strlen(plcPointValue_buffer),"%d",plcPointValue.value[i]);
	}
}
