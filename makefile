# Copyright (C) 2015 Tom Parks
# Copyright (C) 2017-2018 Tiago Susano Pinto <tiagosusanopinto@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

CFLAGS=-Wall -g -O2 -I./include
IP=192.168.1.100
# Could also build this from source, https://github.com/clade/RedPitaya
OS=http://clade.pierre.free.fr/python-on-red-pitaya/ecosystem-0.92-0-devbuild.zip

#PREFIX=arm-linux-gnueabi-
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

objs/primeNums.o: src/primeNums.c objs
	$(CC) $(CFLAGS) -c $< -o $@

objs/rpouts.o: src/rpouts.c include/rpouts.h objs
	$(CC) $(CFLAGS) -c $< -o $@

objs/timeControl.o: src/timeControl.c include/timeControl.h
	$(CC) $(CFLAGS) -c $< -o $@

objs/actionTable.o: src/actionTable.c include/actionTable.h
	$(CC) $(CFLAGS) -c $< -o $@

dsp: objs/dsp.o objs/timeControl.o objs/rpouts.o objs/actionTable.o
	mkdir -p build/bin
	$(CC) $(CFLAGS) -o build/bin/$@ $^

primeNums: objs/primeNums.o
	mkdir -p build/bin
	$(CC) $(CFLAGS) -o build/bin/$@ $^

server: src/server.py
	mkdir -p build/bin
	cp src/server.py build/bin/server
	chmod +x build/bin/server

rpos: tmp build
	-wget $(OS) --no-clobber -O tmp/ecosystem-0.92-0-devbuild.zip
	unzip tmp/ecosystem-0.92-0-devbuild.zip -d build/

pythonpackages: rpos
	# we need pyro and he red pitaya libs, --no-deps as they are both satisfied
	# allreday on the RP but pip could not know this
	pip install -t $(PYTHONLIBS) --no-dependencies $(PYTHONPACKAGES)

atest: Atest.c
	$(CC) $(CFLAGS) -O0 Atest.c -lm -o atest $(LNK)

scp:
	scp build/bin/server build/bin/dsp root@$(IP):/tmp/

clean:
	rm -rf dsp atest primeNums *.o include/*.o libxil.so *.a objs/ bin/ build/
