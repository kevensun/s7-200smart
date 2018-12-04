/*
 * plc-mitsubishi-fx-3u.h
 *
 *  Created on: 2018-9-17
 *      Author: xm
 */

#ifndef PLC_MITSUBISHI_FX_3U_H_
#define PLC_MITSUBISHI_FX_3U_H_

#ifdef __cplusplus    //__cplusplus是cpp中自定义的一个宏
extern "C" {          //告诉编译器，这部分代码按C语言的格式进行编译，而不是C++的
#endif

void MITSUBISHI_FX_3U_Read_And_Send(void);

#ifdef __cplusplus
}
#endif

#endif /* PLC_MITSUBISHI_FX_3U_H_ */
