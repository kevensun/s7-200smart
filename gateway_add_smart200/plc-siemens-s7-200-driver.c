/*
 * plc-siemens-s7-200-driver.c
 *
 *  Created on: 2018-9-17
 *      Author: xm
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include "plc-siemens-s7-200-driver.h"

/*******************************************************************************
 *
 ******************************************************************************/
int uart_open(const char *path)
{
	if( path )
	{
		return open(path, O_RDWR|O_NOCTTY);
	}
	return -1;
}

/*******************************************************************************
 *
 ******************************************************************************/
int uart_config(int fd, int baud, char parity, int bsize, int stop)
{
	if( fd < 0 )
	{
		return -1;
	}
	else
	{
		struct termios newDev = {0};

		bzero(&newDev, sizeof(newDev));
		if( set_speed(&newDev, baud) )
		{
			return -2;
		}
		if( set_parity(&newDev, parity) )
		{
			return -3;
		}
		if( set_bsize(&newDev, bsize) )
		{
			return -4;
		}
		if( set_stop(&newDev, stop) )
		{
			return -5;
		}
		enable_read(&newDev);
		disable_flow_control(&newDev);
		if( set_termios(fd, &newDev) )
		{
			return -6;
		}
	}
	return 0;
}

/*******************************************************************************
 *
 ******************************************************************************/
int uart_read(int fd, char *buf, int len)
{
	if( fd < 0 || 0 == buf || len < 1 )
	{
		return -1;
	}
	return read(fd, buf, len);
}

/*******************************************************************************
 *
 ******************************************************************************/
int uart_write(int fd,  const char *data, int len)
{
	if( fd < 0 || 0 == data || len < 1 )
	{
		return -1;
	}
	return write(fd, data, len);
}

/*******************************************************************************
 *
 ******************************************************************************/
int uart_close(int fd)
{
	return close(fd);
}






/*******************************************************************************
 *
 ******************************************************************************/
void show_termios(const struct termios *s)
{
	if( s )
	{
		printf("speed=%d\n", get_speed(s));
		printf("parity=%c\n", get_parity(s));
		printf("bsize=%d\n", get_bsize(s));
		printf("stop=%d\n", get_stop(s));
	}
}

/*******************************************************************************
 *
 ******************************************************************************/
int get_termios(int fd, struct termios *s)
{
	if( -1 == fd || 0 == s )
	{
		return -1;
	}
	return tcgetattr(fd, s);
}

/*******************************************************************************
 *
 ******************************************************************************/
int set_termios(int fd, const struct termios *s)
{
	if( -1 == fd || 0 == s )
	{
		return -1;
	}
	return tcsetattr(fd, TCSANOW, s);
}

/*******************************************************************************
 *
 ******************************************************************************/
int baud_to_speed(int baud)
{
	switch( baud )
	{
		case B1200:
			return 1200;
		case B2400:
			return 2400;
		case B4800:
			return 4800;
		case B9600:
			return 9600;
		case B19200:
			return 19200;
		case B38400:
			return 38400;
		case B57600:
			return 57600;
		case B115200:
			return 115200;
	}
	return 0;
}

/*******************************************************************************
 *
 ******************************************************************************/
int get_speed(const struct termios *s)
{
	if( s )
	{
		return baud_to_speed(s->c_cflag & CBAUD);
	}
	return -1;
}

/*******************************************************************************
 *
 ******************************************************************************/
int get_ispeed(const struct termios *s)
{
	if( s )
	{
		return baud_to_speed(s->c_iflag & CBAUD);
	}
	return -1;
}

/*******************************************************************************
 *
 ******************************************************************************/
int get_ospeed(const struct termios *s)
{
	if( s )
	{
		return baud_to_speed(s->c_oflag & CBAUD);
	}
	return 0;
}

/*******************************************************************************
 *
 ******************************************************************************/
int get_bsize(const struct termios *s)
{
	if( s )
	{
		switch(s->c_cflag & CSIZE)
		{
			case CS5:
				return 5;
			case CS6:
				return 6;
			case CS7:
				return 7;
			case CS8:
				return 8;
		}
	}
	return -1;
}

/*******************************************************************************
 *
 ******************************************************************************/
char get_parity(const struct termios *s)
{
	if( s )
	{
		if( s->c_cflag & PARENB )
		{
			if( s->c_cflag & PARODD )
			{
				return 'O';
			}
			else
			{
				return 'E';
			}
		}
		return 'N';
	}
	return -1;
}

/*******************************************************************************
 *
 ******************************************************************************/
int get_stop(const struct termios *s)
{
	if( s )
	{
		if( s->c_cflag & CSTOPB )
		{
			return 2;
		}
		return 1;
	}
	return -1;
}

/*******************************************************************************
 *
 ******************************************************************************/
int speed_to_baud(int speed)
{
	switch(speed)
	{
		case 1200:
			return B1200;
		case 2400:
			return B2400;
		case 4800:
			return B4800;
		case 9600:
			return B9600;
		case 19200:
			return B19200;
		case 38400:
			return B38400;
		case 57600:
			return B57600;
		case 115200:
			return B115200;
	}
	return B9600;
}

/*******************************************************************************
 *
 ******************************************************************************/
int set_speed(struct termios *s, int speed)
{
	if( s )
	{
		s->c_cflag &= ~CBAUD;
		s->c_cflag |= speed_to_baud(speed);
		return 0;
	}
	return -1;
}

/*******************************************************************************
 *
 ******************************************************************************/
int set_ispeed(struct termios *s, int speed)
{
	if( s )
	{
		return cfsetispeed(s, speed_to_baud(speed) );
	}
	return -1;
}

/*******************************************************************************
 *
 ******************************************************************************/
int set_ospeed(struct termios *s, int speed)
{
	if( s )
	{
		return cfsetospeed(s, speed_to_baud(speed) );
	}
	return -1;
}

/*******************************************************************************
 *
 ******************************************************************************/
int set_bsize(struct termios *s, int bsize)
{
	if( s )
	{
		s->c_cflag &= ~CSIZE;
		switch( bsize )
		{
			case 5:
				s->c_cflag |= CS5;
				break;
			case 6:
				s->c_cflag |= CS6;
				break;
			case 7:
				s->c_cflag |= CS7;
				break;
			case 8:
				s->c_cflag |= CS8;
				break;
		}
		return 0;
	}
	return -1;
}

/*******************************************************************************
 *
 ******************************************************************************/
int set_parity(struct termios *s, char parity)
{
	if( s )
	{
		switch(parity)
		{
			case 'n':
			case 'N':
				s->c_cflag &= ~PARENB;
				break;
			case 'o':
			case 'O':
				s->c_cflag |= PARENB;
				s->c_cflag |= PARODD;
				break;
			case 'e':
			case 'E':
				s->c_cflag |= PARENB;
				s->c_cflag &= ~PARODD;
				break;
		}
		return 0;
	}
	return -1;
}

/*******************************************************************************
 *
 ******************************************************************************/
int set_stop(struct termios *s, int stop)
{
	if( s )
	{
		if( 1 == stop )
		{
			s->c_cflag &= ~CSTOPB;
		}
		else
		{
			s->c_cflag |= CSTOPB;
		}
		return 0;
	}
	return -1;
}

/*******************************************************************************
 *
 ******************************************************************************/
int enable_read(struct termios *s)
{
	if( s )
	{
		s->c_cflag |= CREAD;
		return 0;
	}
	return -1;
}

/*******************************************************************************
 *
 ******************************************************************************/
int disable_read(struct termios *s)
{
	if( s )
	{
		s->c_cflag &= ~CREAD;
		return 0;
	}
	return -1;
}

/*******************************************************************************
 *
 ******************************************************************************/
int enable_flow_control(struct termios *s)
{
	if( s )
	{
		s->c_cflag |= CRTSCTS;
		return 0;
	}
	return -1;
}

/*******************************************************************************
 *
 ******************************************************************************/
int disable_flow_control(struct termios *s)
{
	if( s )
	{
		s->c_cflag &= ~CRTSCTS;
		return 0;
	}
	return -1;
}


