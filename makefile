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

dsp: dsp.o libxil.a
	@echo ">> Linking $@"
	$(CC) $(LIBRARIES) -fPIC -o dsp dsp.o libxil.a

dsp.o: dsp.c
	$(CC) $(CFLAGS) -c dsp.c -o dsp.o

atest: Atest.c
	$(CC) $(CFLAGS) -O0 Atest.c -o atest $(LNK)
	scp atest root@192.168.1.100:/tmp/

libxil.so: $(SRC)
	$(CC) -fPIC -shared -o libxil.so $^

libxil.a: $(OBJ)
	$(PREFIX)ar -cqv libxil.a $^

%.o: %.c
	$(CC) -std=c99 -O2 -c -g -DUSE_AMP=1 -o $@ $^

clean:
	rm -f dsp atest *.o standalone/*.o libxil.so *.a
