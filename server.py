from __future__ import print_function

import Pyro4
import subprocess
import time
import os
import uuid
from PyRedPitaya.board import RedPitaya

def bin(s):
    ''' Returns the set bits in a positive int as a str.'''
    return str(s) if s<=1 else bin(s>>1) + str(s&1)

class rpServer(object):

    RPPINNAMES = [
        'RP_LED0',
        'RP_LED1',
        'RP_LED2',
        'RP_LED3',
        'RP_LED4',
        'RP_LED5',
        'RP_LED6',
        'RP_LED7',
        'RP_DIO0_P',
        'RP_DIO1_P',
        'RP_DIO2_P',
        'RP_DIO3_P',
        'RP_DIO4_P',
        'RP_DIO5_P',
        'RP_DIO6_P',
        'RP_DIO7_P',
        'RP_DIO0_N',
        'RP_DIO1_N',
        'RP_DIO2_N',
        'RP_DIO3_N',
        'RP_DIO4_N',
        'RP_DIO5_N',
        'RP_DIO6_N',
        'RP_DIO7_N',
    ]

    # The names are a enum, so mapping 0 to the first will allow lookups into the
    # enum in C. Defined in rp.h
    RPPINS =  dict(zip(RPPINNAMES, range(len(RPPINNAMES))))

    ## The bitshifts are from the definitions of the old dsp card.
    DigitalLineToRPPORT = {
    	1 << 0:RPPINS['RP_DIO1_P'], # 'EMCCDa'
    	1 << 1:RPPINS['RP_DIO2_P'], # 'EMCCDb'
    	1 << 6:RPPINS['RP_DIO3_P'], # 'BNSSLM'
        1 << 2:RPPINS['RP_DIO4_P'], # '405 DeepStar'
        1 << 3:RPPINS['RP_DIO5_P'], # '488 DeepStar'
        1 << 4:RPPINS['RP_DIO6_P'], # '561 Cobalt'
        1 << 5:RPPINS['RP_DIO7_P'], # '647 DeepStar'
    }

    def __init__(self):
        self.timeLineValues = []
        self.profileFile = None
        self.red_pitaya = RedPitaya()
        self.pid = None
        self.name = None

    ## In order to just toggle a pin, we create a short actionTable
    # turning the pin on and then off after 5 secs.
    # if we just turned it on, the end of the DSP program will reset everything
    # too 0 instantly.
    def high(self, name):
        with open('tmp', 'w') as f:
            f.write('0 0 {} 1\n'.format(self.RPPINS[name]))
            f.write('5 0 {} 0'.format(self.RPPINS[name]))
        cmd = ['./dsp', os.path.join(os.getcwd(), 'tmp')]
        subprocess.call(cmd)

    # raw version does not do pin name lookups.
    def analog_raw(self, pin, value):
        with open('tmp', 'w') as f:
            f.write('0 0 {} {}\n'.format(-1*pin, value))
            f.write('5 0 {} 0'.format(-1*pin))
        cmd = ['./dsp', os.path.join(os.getcwd(), 'tmp')]
        subprocess.call(cmd)

    def high_raw(self, pin):
        with open('tmp', 'w') as f:
            f.write('0 0 {} 1\n'.format(pin))
            f.write('5 0 {} 0'.format(pin))
        cmd = ['./dsp', os.path.join(os.getcwd(), 'tmp')]
        subprocess.call(cmd)

    # The dsp has a handler for SIGINT that cleans up
    def Abort(self):
        # kill the server process
        subprocess.call(['kill', '-9', str(self.pid)])

    def MoveAbsoluteADU(self, aline, aduPos):
        # probably just use the python lib
        # volts to ADU's for the DSP card: int(pos * 6553.6))
        pass

    def arcl(self, cameraMask, lightTimePairs):
        # wha?
        pass

    def profileSet(self, profileStr, digitals, *analogs): # This is downloading the action table
        # digitals is numpy.zeros((len(times), 2), dtype = numpy.uint32),
        # starting at 0 -> [times for digital signal changes, digital lines]
        # analogs is a list of analog lines and the values to put on them at each time

        for time, DigitalBitMask in zip(*digitals):
            for shift, value in enumerate(bin(DigitalBitMask)[::-1].ljust(6, '0')):
                self.timeLineValues.append( (time, self.DigitalLineToRPPORT[1<<shift], value) )

        for pin, analogLine in enumerate(analogs):
            print(pin, analogLine)
            for time, value in zip(*analogLine):
                self.timeLineValues.append( (time, -(pin+1), value*(2**31-1)))

        self.timeLineValues.sort(key=lambda x: x[0])

    def DownloadProfile(self): # This is saving the action table
        for time, line, value in self.timeLineValues:
            secs = repr(int(time/1e9))
            nanos = repr(int(time%1e9))
            print('{} {} {} {}'.format(secs, nanos, line, int(value)), file=self.profileFile)
        self.profileFile.flush()
        self.profileFile.close()

    def InitProfile(self, numReps):
        self.name = 'profile'
        self.timeLineValues = []
        self.profileFile = open(self.name, 'w')

    def trigCollect(self):
        cmd = ['./dsp', os.path.join(os.getcwd(), self.name)]
        print('calling', cmd)
        self.pid = subprocess.Popen(cmd).pid
        # this is the "run" return when done. No error checking so...

    def ReadPosition(self):
        pass

    def WriteDigital(self, level):
        self.red_pitaya.hk.led = level


dsp = rpServer()

print("providing dsp.d() as [pyroDSP] on port 7766")
print("Started program at",time.strftime("%A, %B %d, %I:%M %p"))

import random
daemon = Pyro4.Daemon(port = random.randint(2000, 10000), host = '192.168.1.100')
Pyro4.Daemon.serveSimple({dsp: 'pyroDSP'},
        daemon = daemon, ns = False, verbose = True)
