/*
 * rWebCfgRegister.c
 *
 *  Created on: 2018-9-14
 *      Author: xm
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <ctype.h>
#include "typedef.h"
#include "rWebCfgCjson.h"

extern gatewayCfgInfo_t rWebGatewayCfgInfo;
int register_sock_fd = 0;

struct registerParam
{
	char deviceNum[GATEWAY_DEV_ID_MAX_LEN+1];
	char serialNumber[GATEWAY_DEV_ID_MAX_LEN+1];
	char version[GATEWAY_VER_INFO_MAX_LEN+1];
	char lanMac[GATEWAY_MAC_INFO_MAX_LEN+1];
	char lanIP[GATEWAY_IP_INFO_MAX_LEN+1];
	char lanMask[GATEWAY_IP_INFO_MAX_LEN+1];
	char lanGateway[GATEWAY_IP_INFO_MAX_LEN+1];
	char srvAddress[GATEWAY_SERVER_ADDRESS_INFO_MAX_LEN+1];
}registerParam;

struct register_deviceInfo_respond
{
	char result[10];
	int resultType;
	char message[200];
}register_deviceInfo_respond;



int analysis_register_deviceInfo_respond(char *recvbuf)
{
	char substr[] = "\r\n\r\n";
	char *p_str = strstr(recvbuf,substr);
//	puts(p_str+strlen(substr));
	p_str += strlen(substr);

	cJSON *root = cJSON_Parse(p_str);
	if(root == NULL)
	{
		puts("cJSON_Parse : root = NULL");
		return -1;
	}

	memset(&register_deviceInfo_respond,0,sizeof(register_deviceInfo_respond));

	p_str = cJSON_GetObjectItem(root,"result")->valuestring;
	memcpy(register_deviceInfo_respond.result,p_str,strlen(p_str));

	int value = cJSON_GetObjectItem(root,"resultType")->valueint;
	register_deviceInfo_respond.resultType = value;

	p_str = cJSON_GetObjectItem(root,"message")->valuestring;
	memcpy(register_deviceInfo_respond.message,p_str,strlen(p_str));

	cJSON_Delete(root);

	return 0;
}

void get_register_postParam(char *result,int size)
{
	memset(result,0,size);
	char buf[1024] = {0};
	sprintf(buf,"deviceNum=%s"
			"&serialNumber=%s"
			"&version=%s"
			"&lanMac=%s"
			"&lanIp=%s"
			"&subnetMask=%s"
			"&defaultGateway=%s"
			"&thirdParty=%s",
			registerParam.deviceNum,
			registerParam.serialNumber,
			registerParam.version,
			registerParam.lanMac,
			registerParam.lanIP,
			registerParam.lanMask,
			registerParam.lanGateway,
			registerParam.srvAddress);

	memcpy(result,buf,strlen(buf));
}


void getGateWay(char *gateway)
{
    FILE *fp;
    char buf[512];
    char cmd[128];
    char *tmp;

    strcpy(cmd, "ip route");
    fp = popen(cmd, "r");
    if(NULL == fp)
    {
        perror("popen error");
        return;
    }

    while(fgets(buf, sizeof(buf), fp) != NULL)
    {
        tmp =buf;
        while(*tmp && isspace(*tmp))
            ++ tmp;
        if(strncmp(tmp, "default", strlen("default")) == 0)
            break;
    }
    sscanf(buf, "%*s%*s%s", gateway);
    pclose(fp);
}

void set_registerParam(int sock_fd)
{
	int i;

	memcpy(registerParam.deviceNum, rWebGatewayCfgInfo.devId ,sizeof(rWebGatewayCfgInfo.devId));
	memcpy(registerParam.serialNumber, rWebGatewayCfgInfo.devId ,sizeof(rWebGatewayCfgInfo.devId));
	memcpy(registerParam.version, GATEWAY_SOFTWARE_VERSION,sizeof(GATEWAY_SOFTWARE_VERSION));
	memcpy(registerParam.srvAddress,rWebGatewayCfgInfo.srvAddress,sizeof(rWebGatewayCfgInfo.srvAddress));

	char ifcu_buf[1024] = {0};
	struct ifconf ifc;
	ifc.ifc_len = sizeof(ifcu_buf);
	ifc.ifc_ifcu.ifcu_buf = ifcu_buf;

	ioctl(sock_fd,SIOCGIFCONF,&ifc);

	int ifreq_num = ifc.ifc_len/sizeof(struct ifreq);

	for(i=0;i<ifreq_num;i++)
	{
		struct ifreq *ifrp = ifc.ifc_ifcu.ifcu_req+i;
		char ifreq_name[16] = {0};
		memcpy(ifreq_name,ifrp->ifr_ifrn.ifrn_name,sizeof(ifreq_name));
		printf("%s\n",ifreq_name);

		char name_lo[] = "lo";
		if(memcmp(ifreq_name,name_lo,strlen(name_lo)) == 0)
		{
			puts("detect lo,jump over.");
			continue;
		}

		ioctl(sock_fd,SIOCGIFHWADDR,ifrp);
		sprintf(registerParam.lanMac,"%02x:%02x:%02x:%02x:%02x:%02x",
				(unsigned char)ifrp->ifr_hwaddr.sa_data[0],
				(unsigned char)ifrp->ifr_hwaddr.sa_data[1],
				(unsigned char)ifrp->ifr_hwaddr.sa_data[2],
				(unsigned char)ifrp->ifr_hwaddr.sa_data[3],
				(unsigned char)ifrp->ifr_hwaddr.sa_data[4],
				(unsigned char)ifrp->ifr_hwaddr.sa_data[5]);
		printf("lanMac = %s\n",registerParam.lanMac);

		ioctl(sock_fd,SIOCGIFADDR,ifrp);
		struct sockaddr_in *addr = (struct sockaddr_in *)&ifrp->ifr_addr;
		char *address = inet_ntoa(addr->sin_addr);
		printf("lanIP = %s\n",address);
		memset(registerParam.lanIP,0,sizeof(registerParam.lanIP));
		memcpy(registerParam.lanIP,address,strlen(address));

//			ioctl(sock_fd,SIOCGIFBRDADDR,ifrp);
//			addr = (struct sockaddr_in *)&ifrp->ifr_broadaddr;
//			address = inet_ntoa(addr->sin_addr);
//			printf("%d broad = %s\n",__LINE__,address);

		ioctl(sock_fd,SIOCGIFNETMASK,ifrp);
		addr = (struct sockaddr_in *)&ifrp->ifr_netmask;
		address = inet_ntoa(addr->sin_addr);
		printf("lanMask = %s\n",address);
		memset(registerParam.lanMask,0,sizeof(registerParam.lanMask));
		memcpy(registerParam.lanMask,address,strlen(address));

		memset(registerParam.lanGateway,0,sizeof(registerParam.lanGateway));
		getGateWay(registerParam.lanGateway);
		printf("lanGateway = %s\n",registerParam.lanGateway);

//			ioctl(sock_fd,SIOCGIFFLAGS,ifrp);
//			unsigned short flag = ifrp->ifr_flags;//short int ifru_flags;
//			printf("%d flag = 0x%x\n",__LINE__,flag);
	}
}

/*
 * 设备注册
 */
int register_deviceInfo()
{
	puts("register_deviceInfo");

	struct sockaddr_in server_sockaddr;
	memset(&server_sockaddr, 0, sizeof(server_sockaddr));
	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_port = htons(PORT);
	inet_aton(IP, &server_sockaddr.sin_addr);

	register_sock_fd = socket(AF_INET, SOCK_STREAM, 0);

	int ret = connect(register_sock_fd,(struct sockaddr *)&server_sockaddr,sizeof(server_sockaddr));
	if(ret)
	{
		printf("register_deviceInfo connect fail.\n");
		perror("connect");
		close(register_sock_fd);
		return -1;
	}

	set_registerParam(register_sock_fd);

	char postParam[1024] = {0};
	get_register_postParam(postParam,sizeof(postParam));
	puts(postParam);

	char sendbuf[1024] = {0};
	strcat(sendbuf, "POST /gateway/register HTTP/1.1\r\n");
	sprintf(sendbuf+strlen(sendbuf),"Host:%s\r\n",IP);
	sprintf(sendbuf+strlen(sendbuf),"Content-Length: %u\r\n\r\n",strlen(postParam));
	strcat(sendbuf,postParam);

	ret = send(register_sock_fd,sendbuf,strlen(sendbuf),0);

	char recvbuf[2048] = {0};
	ret = recv(register_sock_fd,recvbuf,sizeof(recvbuf),0);
	if(ret < 1)
	{
		printf("register_deviceInfo recv err.\n");
		perror("recv");
		close(register_sock_fd);
		return -1;
	}
	else
	{
		int ret = analysis_register_deviceInfo_respond(recvbuf);
		if(ret == -1)
		{
			puts("analysis_register_deviceInfo_respond fail.");
			close(register_sock_fd);
			return -1;
		}
		else
		{
			if(strcmp(register_deviceInfo_respond.result,"success") != 0)
			{
				puts("register_deviceInfo_respond fail.");
				printf("message = %s\n",register_deviceInfo_respond.message);
				close(register_sock_fd);
				return -1;
			}
		}
	}

	close(register_sock_fd);

	return 0;
}





