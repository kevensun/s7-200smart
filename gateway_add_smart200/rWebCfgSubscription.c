/*
 * rWebCfgSubscription.c
 *
 *  Created on: 2018-9-14
 *      Author: xm
 */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "rWebCfgCjson.h"
#include <malloc.h>
#include <sqlite3.h>
#include <pthread.h>
#include <curl/curl.h>
#include "rWebCfgCrc32.h"
#include <stdlib.h>
#include <sys/stat.h>
#include "typedef.h"
#include "rWebCfgRegister.h"
#include "database.h"

struct image_header_t//64K
{
	uint32_t        ih_magic;	/* Image Header Magic Number	*/
	uint32_t        ih_hcrc;	/* Image Header CRC Checksum	*/
	uint32_t        ih_time;	/* Image Creation Timestamp	*/
	uint32_t        ih_size;	/* Image Data Size		*/
	uint32_t        ih_load;	/* Data	 Load  Address		*/
	uint32_t        ih_ep;		/* Entry Point Address		*/
	uint32_t        ih_dcrc;	/* Image Data CRC Checksum	*/
	uint8_t	        ih_os;		/* Operating System		*/
	uint8_t         ih_arch;	/* CPU architecture		*/
	uint8_t		ih_type;	/* Image Type			*/
	uint8_t		ih_comp;	/* Compression Type		*/
	uint8_t		ih_name[32];	/* Image Name		*/
};

struct FileInfo
{
	int addr;
	int size;
	char name[24];
};

struct firmware_Respond
{
	int type;
	char result[10];
	char path[256];
}firmware_Respond;

struct deviceRestart_Respond
{
	int type;
	char result[10];
}deviceRestart_Respond;

void* download_firmware_thread_start(void *arg);

int check_firmware(const char *firmware_name);
static void get_newVersion(struct image_header_t Header,unsigned char *newVersion);
static int check_firmwareHeader(struct image_header_t Header);

void feedback_download_result(int result);
void depart_firmware(const char *firmware_name);

extern gatewayCfgInfo_t rWebGatewayCfgInfo;
extern pointTable_t rWebPointTable;
int subscription_sock_fd = 0;

int analysis_subscription()
{
	puts("analysis_subscription");

	FILE *fp = fopen("recv.data","r");
	fseek(fp,0,SEEK_END);
	long file_size = ftell(fp);
	if(file_size == 0)
	{
		puts("subscription info is null.");
		sleep(2);
		return -1;
	}

	char *p_malloc = (char *)malloc(file_size * sizeof(char));
	if(p_malloc == NULL)
	{
		puts("malloc fail.");
		return -1;
	}

	rewind(fp);
	fread(p_malloc,sizeof(char),file_size,fp);
	fclose(fp);

	char substr[] = "\r\n\r\n";
	char *p_str = strstr(p_malloc,substr);
//	puts(p_str+strlen(substr));
	p_str += strlen(substr);

	cJSON *root = cJSON_Parse(p_str);
	if(root == NULL)
	{
		puts("cJSON_Parse err : root = NULL");
		return -1;
	}

	int value = cJSON_GetObjectItem(root,"type")->valueint;

	int type = value;
	printf("type = %d\n",type);

	if(type == 0)//点表信息
	{
		memset(&rWebPointTable, 0, sizeof(pointTable_t));

		p_str = cJSON_GetObjectItem(root,"result")->valuestring;
		printf("result= %s\n",p_str);

		value = cJSON_GetObjectItem(root,"plcType")->valueint;
		printf("plcType = %d\n",value);
		rWebGatewayCfgInfo.plcType = value;

		value = cJSON_GetObjectItem(root,"listLength")->valueint;
		printf("listLength = %d\n",value);
		rWebPointTable.pointTableNumber = value;

		cJSON *arrayItem = cJSON_GetObjectItem(root,"list");
		int array_size = cJSON_GetArraySize(arrayItem);

		int i = 0;
		for(;i<array_size && i < PLC_POINT_MAX_NUMBER;i++)
		{
			cJSON *element = cJSON_GetArrayItem(arrayItem,i);

			value = cJSON_GetObjectItem(element,"id")->valueint;
			printf("id = %d\n",value);

			p_str = cJSON_GetObjectItem(element,"address")->valuestring;
			printf("address = %s\n",p_str);
			if(strlen(p_str) <= PLC_POINT_ADDRESS_MAX_LEN)
				memcpy(rWebPointTable.pointTable[i].address, p_str, strlen(p_str));
			else
				memcpy(rWebPointTable.pointTable[i].address, p_str, PLC_POINT_ADDRESS_MAX_LEN);

			p_str = cJSON_GetObjectItem(element,"name")->valuestring;
			printf("name = %s\n",p_str);
			if(strlen(p_str) <= PLC_POINT_NAME_MAX_LEN)
				memcpy(rWebPointTable.pointTable[i].name, p_str, strlen(p_str));
			else
				memcpy(rWebPointTable.pointTable[i].name, p_str, PLC_POINT_NAME_MAX_LEN);

			p_str = cJSON_GetObjectItem(element,"interval")->valuestring;
			printf("interval = %s\n",p_str);
			if(strlen(p_str) <= PLC_POINT_INTERVAL_MAX_LEN)
				memcpy(rWebPointTable.pointTable[i].readInterval, p_str, strlen(p_str));
			else
				memcpy(rWebPointTable.pointTable[i].readInterval, p_str, PLC_POINT_INTERVAL_MAX_LEN);

			p_str = cJSON_GetObjectItem(element,"datatype")->valuestring;
			printf("datatype = %s\n",p_str);
			if(strlen(p_str) <= PLC_POINT_DATATYPE_MAX_LEN)
				memcpy(rWebPointTable.pointTable[i].datatype, p_str, strlen(p_str));
			else
				memcpy(rWebPointTable.pointTable[i].datatype, p_str, PLC_POINT_DATATYPE_MAX_LEN);

			p_str = cJSON_GetObjectItem(element,"deviation")->valuestring;
			printf("deviation = %s\n",p_str);
			if(strlen(p_str) <= PLC_POINT_DEVIATION_MAX_LEN)
				memcpy(rWebPointTable.pointTable[i].deviation, p_str, strlen(p_str));
			else
				memcpy(rWebPointTable.pointTable[i].deviation, p_str, PLC_POINT_DEVIATION_MAX_LEN);
		}
	}
	else if(type == 1)//网关配置信息
	{
		p_str = cJSON_GetObjectItem(root,"result")->valuestring;
		printf("result = %s\n",p_str);

		cJSON *deviceInfo = cJSON_GetObjectItem(root,"deviceInfo");

		value = cJSON_GetObjectItem(deviceInfo,"id")->valueint;
		printf("id = %d\n",value);

		p_str = cJSON_GetObjectItem(deviceInfo,"deviceNum")->valuestring;
		printf("deviceNum = %s\n",p_str);

		value = cJSON_GetObjectItem(deviceInfo,"onLine")->valueint;
		printf("onLine = %d\n",value);

		p_str = cJSON_GetObjectItem(deviceInfo,"lanIp")->valuestring;
		printf("lanIp = %s\n",p_str);
		memset(rWebGatewayCfgInfo.netIp, 0, sizeof(rWebGatewayCfgInfo.netIp));
		if(strlen(p_str) <= GATEWAY_IP_INFO_MAX_LEN)
			memcpy(rWebGatewayCfgInfo.netIp, p_str, strlen(p_str));
		else
			memcpy(rWebGatewayCfgInfo.netIp, p_str, GATEWAY_IP_INFO_MAX_LEN);

		p_str = cJSON_GetObjectItem(deviceInfo,"wanIp")->valuestring;
		printf("wanIp = %s\n",p_str);

		p_str = cJSON_GetObjectItem(deviceInfo,"subnetMask")->valuestring;
		printf("subnetMask = %s\n",p_str);
		memset(rWebGatewayCfgInfo.netMask, 0, sizeof(rWebGatewayCfgInfo.netMask));
		if(strlen(p_str) <= GATEWAY_IP_INFO_MAX_LEN)
			memcpy(rWebGatewayCfgInfo.netMask, p_str, strlen(p_str));
		else
			memcpy(rWebGatewayCfgInfo.netMask, p_str, GATEWAY_IP_INFO_MAX_LEN);

		p_str = cJSON_GetObjectItem(deviceInfo,"defaultGateway")->valuestring;
		printf("defaultGateway = %s\n",p_str);
		memset(rWebGatewayCfgInfo.netGateway, 0, sizeof(rWebGatewayCfgInfo.netGateway));
		if(strlen(p_str) <= GATEWAY_IP_INFO_MAX_LEN)
			memcpy(rWebGatewayCfgInfo.netGateway, p_str, strlen(p_str));
		else
			memcpy(rWebGatewayCfgInfo.netGateway, p_str, GATEWAY_IP_INFO_MAX_LEN);

		value = cJSON_GetObjectItem(deviceInfo,"networkType")->valueint;
		printf("networkType = %d\n",value);
		rWebGatewayCfgInfo.netType = value;

		p_str = cJSON_GetObjectItem(deviceInfo,"thirdParty")->valuestring;
		printf("thirdParty = %s\n",p_str);
		memset(rWebGatewayCfgInfo.srvAddress, 0, sizeof(rWebGatewayCfgInfo.srvAddress));
		if(strlen(p_str) <= GATEWAY_SERVER_ADDRESS_INFO_MAX_LEN)
			memcpy(rWebGatewayCfgInfo.srvAddress, p_str, strlen(p_str));
		else
			memcpy(rWebGatewayCfgInfo.srvAddress, p_str, GATEWAY_SERVER_ADDRESS_INFO_MAX_LEN);

		p_str = cJSON_GetObjectItem(deviceInfo,"plcIp")->valuestring;
		printf("plcIp = %s\n",p_str);
		memset(rWebGatewayCfgInfo.plcIp, 0, sizeof(rWebGatewayCfgInfo.plcIp));
		if(strlen(p_str) <= GATEWAY_IP_INFO_MAX_LEN)
			memcpy(rWebGatewayCfgInfo.plcIp, p_str, strlen(p_str));
		else
			memcpy(rWebGatewayCfgInfo.plcIp, p_str, GATEWAY_IP_INFO_MAX_LEN);

		value = cJSON_GetObjectItem(deviceInfo,"plcRack")->valueint;
		printf("plcRack = %d\n",value);
		rWebGatewayCfgInfo.plcRack = value;

		value = cJSON_GetObjectItem(deviceInfo,"plcSlot")->valueint;
		printf("plcSlot = %d\n",value);
		rWebGatewayCfgInfo.plcSlot = value;

		value = cJSON_GetObjectItem(deviceInfo,"plcType")->valueint;
		printf("plcType = %d\n",value);
		rWebGatewayCfgInfo.plcType = value;
	}
	else if(type == 2)//固件更新
	{
		memset(&firmware_Respond,0,sizeof(firmware_Respond));

		firmware_Respond.type = value;

		p_str = cJSON_GetObjectItem(root,"result")->valuestring;
		printf("result = %s\n",p_str);
		memcpy(firmware_Respond.result,p_str,strlen(p_str));

		p_str = cJSON_GetObjectItem(root,"path")->valuestring;
		printf("path = %s\n",p_str);
		if(strlen(p_str)<sizeof(firmware_Respond.path))
		{
			memcpy(firmware_Respond.path,p_str,strlen(p_str));
		}
	}
	else if(type == 3)//设备重启
	{
		memset(&deviceRestart_Respond,0,sizeof(deviceRestart_Respond));

		deviceRestart_Respond.type = value;

		p_str = cJSON_GetObjectItem(root,"result")->valuestring;
		printf("result = %s\n",p_str);
		memcpy(deviceRestart_Respond.result,p_str,strlen(p_str));
	}

	puts("analysis END.");

	cJSON_Delete(root);
	free(p_malloc);

	return type;
}

void device_reboot()
{
	puts("device reboot.");
#ifndef PC
	system("reboot");
#endif
}

void subscription()
{
	char requestParam[100] = {0};
	sprintf(requestParam,"deviceNum=%s",rWebGatewayCfgInfo.devId);
	puts(requestParam);

	struct sockaddr_in server_sockaddr;
	memset(&server_sockaddr, 0, sizeof(server_sockaddr));
	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_port = htons(PORT);
	inet_aton(IP, &server_sockaddr.sin_addr);

	while(1)
	{

		subscription_sock_fd = socket(AF_INET, SOCK_STREAM, 0);

		int ret = connect(subscription_sock_fd,
				(struct sockaddr *)&server_sockaddr,sizeof(server_sockaddr));
		if(ret)
		{
			printf("subscription connect fail.\n");
			perror("subscription connect");
			close(subscription_sock_fd);
			return;
		}

		char sendbuf[1024] = {0};
		strcat(sendbuf, "POST /gateway/release HTTP/1.1\r\n");
		sprintf(sendbuf+strlen(sendbuf),"Host:%s\r\n",IP);
		sprintf(sendbuf+strlen(sendbuf),"Content-Length: %u\r\n\r\n",strlen(requestParam));
		strcat(sendbuf,requestParam);

		ret = send(subscription_sock_fd,sendbuf,strlen(sendbuf),0);

		puts("wait subscription ...");

		char recvbuf[1024] = {0};
		FILE *fp = fopen("recv.data","w");

		do
		{
			ret = recv(subscription_sock_fd,recvbuf,sizeof(recvbuf),0);
			fwrite(recvbuf,sizeof(char),ret,fp);
		}while(ret > 0);

		fclose(fp);
		close(subscription_sock_fd);

		int type = analysis_subscription();
		if(type == 0)//点表配置
		{
			databaseWriteGateWayCfg(&rWebGatewayCfgInfo);
			databaseWritePointTable(&rWebPointTable);

			device_reboot();
		}
		else if(type == 1)//设备配置
		{
			databaseWriteGateWayCfg(&rWebGatewayCfgInfo);
			databaseCleanPointTable(&rWebPointTable);

			device_reboot();
		}
		else if(type == 2)//固件更新,根据path 自行下载固件
		{
			char *p = strrchr(firmware_Respond.path,'/');
			if(p == NULL)
			{
				fprintf(stdout,"can't get firmware name.\n");
				feedback_download_result(2);
				continue;
			}

			char firmware_name[256] = {0};
			memcpy(firmware_name,p+1,strlen(p+1));
			printf("firmware name = %s\n",firmware_name);

			pthread_t ID;
			pthread_create(&ID, NULL, download_firmware_thread_start, firmware_name);
//			sprintf(cmdBuf, "curl %s >> %s",firmware_Respond.path, firmware_name);
//			printf("\n\n\n\n=============Upgrade Start=============\n\n\n\n");
//			system(cmdBuf);
//			printf("\n\n\n\n=============Upgrade End=============\n\n\n\n");
		}
		else if(type == 3)//设备重启
		{
			system("sync");
			device_reboot();
		}
	}
}


size_t receive_data(void *buffer, size_t size, size_t nmemb, FILE *file) {
    size_t r_size = fwrite(buffer, size, nmemb, file);
    return r_size;
}

void* download_firmware_thread_start(void *arg)
{
	puts("download_firmware_thread_start");

	const char *firmware_name = (char *)arg;

	FILE *fp = fopen(firmware_name, "w");

	CURL *curl = curl_easy_init();
	if(curl == NULL)
	{
		fprintf(stdout,"curl_easy_init err : curl = NULL.\n");
		return NULL;
	}

	curl_easy_setopt(curl, CURLOPT_URL,firmware_Respond.path);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	CURLcode code =  curl_easy_perform(curl);
	printf("curl code = %d : %s\n",code,curl_easy_strerror(code));
	fclose(fp);
	if(code != CURLE_OK)
	{
		feedback_download_result(2);
	}
	else//code = CURLE_OK
	{
		////ret: 1/success 2/fail 3/same
		int ret = check_firmware(firmware_name);
		feedback_download_result(ret);
		if(ret == 1)
		{
			depart_firmware(firmware_name);
		}
	}

	puts("remove firmware.");
	remove(firmware_name);

	return NULL;
}

void depart_firmware(const char *firmware_name)
{
	puts("depart firmware ...");
	puts("in ./upgradefiles/");
	char *dir = "./upgradefiles/";
	mkdir(dir,0777);

	const int Header_size = 64;
	const int FilesInfo_size = 1024;

	FILE *fp_r = fopen(firmware_name,"r");
	int offset = Header_size;

	while( offset < (Header_size + FilesInfo_size) )
	{
		struct FileInfo fileInfo;

		fseek(fp_r,offset,SEEK_SET);
		fread(&fileInfo,sizeof(char),sizeof(fileInfo),fp_r);
		offset += sizeof(fileInfo);

		if(fileInfo.size == 0)
		{
			break;
		}
		else
		{
			puts(fileInfo.name);

			char filepath[64] = {0};
			strcat(filepath,dir);
			strcat(filepath,fileInfo.name);

			FILE *fp_w = fopen(filepath,"w");
			fseek(fp_r,fileInfo.addr+Header_size,SEEK_SET);
			int readnum = 0;

			char readbuf[1024] = {0};
			while(readnum != fileInfo.size)
			{
				if((fileInfo.size - readnum)>1024)
				{
					fread(readbuf,sizeof(char),1024,fp_r);
					fwrite(readbuf,sizeof(char),1024,fp_w);
					readnum += 1024;
				}
				else
				{
					fread(readbuf,sizeof(char),(fileInfo.size-readnum),fp_r);
					fwrite(readbuf,sizeof(char),(fileInfo.size-readnum),fp_w);
					readnum += (fileInfo.size-readnum);
				}
			}

			fclose(fp_w);
		}
	}

	puts("depart firmware finish.");
}

void feedback_download_result(int result)
{
	printf("feedback_download_result:%d\n",result);

	CURL *curl = curl_easy_init();
	if(curl == NULL)
	{
		puts("curl_easy_init err : curl = NULL.");
		return;
	}

	char URL[100] = {0};
	sprintf(URL,"http://%s:%d/gateway/updateResult",IP,PORT);

	char postParam[50] = {0};
	sprintf(postParam,"deviceNum=%s&type=%d",rWebGatewayCfgInfo.devId,result);

	curl_easy_setopt(curl, CURLOPT_URL, URL);
	curl_easy_setopt(curl, CURLOPT_POST, 1L);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS,postParam);
//	curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
	CURLcode code =  curl_easy_perform(curl);
	printf("\ncurl code = %d : %s\n",code,curl_easy_strerror(code));
}

//ret: 1/success 2/fail 3/same
int check_firmware(const char *firmware_name)
{
	puts("check_firmware");
	FILE *fp = fopen(firmware_name,"r");
	if(fp == NULL)
	{
		perror("fopen");
		return 2;
	}

	struct image_header_t Header;
	fread(&Header,sizeof(char),sizeof(Header),fp);

	int ret = check_firmwareHeader(Header);
	if(ret == -1)
	{
		fprintf(stdout,"header check fail.\n");
		return 2;
	}

	unsigned char newVersion[3] = {0};
	get_newVersion(Header,newVersion);

	char str_newVersion[10] = {0};
	sprintf(str_newVersion,"%d.%d.%d",newVersion[0],newVersion[1],newVersion[2]);

	if(strcmp(str_newVersion,GATEWAY_SOFTWARE_VERSION) == 0)
	{
		puts("same version.");
		return 3;
	}

	unsigned char databuf[64] = {0};
	uint16_t ChunkLen = 0;
	uint32_t size = ntohl(Header.ih_size);
	uint32_t TempCRC = 0;

	uint32_t i;
    for(i = 0; i < size; i += ChunkLen)
    {
        //ChunkLen = MIN(size - i, 64);
    	ChunkLen = ((size - i)<64) ? (size - i) : 64;
        fread(databuf,1,ChunkLen,fp);
        TempCRC = crc32(TempCRC, databuf, ChunkLen);
    }
  //  printf("TempCRC = %x\n",TempCRC);

	uint32_t datacrc = Header.ih_dcrc;
	datacrc = ntohl(datacrc);
//	printf("datacrc = %x\n",datacrc);

    if(TempCRC == datacrc)
    {
    	puts("check success.");
    	return 1;
    }
    else
    {
    	puts("check fail.");
    	return 2;
    }
}

static int check_firmwareHeader(struct image_header_t Header)
{
	uint32_t i;
	uint32_t TempCRC = 0;
	unsigned char buffer_crc[1024] = {0};

	memcpy(buffer_crc,&Header,sizeof(Header));
    for(i=0;i<4;i++)
    {
    	buffer_crc[i+4] = 0;
    }

    TempCRC = crc32(TempCRC, buffer_crc, sizeof(Header));
  //  printf("TempCRC = %x\n",TempCRC);

	uint32_t headcrc = Header.ih_hcrc;
	headcrc = ntohl(headcrc);
//	printf("headcrc = %x\n",headcrc);

    if(TempCRC == headcrc)
    {
    	return 0;
    }
    else
    {
    	return -1;
    }
}

static void get_newVersion(struct image_header_t Header,unsigned char *newVersion)
{
	char *p_ver0 = strchr((char *)Header.ih_name,'_') + 1;
	char *p_ver1 = strchr((char *)Header.ih_name,'.') + 1;
	char *p_ver2 = strrchr((char *)Header.ih_name,'.') + 1;

	char temp[4] = {0};
	memcpy(temp,p_ver0,p_ver1-p_ver0);
	newVersion[0] = atoi(temp);
	printf("newVersion[0] = %d\n",newVersion[0]);

	memset(temp,0,sizeof(temp));
	memcpy(temp,p_ver1,p_ver2-p_ver1);
	newVersion[1] = atoi(temp);
	printf("newVersion[1] = %d\n",newVersion[1]);

	newVersion[2] = atoi(p_ver2);
	printf("newVersion[2] = %d\n",newVersion[2]);
}


