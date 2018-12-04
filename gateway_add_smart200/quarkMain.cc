/*
 * quarkMain.cc
 *
 *  Created on: 2018-9-14
 *      Author: xm
 */

/*
 * Copyright (C) 2015-2017 Cumulocity GmbH
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <iostream>
#include <cstdlib>
#include <sragent.h>
#include <srreporter.h>
#include <srlogger.h>
#include "srdevicepush.h"
#include "srutils.h"
#include "quarkIntegrate.h"

#include <unistd.h>
#include <sys/ipc.h>
#include "database.h"
#include "msgQueue.h"
#include "plcMain.h"
#include "rWebMain.h"
#include "createSrTemplate.h"

using namespace std;

sysStatus_t quarkSysStatus;
pointTable_t quarkPointTable;
gatewayCfgInfo_t quarkGatewayCfgInfo;

msgQueueData_t msgQueueData;

void* quarkThreadHandler(void* arg)
{
	while(1)
	{
		msgQReceiveForQuarkioe(&msgQueueData);
	}

	return NULL;
}

class CPUMeasurement: public SrTimerHandler
{
public:
    CPUMeasurement()
    {
    }

    virtual ~CPUMeasurement()
    {
    }

    virtual void operator()(SrTimer &timer, SrAgent &agent)
    {
    	char buf[1024] = {0};
		sprintf(buf,"200,%s,",agent.ID().c_str());

    	if(strcmp(msgQueueData.data,"")!=0)
		{
			strcat(buf,msgQueueData.data);
		//	memset(msgQueueData.data,0,sizeof(msgQueueData.data));
		}
		else
		{
			for(int i=0;i<quarkPointTable.pointTableNumber;i++)
			{
				if(i!=0)
				{
					strcat(buf,",");
				}
				strcat(buf,"0");
			}
		}

    	agent.send(SrNews(buf));
    }
};

class RestartHandler: public SrMsgHandler
{
public:
    RestartHandler()
    {
    }

    virtual ~RestartHandler()
    {
    }

    virtual void operator()(SrRecord &r, SrAgent &agent)
    {
        agent.send("105," + r.value(2) + ",EXECUTING");
        for (unsigned int i = 0; i < r.size(); ++i)
        {
            cerr << r.value(i) << " ";
        }
        cerr << endl;

        agent.send("105," + r.value(2) + ",SUCCESSFUL");
    }
};

int main()
{
	//del by yanghailong
    //const char* const server = "https://wentest.quarkioe.com";
    const char* const credentialPath = "./credentialInfo";
    //del by yanghailong
    //const char* const deviceID = "18612345678"; // unique device identifier

    /***************************************************************************
     * add by yanghailong ===>
     **************************************************************************/
    pid_t pid;
    pthread_t tid;

    memset(&quarkSysStatus, 0, sizeof(sysStatus_t));
    memset(&quarkGatewayCfgInfo, 0, sizeof(gatewayCfgInfo_t));
    memset(&quarkPointTable, 0, sizeof(pointTable_t));

    databaseInit();
    databaseDefaultDevId();
    databaseReadDevId(quarkGatewayCfgInfo.devId);

	if(msgQInit() != 0)
	{
		printf("msg queue init error.\n");
		return 0;
	}

//	  rWebCfgMain();
//	  plcMain();
//	  return 0;

	if((pid = fork()) < 0)
	{
		printf("fork error.\n");
		return 0;
	}
	else if(pid > 0)
	{
		plcMain();
		return 0;
	}

	if((pid = fork()) < 0)
	{
		printf("fork error.\n");
		return 0;
	}
	else if(pid > 0)
	{
		rWebCfgMain();
		return 0;
	}

    /*
     * 读取数据库，检查server地址是否已经配置，未配置则等待配置成功
     */
    if(databaseReadGatewayCfg(&quarkGatewayCfgInfo) == -1)
    {
    	quarkSysStatus.isSrvAddressAvailable = NOT_AVAILABLE;
    }
    else
    {
    	quarkGatewayCfgInfo.srvAddress[GATEWAY_SERVER_ADDRESS_INFO_MAX_LEN] = '\0';
    	if(strlen(quarkGatewayCfgInfo.srvAddress) < 1)
    	{
    		quarkSysStatus.isSrvAddressAvailable = NOT_AVAILABLE;
    	}
    	else
    	{
        	quarkSysStatus.isSrvAddressAvailable = AVAILABLE;
    	}
    }

    while(1)
    {
        if(quarkSysStatus.isSrvAddressAvailable == AVAILABLE)
        	break;

        sleep(5);

        if(databaseReadGatewayCfg(&quarkGatewayCfgInfo) == -1)
        {
        	quarkSysStatus.isSrvAddressAvailable = NOT_AVAILABLE;
        	printf("quarkSysStatus.isSrvAddressAvailable = NOT_AVAILABLE\n");
        }
        else
        {
        	quarkGatewayCfgInfo.srvAddress[GATEWAY_SERVER_ADDRESS_INFO_MAX_LEN] = '\0';
        	if(strlen(quarkGatewayCfgInfo.srvAddress) < 1)
        	{
        		quarkSysStatus.isSrvAddressAvailable = NOT_AVAILABLE;
            	printf("quarkSysStatus.isSrvAddressAvailable = NOT_AVAILABLE\n");
        	}
        	else
        	{
            	quarkSysStatus.isSrvAddressAvailable = AVAILABLE;
            	printf("quarkSysStatus.isSrvAddressAvailable = AVAILABLE\n");
        	}
        }
    }

    /*
     * 读取数据库，检查点表信息是否已经配置，未配置则等待配置成功
     */
    if(databaseReadPointTable(&quarkPointTable) == -1)
    	quarkSysStatus.isPointTableAvailable = NOT_AVAILABLE;
    else
    	quarkSysStatus.isPointTableAvailable = AVAILABLE;

    while(1)
    {
    	if(quarkSysStatus.isPointTableAvailable == AVAILABLE)
    	{
    		databaseReadGatewayCfg(&quarkGatewayCfgInfo);
    		break;
    	}

    	sleep(5);

        if(databaseReadPointTable(&quarkPointTable) == -1)
        {
        	printf("quarkSysStatus.isPointTableAvailable = NOT_AVAILABLE\n");
        	quarkSysStatus.isPointTableAvailable = NOT_AVAILABLE;
        }
        else
        {
        	printf("quarkSysStatus.isPointTableAvailable = AVAILABLE;\n");
        	quarkSysStatus.isPointTableAvailable = AVAILABLE;
        }
    }

//    printf("quarkPointTable.pointTableNumber:%d\n", quarkPointTable.pointTableNumber);
//    for(int index = 0; index < quarkPointTable.pointTableNumber; index++)
//    {
//    	puts(quarkPointTable.pointTable[index].address);
//    	puts(quarkPointTable.pointTable[index].name);
//    	puts(quarkPointTable.pointTable[index].readInterval);
//    }

    create_srTemplate();


	if(pthread_create(&tid, NULL, quarkThreadHandler, NULL) != 0)
	{
		printf("pthread_create fail.\n");
		return 0;
	}

//	getchar();

    /***************************************************************************
     * add by yanghailong <===
     **************************************************************************/

    srLogSetLevel(SRLOG_DEBUG);        // set log level to debug

    Integrate igt;

    SrAgent agent(quarkGatewayCfgInfo.srvAddress, quarkGatewayCfgInfo.devId, &igt); // instantiate SrAgent
    if (agent.bootstrap(credentialPath))   // bootstrap to Cumulocity
    {
        return 0;
    }

    string srversion, srtemplate;
    if (readSrTemplate("srtemplate.txt", srversion, srtemplate) != 0)
    {
        return 0;
    }

    if (agent.integrate(srversion, srtemplate)) // integrate to Cumulocity
    {
        return 0;
    }

    CPUMeasurement cpu;
    SrTimer timer(2 * 1000, &cpu); // Instantiate a SrTimer
    agent.addTimer(timer);          // Add the timer to agent scheduler
    timer.start();                  // Activate the timer

    SrReporter reporter(quarkGatewayCfgInfo.srvAddress, agent.XID(), agent.auth(), agent.egress, agent.ingress);
    if (reporter.start() != 0)      // Start the reporter thread
    {
        return 0;
    }

    // Inform Cumulocity about supported operations
    agent.send("104," + agent.ID() + ",\"\"\"c8y_Restart\"\"\"");
    RestartHandler restartHandler;
    agent.addMsgHandler(502, &restartHandler);
    SrDevicePush push(quarkGatewayCfgInfo.srvAddress, agent.XID(), agent.auth(), agent.ID(), agent.ingress);

    if (push.start() != 0)      // Start the device push thread
    {
        return 0;
    }

    agent.loop();

    return 0;
}
