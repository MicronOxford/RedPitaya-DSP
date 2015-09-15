#! python
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
        self.filename = 'actiontable'
        self.running = None

    def loadDemo(self, dt):
        with open(self.filename, 'w') as f:
            n = 100000
            t = 0
            for l in range(n):
                print('{} {} {} {}'.format( t, 0, 0, l*4000./n ), file=f)
                t += dt
                print('{} {} {} {}'.format( t, 0x000000FF, 4000, l*4000./n ), file=f)
                t += dt

    def load(self, times, digitals, analogA, analogB):
        print("in load", times)
        with open(self.filename, 'w') as f:
            for row in zip([int(t) * 1e9 for t in times], digitals, analogA, analogB):
                print('{} {} {} {}'.format(*row), file=f)

    def abort(self):
        if self.pid:
            subprocess.call(['kill', '-9', str(self.pid)])
            self.pid = None

    def start(self):
        cmd = ['./dsp', os.path.join(os.getcwd(), self.filename)]
        print('calling', cmd)
        self.pid = subprocess.Popen(cmd).pid

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

    # The dsp has a handler for SIGINT that cleans up
    def Abort(self):
        print("abort")
        # kill the server process
        self.DSPRunner.abort()

    def MoveAbsoluteADU(self, aline, aduPos):
        # probably just use the python lib
        # volts to ADU's for the DSP card: int(pos * 6553.6))
        # Bu we won't be hooked up to the stage?
        pass

    def arcl(self, cameraMask, lightTimePairs):
        # wha?
        # takes a image
        pass

    def profileSet(self, profileStr, digitals, *analogs):
        print("profileSet")
        print("values recv")
        print(profileStr, digitals, *analogs)
        # This is downloading the action table
        # digitals is numpy.zeros((len(times), 2), dtype = numpy.uint32),
        # starting at 0 -> [times for digital signal changes, digital lines]
        # analogs is a list of analog lines and the values to put on them at each time
        self.times, self.digitals = digitals
        self.analogA = analogs[0]
        self.analogB = analogs[1]

    def DownloadProfile(self): # This is saving the action table
        print("DownloadProfile")
        self.DSPRunner.load(self.times, self.digitals, self.analogA, self.analogB)

    def InitProfile(self, numReps):
        print("InitProfile")
        self.times, self.digitals, self.analogA, self.analogB = [], [], [], []

    def trigCollect(self):
        print("trigCollect")
        self.DSPRunner.start()
        # needs to block on the dsp finishing

    def ReadPosition(self, axis):
        pass

    def WriteDigital(self, level):
        self.red_pitaya.hk.led = level

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

    import random
    daemon = Pyro4.Daemon(port = random.randint(2000, 10000), host = '192.168.1.100')
    Pyro4.Daemon.serveSimple({dsp: 'pyroDSP'},
            daemon = daemon, ns = False, verbose = True)
