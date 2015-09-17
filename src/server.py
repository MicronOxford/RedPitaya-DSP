#! /usr/bin/python
from __future__ import print_function

import Pyro4
import subprocess
import os
import time

import logging
import sys
logging.basicConfig()  # or your own sophisticated setup
logging.getLogger("Pyro4").setLevel(logging.DEBUG)
logging.getLogger("Pyro4.core").setLevel(logging.DEBUG)
# ... set level of other logger names as desired ...

root = logging.getLogger()
root.setLevel(logging.DEBUG)

ch = logging.StreamHandler(sys.stdout)
ch.setLevel(logging.DEBUG)
formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')
ch.setFormatter(formatter)
root.addHandler(ch)

from PyRedPitaya.board import RedPitaya

BASE            = 0xFFFF9000
COMM_RX_AT_ROWS = 0x10
COMM_RX_AT_FLAG = 0x14
COMM_RX_AT      = 0x18

def bin(s):
    ''' Returns the set bits in a positive int as a str.'''
    return str(s) if s<=1 else bin(s>>1) + str(s&1)


class Runner(object):

    def __init__(self):
        self.pid = None
        self.filename = '/tmp/actiontable'
        self.writtenActionTable = False
        self.running = None

    def loadDemo(self, dt):
        with open(self.filename, 'w') as f:
            n = 100000
            t = 0
            for l in range(n):
                print('{} {} {} {} {}'.format( t, 0, 0, 0, l*4000./n ), file=f)
                t += dt
                print('{} {} {} {} {}'.format( t, 0x000000FF, 0x000000FF, 4000, l*4000./n ), file=f)
                t += dt
        self.writtenActionTable = True

    def load(self, actiontable):
        print("in load")
        with open(self.filename, 'w') as f:
            for row in actiontable:
                time, digitals, a1, a2 = row
                dP, dN = digitals & int('11111111111', 2), (digitals & int('1111111100000000', 2)) >> 8
                finalrow = time, dP, dN, a1, a2
                print('time:{} digitalP:{} digitalN:{} a1:{} a2:{}'.format(*finalrow))
                print('{} {} {} {} {}'.format(*finalrow), file=f)
        self.writtenActionTable = True

    def abort(self):
        if self.pid:
            subprocess.call(['kill', '-9', str(self.pid)])
            self.pid = None

    def start(self):
        if self.writtenActionTable:
            cmd = ['dsp', self.filename]
            print('calling', cmd)
            proc = subprocess.Popen(cmd)
            self.pid = proc.pid
            return proc
        else:
            raise Exception("please write the action table!")

    def stop(self):
        self.abort()


class rpServer(object):

    def __init__(self):
        self.pid = None
        self.name = None
        self.times = []
        self.digitals = []
        self.analogA = []
        self.analogB = []

        self.DSPRunner = Runner()
        self.board = RedPitaya()

        # single value output
        self.board.asga.counter_wrap = self.board.asga.counter_step
        self.board.asgb.counter_wrap = self.board.asgb.counter_step

    # The dsp has a handler for SIGINT that cleans up
    def Abort(self):
        print("abort")
        # kill the server process
        self.DSPRunner.abort()

    def MoveAbsoluteADU(self, aline, aduPos):
        print("aline:", aline, "pos",aduPos)
        # probably just use the python lib
        # volts to ADU's for the DSP card: int(pos * 6553.6))
        # Bu we won't be hooked up to the stage?
        if aline == 0:
            self.board.asga.data = [aduPos]
        if aline == 1:
            self.board.asgb.data = [aduPos]
        else:
            print("aline {}>1".format(aline))

    def arcl(self, cameraMask, lightTimePairs):
        print("arcl")
        print("cameras:", cameraMask, "lightimePais:", lightTimePairs)
        # wha?
        # takes a image
        pass

    def profileSet(self, profileStr, digitals, *analogs):
        try:
            print("profileSet")
            print("called with")
            print(profileStr, '\n', digitals, '\n', *analogs)
            # This is downloading the action table
            # digitals is numpy.zeros((len(times), 2), dtype = numpy.uint32),
            # starting at 0 -> [times for digital signal changes, digital lines]
            # analogs is a list of analog lines and the values to put on them at each time
            # digitals = list of lists. sublist is a time, line pair
            # then 4 analog lines. also list of time: value pairs.
            Dtimes, dvals = zip(*digitals)
            Atimes, Avals = zip(*analogs[0])
            Btimes, Bvals = zip(*analogs[0])


            times, digitals, analogA, analogB = [], [], [], []
            times = set(Dtimes+Atimes+Btimes)
            for timepoint in times:
                self.times.append(timepoint)
                for outline, inval, timesForLine in [(digitals, dvals, Dtimes),
                                                     (analogA,  Avals, Atimes),
                                                     (analogB,  Bvals, Btimes)]:
                    if timepoint in timesForLine:
                        outline.append(inval[timesForLine.index(timepoint)])
                    else:
                        outline.append( inval[len(outline)-1] ) # the last value

            self.actiontable = zip(times, digitals, analogA, analogB)
            print("sort")
            self.actiontable.sort(key=lambda row: row[0])
        except Exception as e:
            print(e)
            raise e


    def DownloadProfile(self): # This is saving the action table
        print("DownloadProfile")
        self.DSPRunner.load(self.actiontable)

    def InitProfile(self, numReps):
        print("InitProfile")
        # I'm pretty sure this does not zero the prev values.
        #self.times, self.digitals, self.analogA, self.analogB = [], [], [], []

    def trigCollect(self):
        print("trigCollect")
        process = self.DSPRunner.start()
        process.wait()
        # needs to block on the dsp finishing

    def ReadPosition(self, axis):
        if axis == 0:
            return int(self.board.asga.data[0])
        elif axis == 1:
            return int(self.board.asgb.data[0])
        else:
            return 0 # FIXME

    def WriteDigital(self, level):
        self.board.hk.led = level

    def demo(self, dt):
        self.DSPRunner.loadDemo(dt)
        self.DSPRunner.start()

    def receiveClient(self, uri):
        print(uri)

if __name__ == '__main__':
    dsp = rpServer()

    print("providing dsp.d() as [pyroDSP] on port 7766")
    print("Started program at",time.strftime("%A, %B %d, %I:%M %p"))

    Pyro4.config.SERIALIZER = 'pickle'
    Pyro4.config.SERIALIZERS_ACCEPTED.add('pickle')

    while True:
        try:
            daemon = Pyro4.Daemon(port = 7000, host = '192.168.1.100')
            break
        except Exception as e:
            print("Socket fail", e)
            time.sleep(1)
    Pyro4.Daemon.serveSimple({dsp: 'pyroDSP'},
            daemon = daemon, ns = False, verbose = True)
