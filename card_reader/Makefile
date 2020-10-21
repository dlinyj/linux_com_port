# Makefile
TARGET = card_reader

#LDFLAGS = -lpthread -Llibfuse -l:libfuse3.so

#CC = /home/dlinyj/Загрузки/OpenWrt-SDK-mxs_gcc-4.8-linaro_uClibc-0.9.33.2_eabi.Linux-x86_64/staging_dir/toolchain-arm_arm926ej-s_gcc-4.8-linaro_uClibc-0.9.33.2_eabi/bin/arm-openwrt-linux-gcc
#CFLAGS = -std=c99 -Wall -DOPENWRT_ARM

CC = gcc
CFLAGS = -Wall -g

#LIBS  = -lkernel32 -luser32 -lgdi32 -lopengl32
#CFLAGS = -Wall -D_REENTERANT -D_FILE_OFFSET_BITS=64 -Ilibfuse -pthread



OBJ = $(TARGET).o    \
	uart.o 


%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@ $(LDFLAGS)

$(TARGET): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS) $(LDFLAGS)

clean:
	rm -f *.o $(TARGET) 
