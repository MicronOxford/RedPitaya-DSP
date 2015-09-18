CFLAGS=-Wall -g -O2 -I./include
IP=192.168.1.100
# Could also build this from source, https://github.com/clade/RedPitaya
OS=http://clade.pierre.free.fr/python-on-red-pitaya/ecosystem-0.92-0-devbuild.zip

PREFIX=arm-linux-gnueabi-
CC=$(PREFIX)gcc

PYTHONLIBS=build/usr/lib/Python2.7
PYTHONPACKAGES=PyRedPitaya Pyro4==4.28 serpent

all: dsp server rpos pythonpackages

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
	chmod +x build/bin/server

rpos: tmp build
	wget $(OS) --no-clobber -O tmp/ecosystem-0.92-0-devbuild.zip
	unzip tmp/ecosystem-0.92-0-devbuild.zip -d build/

pythonpackages: rpos
	# we need pyro and he red pitaya libs, --no-deps as they are both satisfied
	# allreday on the RP but pip could not know this
	pip install -t $(PYTHONLIBS) --no-dependencies $(PYTHONPACKAGES)

atest: Atest.c
	$(CC) $(CFLAGS) -O0 Atest.c fpga_awg.c -lm -o atest $(LNK)

scp:
	scp build/bin/server build/bin/dsp root@$(IP):/tmp/

clean:
	rm -rf dsp atest *.o include/*.o libxil.so *.a objs/ bin/ build/
