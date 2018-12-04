/*
 * plcMain.h
 *
 *  Created on: 2018-9-14
 *      Author: xm
 */

#ifndef PLCMAIN_H_
#define PLCMAIN_H_

#ifdef __cplusplus    //__cplusplus是cpp中自定义的一个宏
extern "C" {          //告诉编译器，这部分代码按C语言的格式进行编译，而不是C++的
#endif

extern pointTable_t plcPointTable;
extern char plcPointValue_buffer[1024];
extern gatewayCfgInfo_t plcGatewayCfgInfo;

void plcMain(void);

#ifdef __cplusplus
}
#endif

#endif /* PLCMAIN_H_ */
