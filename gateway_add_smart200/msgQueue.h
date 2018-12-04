/*
 * msgQueue.h
 *
 *  Created on: 2018-9-14
 *      Author: xm
 */

#ifndef MSGQUEUE_H_
#define MSGQUEUE_H_

#ifdef __cplusplus    //__cplusplus是cpp中自定义的一个宏
extern "C" {          //告诉编译器，这部分代码按C语言的格式进行编译，而不是C++的
#endif

#include "typedef.h"

int msgQInit(void);
void msgQSendToQuarkioe(msgQueueData_t *msgData);
void msgQSendToPlc(msgQueueData_t *msgData);
void msgQSendToRWeb(msgQueueData_t *msgData);
void msgQReceiveForQuarkioe(msgQueueData_t *msgData);
void msgQReceiveForPlc(msgQueueData_t *msgData);
void msgQReceiveForRWeb(msgQueueData_t *msgData);

#ifdef __cplusplus
}
#endif

#endif /* MSGQUEUE_H_ */
