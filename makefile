CFLAGS=-Wall -g -O2 -I./include
IP=192.168.1.100
OS=http://clade.pierre.free.fr/python-on-red-pitaya/ecosystem-0.92-0-devbuild.zip

PREFIX=arm-linux-gnueabi-
CC=$(PREFIX)gcc

all: dsp server rpos

objs:
	mkdir objs

tmp:
	mkdir tmp

build:
	mkdir build

objs/dsp.o: src/dsp.c objs
	$(CC) $(CFLAGS) -c $< -o $@

objs/fpga_awg.o: src/fpga_awg.c include/fpga_awg.h objs
	$(CC) $(CFLAGS) -c $< -o $@

objs/rpouts.o: src/rpouts.c include/rpouts.h objs
	$(CC) $(CFLAGS) -c $< -o $@

objs/timer.o: src/timer.c include/timer.h include/xparameters.h objs
	$(CC) $(CFLAGS) -c $< -o $@

dsp: objs/dsp.o objs/timer.o objs/rpouts.o objs/fpga_awg.o
	mkdir -p build/bin
	$(CC) $(CFLAGS) -o build/bin/$@ $^

server: src/server.py
	mkdir -p build/bin
	cp src/server.py build/bin/server

rpos: tmp build
	wget $(OS) -O tmp/ecosystem-0.92-0-devbuild.zip
	unzip tmp/ecosystem-0.92-0-devbuild.zip -d build/

atest: Atest.c
	$(CC) $(CFLAGS) -O0 Atest.c fpga_awg.c -lm -o atest $(LNK)

clean:
	rm -rf dsp atest *.o include/*.o libxil.so *.a objs/ bin/ tmp/ build/
