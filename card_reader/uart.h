#ifndef __UART_H__
#define __UART_H__
#include <stdbool.h>


/**
	* @brief  inittialize comport
	* @param  comport : string value represents com device, examle: "/dev/ttyUSB0"
	* @retval file decriptor of initialized port (<0 if failed)
	*/
int init_comport(const char *comport, int baud);

/**
	* @brief  reads from comport into ring buffer
	* @param  fd : comport file descriptor
	* @param  len : 
	* @param  timeout : time to wait for data, ms
	* @buff  buffer for read
	* @retval size of data peing placed in ringbuffer
	*/
int read_com(int fd, int len , int timeout, uint8_t * buff);
/**
	* @brief  writes into comport
	* @param  fd : comport file descriptor
	* @param  buf : pointer to data
	* @param  size : data size
	* @param  timeout : time to wait for port to be free, ms
	* @retval 1 - success, 0 - fail
	*/
int write_com(int fd, uint8_t * buf, size_t size, int timeout);

int init_rs485 (int fd);
int set_rts(int fd, int on);
int set_dtr(int fd, int on);

void set_blocking (int fd, int should_block);
#endif //__UART_H__


