/*
 * rWebHeartbeat.c
 *
 *  Created on: 2018-9-15
 *      Author: xm
 */

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "rWebCfgHeartbeat.h"
#include "rWebCfgRegister.h"
#include "rWebCfgSubscription.h"
#include "typedef.h"

#define HEARTBEAT_RECONNECT_TIME 5
#define HEARTBEAT_TIME 20

static int heartbeat_sock_fd = 0;

#define DISCONNECT 1
#define CONNECTING 2
#define WAITRECV 3
#define RECV 4
static int heartbeat_flag = DISCONNECT;

int network_status = NETWORK_BAD;

extern gatewayCfgInfo_t rWebGatewayCfgInfo;


void* heartbeat_thread_start(void *arg)
{
	puts("heartbeat_thread_start");

	struct sockaddr_in server_sockaddr;
	memset(&server_sockaddr, 0, sizeof(server_sockaddr));
	server_sockaddr.sin_family = AF_INET;
	server_sockaddr.sin_port = htons(PORT);
	inet_aton(IP, &server_sockaddr.sin_addr);

	while(1)
	{
		heartbeat_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	//	printf("%d heartbeat_sock_fd = %d\n",__LINE__,heartbeat_sock_fd);

		heartbeat_flag = CONNECTING;
		int ret = connect(heartbeat_sock_fd,(struct sockaddr *)&server_sockaddr,sizeof(server_sockaddr));
		if(ret)
		{
			perror("heartbeat connect");

			heartbeat_flag = DISCONNECT;
			network_status = NETWORK_BAD;
			puts("NETWORK_BAD");

			close(heartbeat_sock_fd);
			sleep(HEARTBEAT_RECONNECT_TIME);
			continue;
		}

		heartbeat_flag = WAITRECV;

		//char postParam[] = "deviceNum=83776582848783780001";
		char postParam[100] = {0};
		sprintf(postParam,"deviceNum=%s",rWebGatewayCfgInfo.devId);

		char sendbuf[1024] = {0};
		strcat(sendbuf, "POST /gateway/heartbeat HTTP/1.1\r\n");
		sprintf(sendbuf+strlen(sendbuf),"Host:%s\r\n",IP);
		sprintf(sendbuf+strlen(sendbuf),"Content-Length: %u\r\n\r\n",strlen(postParam));
		strcat(sendbuf,postParam);

		ret = send(heartbeat_sock_fd,sendbuf,strlen(sendbuf),0);

		char recvbuf[1024] = {0};
		ret = recv(heartbeat_sock_fd,recvbuf,sizeof(recvbuf),0);
		if(ret < 1)
		{
			perror("heartbeat recv");
			printf("NETWORK_BAD:%d\n",ret);

			heartbeat_flag = DISCONNECT;
			network_status = NETWORK_BAD;

			close(heartbeat_sock_fd);
			continue;
		}
		else
		{
			//	puts("NETWORK_NORMAL");

			heartbeat_flag = RECV;
			network_status = NETWORK_NORMAL;

			close(heartbeat_sock_fd);
			sleep(HEARTBEAT_TIME);
		}
	}
}

void* heartbeat_timer_thread_start(void *arg)
{
	puts("heartbeat_timer_thread_start");

	static int times = 0;
	static int previous_network_status = 0;

	while(1)
	{
		if(heartbeat_flag == WAITRECV || heartbeat_flag == CONNECTING)
		{
			times++;
			if(times > 10 )
			{
				fprintf(stdout,"heartbeat respond timeout.\n");
				times = 0;
				shutdown(heartbeat_sock_fd,SHUT_RDWR);
			}
		}
		else
		{
			times = 0;
		}

		if(network_status == NETWORK_BAD)
		{
			if(previous_network_status == NETWORK_NORMAL)
			{
				puts("shutdown");
				shutdown(subscription_sock_fd,SHUT_RDWR);
				shutdown(register_sock_fd,SHUT_RDWR);
			}
		}

		previous_network_status = network_status;

		sleep(1);
	}
}

void start_heartbeat()
{
	pthread_t heartbeat_ID;
	pthread_create(&heartbeat_ID, NULL, heartbeat_thread_start, NULL);

	pthread_t heartbeat_timer_ID;
	pthread_create(&heartbeat_timer_ID, NULL, heartbeat_timer_thread_start, NULL);
}




