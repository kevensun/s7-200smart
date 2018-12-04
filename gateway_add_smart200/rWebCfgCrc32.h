/*
 * rWebCfgCrc32.h
 *
 *  Created on: 2018-9-14
 *      Author: xm
 */

#ifndef RWEBCFGCRC32_H_
#define RWEBCFGCRC32_H_

#ifdef __cplusplus    //__cplusplus是cpp中自定义的一个宏
extern "C" {          //告诉编译器，这部分代码按C语言的格式进行编译，而不是C++的
#endif

#include "typedef.h"

#define        cpu_to_le32(x)		        (x)
#define        le32_to_cpu(x)		        (x)


uint32 crc32(uint32 crc, const uint8 *p, uint32 len);


#ifdef __cplusplus
}
#endif

#endif /* RWEBCFGCRC32_H_ */
