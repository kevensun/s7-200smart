/*
 * rWebMain.c
 *
 *  Created on: 2018-9-14
 *      Author: xm
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "string.h"
#include "typedef.h"
#include "database.h"
#include "rWebCfgHeartbeat.h"
#include "rWebCfgRegister.h"
#include "rWebCfgSubscription.h"

gatewayCfgInfo_t rWebGatewayCfgInfo;
pointTable_t rWebPointTable;

void rWebCfgMain(void)
{
	int ret = access("./upgradefiles/",F_OK);
	if(ret == 0)
	{
		puts("rm -r ./upgradefiles/");
		system("rm -r ./upgradefiles/");
	}


	memset(&rWebGatewayCfgInfo, 0, sizeof(gatewayCfgInfo_t));
	memset(&rWebPointTable, 0, sizeof(pointTable_t));
	databaseReadDevId(rWebGatewayCfgInfo.devId);
	databaseReadGatewayCfg(&rWebGatewayCfgInfo);

	printf("plcType = %d\n",rWebGatewayCfgInfo.plcType);
	printf("webServerIP = %s:%d\n",IP,PORT);

	start_heartbeat();

	while(1)
	{
		if(network_status == NETWORK_NORMAL)
		{
			int ret = register_deviceInfo();
			if(ret == 0)
			{
				puts("register deviceInfo success.");
				subscription();
			}

			sleep(5);
		}
	}
}

