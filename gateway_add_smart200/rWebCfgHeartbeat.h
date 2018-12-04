/*
 * rWebHeartbeat.h
 *
 *  Created on: 2018-9-15
 *      Author: xm
 */

#ifndef RWEBHEARTBEAT_H_
#define RWEBHEARTBEAT_H_

#ifdef __cplusplus    //__cplusplus是cpp中自定义的一个宏
extern "C" {          //告诉编译器，这部分代码按C语言的格式进行编译，而不是C++的
#endif

#define NETWORK_BAD		1
#define NETWORK_NORMAL 2

extern int network_status;

void start_heartbeat(void);

#ifdef __cplusplus
}
#endif

#endif /* RWEBHEARTBEAT_H_ */
