#include <stdio.h>   /* Standard input/output definitions */
#include <stdlib.h>  /* Standard General Utilities Library */
#include <string.h>  /* String function definitions */
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>

#include "uart.h"



void show_help(const char *progname)
{
	printf("usage: %s [-h] -d tty  [-b baudrate] [-r] \n", progname);
	printf("\n"
	"-d\t\t tty filedevice\n"
	"-b\t\t Baud rate, 115200, etc (115200 is default)\n"
	"-r\t\t RS-485\n"
	"-h\t\t This help\n"
	"\n");
}

enum state {
	STATE_NONE,
	STATE_TRUE,
	STATE_FALSE
};


struct config {
	const char *tty;
	int baud;
	enum state rs_485_usage;
};

static struct config config = {
	.tty = NULL,
	.baud = 0,
	.rs_485_usage = STATE_FALSE,
};

void opt_fail(const char *err)
{
	fprintf(stderr, "Error: %s\n", err);
	exit(1);
}


void parse_options(int argc, char **argv)
{
	int c;
	if (argc == 1) {
		show_help(argv[0]);
		exit(0);
	}
	while ((c = getopt(argc, argv, "hgrd:a:f:ib:s")) != -1) {
		switch (c) {
		case 'b':
			config.baud = atoi(optarg);
			break;
		case 'd':
			if (optarg != NULL) {
				config.tty = optarg;
			} else {
				opt_fail("Invalid -d argument!\n");
			}
			break;
		case 'r':
			config.rs_485_usage  = STATE_TRUE;
			break;
		case '?':
		case 'h':
		default:
			show_help(argv[0]);
			exit(0);
			break;
		
			/* сюда на самом деле попасть невозможно. */
			break;
		}
	}
}

static int fd = 0; //filesecriptor of COM-port

int init_reader (const char * com_port_device, int baud) {
	fd = init_comport(com_port_device, baud);
	
	//set_blocking(fd, 1);
	if (config.rs_485_usage) {
		if (!init_rs485 (fd)) {
			printf("Error init RS-485\n");
		} else {
			printf("Init RS-485 OK!\n");
		}
	}
	return fd;
}

int main(int argc, char *argv[]) {
	uint8_t buf[1024] = {0};	
	parse_options(argc, argv);
	if (init_reader (config.tty, config.baud) <= 0) {
		printf ("Error open COM port %s\n", config.tty);
		return -1;
	}
	set_rts(fd, 1);
	set_dtr(fd,1);
	int ret;
	while (1) {
		if (ret =  read_com(fd, 50, 1000, buf) !=0) {
				printf("ret = %d %s\n",ret,  buf);
		} else {
			printf("No data\n");
		}
		
	}
	
	return 0;

}
