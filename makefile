CFLAGS=-Wall
LNK= -L. -lrp
OBJ=$(patsubst %.c,%.o,$(wildcard standalone/*.c))
SRC=$(wildcard standalone/*.c)

PREFIX=arm-linux-gnueabi-
CC=$(PREFIX)gcc

all: dsp libs xtime

scp: dsp server.py
	scp dsp server.py root@192.168.1.100:/tmp/
	scp librp.so libxil.so root@192.168.1.100:/tmp/

dsp: dsp.c timer.c
	$(CC) $(CFLAGS) -o dsp dsp.c timer.c

atest: Atest.c
	$(CC) $(CFLAGS) -O0 Atest.c -o atest $(LNK)
	scp atest root@192.168.1.100:/tmp/

clean:
	rm -f dsp atest *.o standalone/*.o libxil.so *.a
