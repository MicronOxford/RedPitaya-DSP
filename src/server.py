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

import logging
import traceback
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

# from PyRedPitaya.board import RedPitaya

BASE            = 0xFFFF9000
COMM_RX_AT_ROWS = 0x10
COMM_RX_AT_FLAG = 0x14
COMM_RX_AT      = 0x18

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


class Runner(object):

    def __init__(self):
        self.pid = None;
        self.filename = None;
        self.execRunnerFile = 'build/bin/dsp';
        self.actionTablesDirectory = 'actionTables/';
        self.writtenActionTable = False;
        self.running = None;

    def loadDemo(self, deltaTime):
        self.filename = self.actionTablesDirectory + 'actTblTest.txt';
        with open(self.filename, 'w') as f:
            demoDigitalPin = 5;
            demoAnalogueOutput = -1;
            maxOfLines = 100000;
            time = 0;
            for line in range(maxOfLines):
                print('{} {} {}'.format(time, demoDigitalPin, line%2), file=f);
                print('{} {} {}'.format(time, demoAnalogueOutput, line*8000./maxOfLines), file=f);
                time += deltaTime*1e3;
        self.writtenActionTable = True;

    def load(self, actiontable):
        self.filename = self.actionTablesDirectory + 'actTbl-' + time.strftime("%Y%m%d-%H:%M:%S")+".txt";
        try:
            with open(self.filename, 'w') as f:
                for row in actiontable:
                    actTime, pinNumber, actionValue = row;
                    timeNanoSec = int(actTime*1e3); # convert to ns
                    finalRow = timeNanoSec, pinNumber, actionValue;
                    print('time:{} pin:{} action:{}'.format(*finalRow));
                    print('{} {} {}'.format(*finalRow), file=f);
            self.writtenActionTable = True;
        except Exception as caughtException:
            print(caughtException);

    def abort(self):
        if self.pid:
            subprocess.call(['kill', '-9', str(self.pid)])
            self.pid = None

    def start(self):
        if self.writtenActionTable:
            # comandLine = ['dsp', self.filename];
            comandLine = [self.execRunnerFile, self.filename];
            print('calling', comandLine);
            process = subprocess.Popen(comandLine);
            self.pid = process.pid;
            return process;
        else:
            raise Exception("Please write the action table!");

    def stop(self):
        self.abort()

# @Pyro4.expose # needed in newer versions
class rpServer(object):

    __metaclass__ = PrintMetaClass

    def __init__(self):
        self.pid = None
        self.name = None
        self.times = []
        self.digitals = []
        self.analogA = []
        self.analogB = []

        self.DSPRunner = Runner()
        # self.board = RedPitaya()

        # single value output
        # self.board.asga.counter_wrap = self.board.asga.counter_step
        # self.board.asgb.counter_wrap = self.board.asgb.counter_step

        # self.board.hk.expansion_connector_direction_P = 0xFF # set all pins to out
        # self.board.hk.expansion_connector_direction_N = 0xFF

        self.clientConnection = None

        self.fakeALines = [0, 0, 0, 0]

    # The dsp has a handler for SIGINT that cleans up
    def Abort(self):
        # kill the server process
        self.DSPRunner.abort()
        # self.board.hk.expansion_connector_output_P = 0
        # self.board.hk.expansion_connector_output_N = 0

    def MoveAbsoluteADU(self, aline, aduPos):
        # probably just use the python lib
        # volts to ADU's for the DSP card: int(pos * 6553.6))
        # Bu we won't be hooked up to the stage?
        # if aline == 0:
        #     self.board.asga.data = [aduPos]
        # if aline == 1:
        #     self.board.asgb.data = [aduPos]
        # else:
        #     print("aline {}>1".format(aline))
        #     self.fakeALines[aline] = aduPos
        print("NOT IMPLEMENTED");

    def arcl(self, cameras, lightTimePairs):
        if lightTimePairs:
            # Expose all lights at the start, then drop them out
            # as their exposure times come to an end.
            # Sort so that the longest exposure time comes last.
            lightTimePairs.sort(key = lambda a: a[1])
            curDigital = cameras + sum([p[0] for p in lightTimePairs])
            self.WriteDigital(curDigital)
            print("Start with", curDigital)
            totalTime = lightTimePairs[-1][1]
            curTime = 0
            for line, runTime in lightTimePairs:
                # Wait until we need to open this shutter.
                waitTime = runTime - curTime
                if waitTime > 0:
                    busy_wait(waitTime/1000.)
                curDigital -= line
                self.WriteDigital(curDigital)
                curTime += waitTime
                print("At",curTime,"set",curDigital)
            # Wait for the final timepoint to close shutters.
            if totalTime - curTime:
                busy_wait( (totalTime - curTime)/1000. )
            print("Finally at",totalTime,"set",0)
            self.WriteDigital(0)
        else:
            self.WriteDigital(cameras) # "expose"
            self.WriteDigital(0)

    # def profileSet(self, profileStr, digitals, *analogs):
    def setProfile(self, times, pins, values):
        print('Setting action table...', end=' ');
        self.actiontable = zip(times, pins, values);
        print('done');


    def DownloadProfile(self): # This is saving the action table
        self.DSPRunner.load(self.actiontable);

    def InitProfile(self, numReps):
        # I'm pretty sure this does not zero the prev values.
        # is it for allocating space?
        #self.times, self.digitals, self.analogA, self.analogB = [], [], [], []
        pass

    def trigCollect(self):
        process = self.DSPRunner.start();
        process.wait();
        # retVal = (100, [self.ReadPosition(0), self.ReadPosition(1), 0, 0])
        # self.clientConnection.receiveData("DSP done", retVal)
        # needs to block on the dsp finishing.

    def ReadPosition(self, axis):
        # if axis == 0:
        #     return int(self.board.asga.data[0])
        # elif axis == 1:
        #     return int(self.board.asgb.data[0])
        # else:
        #     return self.fakeALines[axis]
        print("NOT IMPLEMENTED");

    def WriteDigital(self, level):
        # dP, dN = level & int('11111111', 2), (level & int('1111111100000000', 2)) >> 8
        # self.board.hk.led = level & int('11111111', 2) # 7 led's
        # self.board.hk.expansion_connector_output_P = dP
        # self.board.hk.expansion_connector_output_N = dN
        print("NOT IMPLEMENTED");

    def demo(self, dt):
        print("DeMo");
        self.DSPRunner.loadDemo(dt);
        process = self.DSPRunner.start();
        process.wait();

    def receiveClient(self, uri):
        self.clientConnection = Pyro4.Proxy(uri)
        print(uri)


if __name__ == '__main__':
    dsp = rpServer();
    dspPort = 7000;
    dspHost = '10.42.0.175';

    print("Started program at",time.strftime("%A, %B %d, %I:%M %p"));

    Pyro4.config.SERIALIZER = 'pickle';
    Pyro4.config.SERIALIZERS_ACCEPTED.add('pickle');

    while True:
        try:
            dspDaemon = Pyro4.Daemon(port = dspPort, host = dspHost);
            break;
        except Exception as e:
            print("Socket fail", e);
            time.sleep(1);

    print('Providing dsp.d() as [pyroDSP] at {}'.format(dspDaemon.locationStr));
    Pyro4.Daemon.serveSimple({dsp: 'pyroDSP'},
        daemon = dspDaemon, ns = False, verbose = True);
