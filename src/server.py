#! /usr/bin/python
from __future__ import print_function

import subprocess
import threading
import os
import time
import struct

import logging
import traceback
import sys
# ... set level of other logger names as desired ...

from PyRedPitaya.board import RedPitaya
import Pyro4


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
    '''Shows the traceback for a pyro function call in the server log.
    also logs function calls.
    '''
    @wraps(func)
    def wrapped(*args, **kwrds):
        logging.debug(func)
        try:
            return func(*args, **kwrds)
        except Exception as e:
            logging.debug(str(e))
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

# typedef struct actionTable {
# 	unsigned long long clocks;
# 	int pinP;
# 	int pinN;
# 	uint32_t a1;
# 	uint32_t a2;
# } actionTable_t;

# the action table stores clock cycles,
# we want nanoseconds -> conversion is done in dsp code

class _dspPrinter(threading.Thread):

    def __init__(self, buffer, logger):
        super(_dspPrinter, self).__init__()
        self.daemon=True
        self.buf = buffer
        self.running = True
        self.logger = logging.getLogger('DSP')

    def run(self):
        '''Outputs the data from the buffer to the logging output.
        '''
        while self.running:
            msg = []
            # read a line.
            while len(msg) == 0 or msg[-1] != '\n':
                msg.append(os.read(self.buf, 1)) # should block on empty pipe?

            self.logger.info(''.join(msg).strip())
            time.sleep(0.1) # don't spinloop


class Runner(object):
    '''Class responsible for communicating with the DSP process.
    '''

    __metaclass__ = PrintMetaClass # logs all the function calls to this class

    def __init__(self):
        self.process = None # currently running process
        self.running = None
        self.actionTableRows = []
        self.structFmt = 'QiiII'

        # set up logging for the DSP process
        self.outBufread, self.outBufWrite = os.pipe()
        printer = _dspPrinter(self.outBufread, logging.getLogger())
        printer.start()

    # actiontable format is time, dP, dN, a1, a2
    # where the time is the delta-t since the start in ns
    # dP, dN are bitmasks to write to the pins
    # a1 and a2 are values to write to the analog pins

    def loadDemo(self, dt):
        self.actionTableRows = [] # clear the buffer when doing a new expr.
        n = int(1*1e9/dt) # want a second of output
        t = 0
        for l in range(n):
            self.actionTableRows.append(struct.pack(self.structFmt, t, 0, 0, 0, l*4000./n))
            t += dt
            self.actionTableRows.append(struct.pack(self.structFmt, t, 0x000000FF, 0x000000FF, 4000, l*4000./n))
            t += dt

    def load(self, actiontable):
        self.actionTableRows = [] # clear the buffer when doing a new expr.

        for row in actiontable:
            time, digitals, a1, a2 = row
            time = int(time*1e3) # convert to ns
            dP, dN = digitals & int('11111111', 2), (digitals & int('1111111100000000', 2)) >> 8
            finalrow = time, dP, dN, a1, a2
            logging.debug('time:{} digitalP:{} digitalN:{} a1:{} a2:{}'.format(*finalrow))
            self.actionTableRows.append(struct.pack(self.structFmt, *finalrow))

    def abort(self):
        """Stops the DSP ASAP.
        """
        # check for none and liveness
        if self.process and self.process.poll():
            subprocess.send_signal(subprocess.signal.SIGINT)

    def stop(self):
        self.abort()

    def start(self):
        if self.actionTableRows:
            actionTableBytes = ''.join(self.actionTableRows)
            cmd = ['/opt/bin/dsp', '-', str(len(self.actionTableRows))]
            logging.info('calling ' + str(cmd))
            self.process = subprocess.Popen(cmd,
                                            stdin=subprocess.PIPE,
                                            stdout=self.outBufWrite)
            logging.debug('transferring {} bytes'.format(len(actionTableBytes)))
            self.process.stdin.write(actionTableBytes)
        else:
            raise Exception("please write the action table!")



class rpServer(object):
    '''Singleton class to be served by Pyro. Provides the API for cockpit to
    talk to us.
    '''

    # Prints all func. calls to stdout
    __metaclass__ = PrintMetaClass

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

        self.board.hk.expansion_connector_direction_P = 0xFF # set all pins to out
        self.board.hk.expansion_connector_direction_N = 0xFF

        self.clientConnection = None

        self.fakeALines = [0, 0, 0, 0]

    # The dsp has a handler for SIGINT that cleans up
    def Abort(self):
        '''Kills running server processes.'''
        # kill the server process
        self.DSPRunner.abort()
        self.board.hk.expansion_connector_output_P = 0
        self.board.hk.expansion_connector_output_N = 0

    def MoveAbsoluteADU(self, aline, aduPos):
        '''Writes a value to the analog outs.

        aline: 0-based line index. Currently only have lines 1&2
        aduPos: value to write to the line.
            volts to ADU's for the DSP card: int(pos * 6553.6))
        '''
        if aline == 0:
            self.board.asga.data = [aduPos]
        if aline == 1:
            self.board.asgb.data = [aduPos]
        else:
            logging.debug("aline {}>1".format(aline))
            self.fakeALines[aline] = aduPos

    def arcl(self, cameras, lightTimePairs):
        '''Exposes the given cameras for the given lengths of time.
        '''
        if lightTimePairs:
            # Expose all lights at the start, then drop them out
            # as their exposure times come to an end.
            # Sort so that the longest exposure time comes last.
            lightTimePairs.sort(key = lambda a: a[1])
            curDigital = cameras + sum([p[0] for p in lightTimePairs])
            self.WriteDigital(curDigital)
            logging.debug("Start with " + curDigital)
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
                logging.debug("At " + str(curTime) + "set " + str(curDigital))
            # Wait for the final timepoint to close shutters.
            if totalTime - curTime:
                busy_wait( (totalTime - curTime)/1000. )
            self.WriteDigital(0)
            logging.debug("Finally at " + totalTime + "set " + 0)
        else:
            self.WriteDigital(cameras) # "expose"
            self.WriteDigital(0)


    def profileSet(self, profileStr, digitals, *analogs):
        logging.debug("profileset called with")
        logging.debug("analog0 " + analogs[0] + "times " + str(zip(*analogs[0])[0]) + "vals " + str(zip(*analogs[0])[1]))
        logging.debug('digitals as well')
        logging.debug(digitals)
        # This is downloading the action table
        # digitals is numpy.zeros((len(times), 2), dtype = numpy.uint32),
        # starting at 0 -> [times for digital signal changes, digital lines]
        # analogs is a list of analog lines and the values to put on them at each time
        # digitals = list of lists. sublist is a time, line pair
        # then 4 analog lines. also list of time: value pairs.
        Dtimes, Dvals = zip(*digitals)
        if len(analogs) > 0:
            Atimes, Avals = zip(*analogs[0])
        else:
            Atimes, Avals = [], []
        if len(analogs) > 0:
            Btimes, Bvals = zip(*analogs[1])
        else:
            Btimes, Bvals = [], []

        Dtimes = list(Dtimes)
        Atimes = list(Atimes)
        Btimes = list(Btimes)

        logging.debug("dt:{},\n at:{},\n bt:{}".format(Dtimes, Atimes, Btimes))
        logging.debug("dv:{},\n av:{},\n bv:{}".format(Dvals, Avals, Bvals))

        times, digitals, analogA, analogB = [], [], [], []
        times = sorted(list(set(Dtimes+Atimes+Btimes)))
        for timepoint in times:
            self.times.append(timepoint)
            for outline, inval, timesForLine in [(digitals, Dvals, Dtimes),
                                                 (analogA,  Avals, Atimes),
                                                 (analogB,  Bvals, Btimes)]:
                if timepoint in timesForLine:
                    outline.append(inval[timesForLine.index(timepoint)])
                else:
                    prevValue = outline[-1] if outline else 0
                    outline.append(prevValue) # the last value

        # times are in provided in units of 50nanos each to setprofile?
        # determined by taking 100ms exposures and adjusting until correct
        self.actiontable = zip([t*100 for t in times], digitals, analogA, analogB)
        logging.debug("sorting action table")
        self.actiontable.sort()

    def DownloadProfile(self): # This is saving the action table
        self.DSPRunner.load(self.actiontable)

    def InitProfile(self, numReps):
        # I'm pretty sure this does not zero the prev values.
        # is it for allocating space?
        #self.times, self.digitals, self.analogA, self.analogB = [], [], [], []
        pass

    def trigCollect(self):
        self.DSPRunner.start()
        retVal = (100, [self.ReadPosition(0), self.ReadPosition(1), 0, 0])
        self.clientConnection.receiveData("DSP done", retVal)
        # needs to block on the dsp finishing.

    def ReadPosition(self, axis):
        if axis == 0:
            return int(self.board.asga.data[0])
        elif axis == 1:
            return int(self.board.asgb.data[0])
        else:
            return self.fakeALines[axis]

    def WriteDigital(self, level):
        dP, dN = level & int('11111111', 2), (level & int('1111111100000000', 2)) >> 8
        self.board.hk.led = level & int('11111111', 2) # 7 led's
        self.board.hk.expansion_connector_output_P = dP
        self.board.hk.expansion_connector_output_N = dN

    def demo(self, dt):
        self.DSPRunner.loadDemo(dt)
        self.DSPRunner.start()

    def receiveClient(self, uri):
        self.clientConnection = Pyro4.Proxy(uri)
        print(uri)


if __name__ == '__main__':
    logs = [logging.getLogger(), logging.getLogger("Pyro4")]

    formatter = logging.Formatter('%(asctime)s - %(name)s - %(levelname)s - %(message)s')

    stdout_handler = logging.StreamHandler(sys.stdout)
    stdout_handler.setLevel(logging.ERROR)
    stdout_handler.setFormatter(formatter)

    # using watched so that we could use a log rotator
    # but that has to be seperate
    file_handler = logging.FileHandler('/server.log')
    file_handler.setLevel(logging.DEBUG)
    file_handler.setFormatter(formatter)

    for log in logs:
        log.setLevel(logging.DEBUG)
        log.addHandler(file_handler)
        log.addHandler(stdout_handler)


    # logging.debug('debug message')
    # logging.info('info message')
    # logging.warn('warn message')
    # logging.error('error message')
    # logging.critical('critical message')

    # plogs = [logging.getLogger(lstr) for lstr in ("Pyro4","Pyro4.core")]
    #
    # for plog in plogs:
    #     plog.setLevel(logging.DEBUG)
    #     plog.addHandler(file_handler)
    #     plog.addHandler(stdout_handler)

    dsp = rpServer()
    logging.debug(threading.enumerate())
    logging.info("providing dsp.d() as [pyroDSP] on port 7000")
    logging.info("Started program at " + time.strftime("%A, %B %d, %I:%M %p"))

    Pyro4.config.SERIALIZER = 'pickle'
    Pyro4.config.SERIALIZERS_ACCEPTED.add('pickle')

    delay = 1 # reduce retry spam, by exp. backoff
    while True:
        try:
            daemon = Pyro4.Daemon(port = 7000, host = '192.168.1.100')
            break
        except Exception as e:
            delay *= 2
            logging.error('Could not open socket. Retrying in {}s.'.format(delay))
            logging.debug(str(e))
            time.sleep(delay)
    Pyro4.Daemon.serveSimple({dsp: 'pyroDSP'},
            daemon = daemon, ns = False, verbose = True)
