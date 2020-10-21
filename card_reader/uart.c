#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <sys/poll.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>

#include <termios.h> /* POSIX terminal control definitions */
#include <linux/serial.h> // for RS-485
#include <sys/ioctl.h>

#include "uart.h"

#define FNDELAY	O_NDELAY
#define RTS_SET	1
#define RTS_CLR 0



// converts integer baud to Linux define
static int get_baud(int baud)
{
	switch (baud) {
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
	case 230400:
		return B230400;
	case 460800:
		return B460800;
	case 500000:
		return B500000;
	case 576000:
		return B576000;
	case 921600:
		return B921600;
	case 1000000:
		return B1000000;
	case 1152000:
		return B1152000;
	case 1500000:
		return B1500000;
	case 2000000:
		return B2000000;
	case 2500000:
		return B2500000;
	case 3000000:
		return B3000000;
	case 3500000:
		return B3500000;
	case 4000000:
		return B4000000;
	default: 
		return -1;
	}
}

/**
	* @brief  inittialize comport
	* @param  comport : string value represents com device, examle: "/dev/ttyUSB0"
	* @retval file decriptor of initialized port (<0 if failed)
	*/
int init_comport(const char *comport, int baud){
	int fd = 0;										/* File descriptor */
	struct termios options;
	fd = open(comport, O_RDWR | O_NOCTTY | O_NDELAY);



	if (fd < 0){														/* Could not open the port */
		fprintf(stderr, "open_port: Unable to open %s - %s\n", comport,
		strerror(errno));
		//~ exit (-1);
		return fd;
	}

	/* Configure port reading */
	//fcntl(fd, F_SETFL, 0); 	//read com-port is the bloking
	fcntl(fd, F_SETFL, FNDELAY);  //read com-port not bloking
	//fcntl(fd, F_SETFL, O_NDELAY);  //read com-port not bloking
	
	//ioctl(fd, FIOASYNC, 1);
	
/* Get the current options for the port */
	tcgetattr(fd, &options);
	if(0 != baud) {
		cfsetispeed(&options,get_baud(baud));
	} else {
		cfsetispeed(&options, B115200);	
	}
	//cfsetispeed(&options, B9600);	
	 
/* Enable the receiver and set local mode */
	options.c_cflag |= (CLOCAL | CREAD);
	options.c_cflag &= ~PARENB; 						/* Mask the character size to 8 bits, no parity */
//	options.c_cflag &= ~CSTOPB; 						//one stop bit
	options.c_cflag |= CSTOPB;							//two stop bit
	options.c_cflag &= ~CSIZE;
	options.c_cflag |=  CS8;							/* Select 8 data bits */
	options.c_cflag &= ~CRTSCTS;						/* Disable hardware flow control */  
	options.c_oflag &= ~OPOST;							/* Disable postprocessing */  
	

	/* Software flow control is disabled */
	options.c_iflag &= ~(IXON | IXOFF | IXANY);
 
/* Enable data to be processed as raw input */
	options.c_lflag &= ~(ICANON | ECHO | ISIG);
/* Set the new options for the port */


// Добавлено для проверки
	options.c_cc[VMIN]  = 50;
	options.c_cc[VTIME] = 10; 
	
	options.c_iflag |= IGNBRK;
//До этого момента всё завелось
	options.c_iflag &= ~ICRNL;
	options.c_oflag &= ~ONLCR;
	options.c_lflag &= ~IEXTEN;
	options.c_lflag &= ~ECHOE;
	options.c_lflag &= ~ECHOK;
#ifndef OPENWRT_ARM
	options.c_lflag &= ~ECHOCTL;
	options.c_lflag &= ~ECHOKE;
#endif
	tcsetattr(fd, TCSANOW, &options);
	return fd;
}


void set_blocking (int fd, int should_block){
	struct termios tty;
	memset (&tty, 0, sizeof tty);
	if (tcgetattr (fd, &tty) != 0) {
		 printf("Error tcgetattr: %s\n", strerror(errno));
		return;
	}

	tty.c_cc[VMIN]  = should_block ? 1 : 0;
	tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

	if (tcsetattr (fd, TCSANOW, &tty) != 0)
		printf("Error tcsetattr: %s\n", strerror(errno));
}

/**
	* @brief  reads from comport into ring buffer
	* @param  fd : comport file descriptor
	* @param  len : 
	* @param  timeout : time to wait for data, ms
	* @buff  buffer for read
	* @retval size of data peing placed in ringbuffer
	*/
int read_com(int fd, int len , int timeout, uint8_t * buff){
	int ret = 0;
	
	struct pollfd fds;
	fds.fd=fd;
	fds.events = POLLIN;
	poll(&fds, 1, timeout);
	if(fds.revents & POLLIN) {
		ret = read(fd, buff, len);
	}
	if(ret<0){
		ret = 0;
	}
	return ret; 
}


/**
	* @brief  writes into comport
	* @param  fd : comport file descriptor
	* @param  buf : pointer to data
	* @param  size : data size
	* @param  timeout : time to wait for port to be free, ms
	* @retval 1 - success, 0 - fail
	*/
int write_com(int fd, uint8_t * buf, size_t size, int timeout){
//	tcflush(fd, TCIOFLUSH);
	int ret = 0;

	struct pollfd fds;
	fds.fd=fd;
	fds.events = POLLOUT;
	
	poll(&fds, 1, timeout);
	if(fds.revents & POLLOUT){
#ifdef USE_RTS
		set_rts(fd,RTS_SET);
#endif
		ret = write(fd, (uint8_t*)buf, size);

		tcdrain(fd); 
#ifdef USE_RTS
		set_rts(fd,RTS_CLR);
#endif
	}
	
	if(ret!=size) return 0;
	return 1; 
}


/* For the two functions below:
 *     level=0 to set line to LOW
 *     level=1 to set line to HIGH
 */

int set_rts(int fd, int on){
	int flags;

	ioctl(fd, TIOCMGET, &flags);
	if (on) {
		flags |= TIOCM_RTS;
	} else {
		flags &= ~TIOCM_RTS;
	}
	ioctl(fd, TIOCMSET, &flags);
	return 1;
}


int set_dtr(int fd, int on){
	int flags;

	ioctl(fd, TIOCMGET, &flags);
	if (on) {
		flags |= TIOCM_DTR;
	} else {
		flags &= ~TIOCM_DTR;
	}
	ioctl(fd, TIOCMSET, &flags);
	return 1;
}

int init_rs485 (int fd) {
	struct serial_rs485 rs485conf;

	// Get
	if (ioctl(fd, TIOCGRS485, &rs485conf) < 0) {
		return -1;
	}
	// Set
		/* Enable RS485 mode: */
	rs485conf.flags |= SER_RS485_ENABLED;

	/* Set logical level for RTS pin equal to 1 when sending: */
	//rs485conf.flags |= SER_RS485_RTS_ON_SEND;
	/* or, set logical level for RTS pin equal to 0 when sending: */
	//rs485conf.flags &= ~(SER_RS485_RTS_ON_SEND);

	/* Set logical level for RTS pin equal to 1 after sending: */
	//rs485conf.flags |= SER_RS485_RTS_AFTER_SEND;
	/* or, set logical level for RTS pin equal to 0 after sending: */
	//rs485conf.flags &= ~(SER_RS485_RTS_AFTER_SEND);

	/* Set rts delay before send, if needed: */
	rs485conf.delay_rts_before_send = 0;

	/* Set rts delay after send, if needed: */
	rs485conf.delay_rts_after_send = 0;

	/* Set this flag if you want to receive data even whilst sending data */
	rs485conf.flags |= SER_RS485_RX_DURING_TX;
	if (ioctl(fd, TIOCSRS485, &rs485conf) < 0) {
		return -1;
	}
	return 0;
}
