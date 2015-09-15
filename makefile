CFLAGS=-Wall -g -O2
LNK= -L. -lrp
OBJ=$(patsubst %.c,%.o,$(wildcard standalone/*.c))
SRC=$(wildcard standalone/*.c)
IP=192.168.1.100

PREFIX=arm-linux-gnueabi-
CC=$(PREFIX)gcc

all: dsp

scp: dsp server.py atest
	scp dsp server.py atest root@$(IP):/tmp/

dsp: dsp.c timer.c rpouts.c fpga_awg.c
	$(CC) $(CFLAGS) -o dsp dsp.c timer.c rpouts.c fpga_awg.c

atest: Atest.c
	$(CC) $(CFLAGS) -O0 Atest.c fpga_awg.c -lm -o atest $(LNK)

clean:
	rm -f dsp atest *.o standalone/*.o libxil.so *.a
