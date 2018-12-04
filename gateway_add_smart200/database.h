/*
 * database.h
 *
 *  Created on: 2018-9-14
 *      Author: xm
 */

#ifndef DATABASE_H_
#define DATABASE_H_

#ifdef __cplusplus    //__cplusplus是cpp中自定义的一个宏
extern "C" {          //告诉编译器，这部分代码按C语言的格式进行编译，而不是C++的
#endif

#include "typedef.h"

int databaseInit(void);
int databaseDefaultDevId(void);
int databaseReadDevId(char *devId);
int databaseWriteGateWayCfg(gatewayCfgInfo_t *gcf);
int databaseReadGatewayCfg(gatewayCfgInfo_t *gcf);
int databaseWritePointTable(pointTable_t *pt);
int databaseReadPointTable(pointTable_t *pt);
int databaseCleanPointTable(pointTable_t *pt);

#ifdef __cplusplus
}
#endif

#endif /* DATABASE_H_ */
