# Copyright (C) 2015 Tom Parks
#
# RedPitaya-DSP is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# RedPitaya-DSP is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with RedPitaya-DSP. If not, see <http://www.gnu.org/licenses/>.

#! /usr/bin/python
from __future__ import print_function

import Pyro4
import subprocess
import threading
import os
import time
from mmap import mmap
import struct

import logging
import traceback
import sys

def bin(s):
    ''' Returns the set bits in a positive int as a str.'''
    return str(s) if s<=1 else bin(s>>1) + str(s&1)

def busy_wait(dt):
    '''Wait without sleeping for higher accuracy.'''
    current_time = time.time()
    while (time.time() < current_time+dt):
        pass


from types import FunctionType
from functools import wraps

def printfunc(func):
    @wraps(func)
    def wrapped(*args, **kwrds):
        print(func)
        try:
            return func(*args, **kwrds)
        except Exception as e:
            print(e)
            traceback.print_exc()
            raise e
    return wrapped

class PrintMetaClass(type):
    def __new__(meta, classname, bases, classDict):
        newClassDict = {}
        for attributeName, attribute in classDict.items():
            if type(attribute) == FunctionType:
                # replace it with a wrapped version
                attribute = printfunc(attribute)
            newClassDict[attributeName] = attribute
        return type.__new__(meta, classname, bases, newClassDict)

class RedPitayaBoard(object):

    offset_init = 0x40000000
    offset_size = 0x4022FFFF - offset_init

    direct_pinP = 0x10
    direct_pinN = 0x14
    out_pinP    = 0x18
    out_pinN    = 0x1C
    in_pinP     = 0x20
    in_pinN     = 0x24
    led         = 0x30

    asg_chanelA = 0x210000
    asg_chanelB = 0x220000

    def __init__(self):
        with open('/dev/mem', 'r+b') as f:
            self.mem = mmap(f.fileno(), self.offset_size,
                offset = self.offset_init)

    def read(self, offset, safe = True):
        if safe and offset not in [self.direct_pinP, self.direct_pinN,
        self.out_pinP, self.out_pinN, self.in_pinP, self.in_pinN,
        self.led, self.asg_chanelA, self.asg_chanelB]:
            raise NameError('offset {0:#x} not implemented'.format(offset))

        return struct.unpack('<L', self.mem[offset:offset+4])[0]

    def write(self, offset, value, safe = True):
        if safe and offset not in [self.direct_pinP, self.direct_pinN,
        self.out_pinP, self.out_pinN, self.in_pinP, self.in_pinN,
        self.led, self.asg_chanelA, self.asg_chanelB]:
            raise NameError('offset {0:#x} not implemented'.format(offset))

        self.mem[offset:offset+4] = struct.pack('<L', value)

class Runner(object):

    def __init__(self):
        self.pid = None
        self.filename = None
        self.execRunnerFile = 'build/bin/dsp'
        self.actionTablesDirectory = 'actionTables/'
        self.writtenActionTable = False
        self.running = None

    def loadDemo(self, deltaTime):
        self.filename = self.actionTablesDirectory + 'actTblTest.txt'
        with open(self.filename, 'w') as f:
            demoDigitalPin = 5
            demoAnalogueOutput = -1
            maxOfLines = 100000
            time = 0
            for line in range(maxOfLines):
                print('{} {} {}'.format(time, demoDigitalPin, line%2), file=f)
                print('{} {} {}'.format(time,
                    demoAnalogueOutput, line*8000./maxOfLines), file=f)
                # print('{} {} {}\n{} {} {}'.format(time, demoDigitalPin, line%2,
                #     time, demoAnalogueOutput, line*8000./maxOfLines), file=f)
                    time += deltaTime*1e3
        self.writtenActionTable = True

    def load(self, actiontable):
        self.filename = self.actionTablesDirectory + 'actTbl-' +
            time.strftime('%Y%m%d-%H:%M:%S') + '.txt'
        try:
            with open(self.filename, 'w') as f:
                for row in actiontable:
                    actTime, pinNumber, actionValue = row
                    timeNanoSec = int(actTime*1e3) # convert to ns
                    finalRow = timeNanoSec, pinNumber, actionValue
                    print('time:{} pin:{} action:{}'.format(*finalRow))
                    print('{} {} {}'.format(*finalRow), file=f)
            self.writtenActionTable = True
        except Exception as caughtException:
            print(caughtException)

    def abort(self):
        if self.pid:
            subprocess.call(['kill', '-9', str(self.pid)])
            self.pid = None

    def start(self):
        if self.writtenActionTable:
            comandLine = [self.execRunnerFile, self.filename]
            print('calling {}'.format(comandLine))
            process = subprocess.Popen(comandLine)
            self.pid = process.pid
            return process
        else:
            raise Exception('Please write the action table!')

    def stop(self):
        self.abort()

@Pyro4.expose
class RedPitayaServer(object):

    __metaclass__ = PrintMetaClass

    def __init__(self):
        self.pid = None
        self.name = None
        self.times = []
        self.digitals = []
        self.analogA = []
        self.analogB = []

        self.runner = Runner()
        self.board = RedPitayaBoard()

        # single value output
        # self.board.asga.counter_wrap = self.board.asga.counter_step
        # self.board.asgb.counter_wrap = self.board.asgb.counter_step

        # set all pins to out
        self.board.write(RedPitayaBoard.direct_pinP, 0xFF)
        self.board.write(RedPitayaBoard.direct_pinN, 0xFF)

        self.clientConnection = None

        self.fakeALines = [0, 0, 0, 0]

    def abort(self):
        # kill the server process
        self.runner.abort()
        # The RedPitaya-DSP has a handler for SIGINT that cleans up

    def arcl(self, cameras, lightTimePairs):
        if lightTimePairs:
            # Expose all lights at the start, then drop them out
            # as their exposure times come to an end.
            # Sort so that the longest exposure time comes last.
            lightTimePairs.sort(key = lambda a: a[1])
            curDigital = cameras + sum([p[0] for p in lightTimePairs])
            self.writeDigital(curDigital)
            print('Start with {}'.format(curDigital))
            totalTime = lightTimePairs[-1][1]
            curTime = 0
            for line, runTime in lightTimePairs:
                # Wait until we need to open this shutter.
                waitTime = runTime - curTime
                if waitTime > 0:
                    busy_wait(waitTime/1000.)
                curDigital -= line
                self.writeDigital(curDigital)
                curTime += waitTime
                print('At {} set {}'.format(curTime, curDigital))
            # Wait for the final timepoint to close shutters.
            if totalTime - curTime:
                busy_wait( (totalTime - curTime)/1000. )
            print('Finally at {} set {}'.format(totalTime, 0))
            self.writeDigital(0)
        else:
            self.writeDigital(cameras) # "expose"
            self.writeDigital(0)

    # def profileSet(self, profileStr, digitals, *analogs):
    def setProfile(self, times, pins, values):
        print('Setting action table...', end=' ')
        self.actiontable = zip(times, pins, values)
        print('done')


    def downloadProfile(self): # This is saving the action table
        self.runner.load(self.actiontable)

    def initProfile(self, numReps):
        # I'm pretty sure this does not zero the prev values.
        # is it for allocating space?
        #self.times, self.digitals, self.analogA, self.analogB = [], [], [], []
        pass

    def trigCollect(self, wait = True):
        process = self.runner.start()
        if wait:
            process.wait()
        if self.clientConnection
            retVal = (100, [self.readPosition(0), self.readPosition(1), 0, 0])
            self.clientConnection.receiveData('DSP done', retVal)
        # needs to block on the dsp finishing.

    def moveAbsoluteADU(self, aline, aduPos):
        # probably just use the python lib
        # volts to ADU's for the DSP card: int(pos * 6553.6))
        # But we won't be hooked up to the stage?
        if aline == 0:
            self.board.write(RedPitayaBoard.asg_chanelA, aduPos)
        elif aline == 1:
            self.board.write(RedPitayaBoard.asg_chanelB, aduPos)
        else:
            print('aline {}>1'.format(aline))
            self.fakeALines[aline] = aduPos

    def readPosition(self, axis):
        if axis == 0:
            return int(self.board.read(RedPitayaBoard.asg_chanelA))
        elif axis == 1:
            return int(self.board.read(RedPitayaBoard.asg_chanelB))
        else:
            return self.fakeALines[axis]

    def writeDigital(self, level, value):
        toWrite = value & int('11111111', 2)
        if level == 0:
            self.board.write(RedPitayaBoard.out_pinP, toWrite)
        elif level == 1:
            self.board.write(RedPitayaBoard.out_pinN, toWrite)

    def readDigital(self, level):
        if level == 0:
            return self.board.read(RedPitayaBoard.out_pinP)
        elif level == 1:
            return self.board.read(RedPitayaBoard.out_pinN)
        else:
            return None

    def writeLed(self, leds):
        self.board.write(RedPitayaBoard.led, leds)

    def readLed(self, leds):
        self.board.read(RedPitayaBoard.led)

    def demo(self, dt):
        self.runner.loadDemo(dt)
        process = self.runner.start()
        process.wait()

    def receiveClient(self, uri):
        self.clientConnection = Pyro4.Proxy(uri)
        print(uri)


if __name__ == '__main__':
    dsp = RedPitayaServer()
    dspPort = 7000
    dspHost = '10.42.0.175'

    print('Started program at {}'.format(time.strftime('%A, %B %d, %I:%M %p')))

    Pyro4.config.SERIALIZER = 'pickle'
    Pyro4.config.SERIALIZERS_ACCEPTED.add('pickle')

    while True:
        try:
            dspDaemon = Pyro4.Daemon(port = dspPort, host = dspHost)
            break
        except Exception as e:
            print('Socket fail {}'.format(e))
            time.sleep(1)

    print('Providing dsp.d() as [pyroDSP] at {}'.format(dspDaemon.locationStr))
    Pyro4.Daemon.serveSimple({dsp: 'pyroDSP'},
        daemon = dspDaemon, ns = False, verbose = True)
