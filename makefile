CFLAGS=-Wall
LNK= -L. -L/home/thomas/Projects/linux-xlnx/include/ -lrp

all: dsp

dsp: dsp.c
	arm-linux-gnueabi-gcc $(CFLAGS) dsp.c -o dsp $(LNK)
	scp dsp root@192.168.1.100:/tmp/

clean:
	rm dsp
