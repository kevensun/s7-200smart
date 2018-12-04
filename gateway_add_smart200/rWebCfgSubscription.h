/*
 * rWebCfgSubscription.h
 *
 *  Created on: 2018-9-14
 *      Author: xm
 */

#ifndef RWEBCFGSUBSCRIPTION_H_
#define RWEBCFGSUBSCRIPTION_H_

#ifdef __cplusplus    //__cplusplus是cpp中自定义的一个宏
extern "C" {          //告诉编译器，这部分代码按C语言的格式进行编译，而不是C++的
#endif

extern int subscription_sock_fd;
void subscription();

#ifdef __cplusplus
}
#endif

#endif /* RWEBCFGSUBSCRIPTION_H_ */
