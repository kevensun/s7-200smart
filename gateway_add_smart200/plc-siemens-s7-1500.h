#ifndef PLC_SIEMENS_S7_1500_H_
#define PLC_SIEMENS_S7_1500_H_

#ifdef __cplusplus    //__cplusplus是cpp中自定义的一个宏
extern "C" {          //告诉编译器，这部分代码按C语言的格式进行编译，而不是C++的
#endif

#include <python2.7/Python.h>

extern PyObject *client;
extern PyObject *pmodule;

typedef enum Flag{
	connected,
	disconnected
}Flag;

extern Flag plc_connect_flag;

extern PyObject *create_plc_client();
extern int connect_plc(PyObject *client,char *ip,long rack,long slot);
extern void destroy_plc_client(PyObject *client);
extern int read_plc(PyObject *client,char *addr,int *value);

void SIEMENS_S7_1500_Init(void);
void SIEMENS_S7_1500_Read_And_Send(void);
void SIEMENS_S7_1500_End(void);

#ifdef __cplusplus
}
#endif

#endif /* PLC_SIEMENS_S7_1500_H_ */

