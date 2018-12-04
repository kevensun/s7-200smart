/*
 * plc-siemens-s7-200.h
 *
 *  Created on: 2018-9-17
 *      Author: xm
 */

#ifndef PLC_SIEMENS_S7_200_H_
#define PLC_SIEMENS_S7_200_H_

#ifdef __cplusplus    //__cplusplus是cpp中自定义的一个宏
extern "C" {          //告诉编译器，这部分代码按C语言的格式进行编译，而不是C++的
#endif

void SIEMENS_S7_200_Init(void);
void SIEMENS_S7_200_Read_And_Send(void);
void SIEMENS_S7_200_End(void);

#ifdef __cplusplus
}
#endif

#endif /* PLC_SIEMENS_S7_200_H_ */
