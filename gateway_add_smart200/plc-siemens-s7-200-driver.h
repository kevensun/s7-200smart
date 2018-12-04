/*
 * plc-siemens-s7-200-driver.h
 *
 *  Created on: 2018-9-17
 *      Author: xm
 */

#ifndef PLC_SIEMENS_S7_200_DRIVER_H_
#define PLC_SIEMENS_S7_200_DRIVER_H_

#ifdef __cplusplus    //__cplusplus是cpp中自定义的一个宏
extern "C" {          //告诉编译器，这部分代码按C语言的格式进行编译，而不是C++的
#endif

#include <termios.h>

int uart_open(const char *path);
int uart_config(int fd, int baud, char parity, int bsize, int stop);
int uart_read(int fd, char *buf, int len);
int uart_write(int fd, const char *data, int len);
int uart_close(int fd);


void show_termios(const struct termios *s);

int get_termios(int fd, struct termios *s);
int set_termios(int fd, const struct termios *s);

int baud_to_speed(int baud);
int get_speed(const struct termios *s);
int get_ispeed(const struct termios *s);
int get_ospeed(const struct termios *s);
int get_bsize(const struct termios *s);
char get_parity(const struct termios *s);
int get_stop(const struct termios *s);
int readable(const struct termios *s);

int speed_to_baud(int speed);
int set_speed(struct termios *s, int speed);
int set_ispeed(struct termios *s, int speed);
int set_ospeed(struct termios *s, int speed);
int set_bsize(struct termios *s, int bsize);
int set_parity(struct termios *s, char parity);
int set_stop(struct termios *s, int stop);
int enable_read(struct termios *s);
int disable_read(struct termios *s);
int enable_flow_control(struct termios *s);
int disable_flow_control(struct termios *s);

#ifdef __cplusplus
}
#endif

#endif /* PLC_SIEMENS_S7_200_DRIVER_H_ */
