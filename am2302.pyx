from libc.stdint cimport uint8_t
from libcpp cimport bool

cimport am2302
import time

cdef bool _initialized = False

def setup():
    global _initialized
    if not _initialized:
        wiringPiSetup()
        _initialized = True

def initialized():
    return _initialized

cdef class Stream:
    cdef am2302.StreamReader reader

    def __init__(self, pin):
        self.reader.pin = pin

    # Poll sensor and read data

    def run(self):
        self.reader.run()

    #
    # Debug output
    #

    def print(self):
        self.reader.print()

    #
    # Get results
    #

    @property
    def valid(self):
        return self.reader.valid()

    @property
    def humidity(self):
        return float(self.reader.humidity(self.reader.bits)) / 10.0

    @property
    def temperature(self):
        t = self.reader.temperature(self.reader.bits)
        if t & 0x8000:
            t = (~t + 1) | 0x8000
        return float(t) / 10.0

    #
    # Internal timing arrays
    #
    
    def timingsStart(self, i, val=None):
        if val is None:
            return self.reader.timingsStart[i]
        else:
            self.reader.timingsStart[i] = val

    def timingsHigh(self, i, val=None):
        if val is None:
            return self.reader.timingsHigh[i]
        else:
            self.reader.timingsHigh[i] = val

    def timingsLow(self, i, val=None):
        if val is None:
            return self.reader.timingsLow[i]
        else:
            self.reader.timingsLow[i] = val

    def fillBits(self):
        self.reader.fillBits()

    # 
    # Error detection
    #

    def missingBits(self):
        return self.reader.missingBits().count()

    def tryCorrect(self):
        return self.reader.tryCorrect()

class Result:
    def __init__(self, temperature, humidity):
        self.temperature = temperature
        self.humidity = humidity

def read(pin=7, retries=10):

    # make sure we're initialized
    setup()

    # load stream
    s = Stream(pin=pin)

    m = 1 + retries # number of attempts
    for i in range(m):
        s.run()
        if s.valid:
            return Result(
                temperature=s.temperature,
                humidity=s.humidity,
            )
        # sleep and retry
        if i < m-1:
            time.sleep(2)
