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

    def run(self):
        self.reader.run()

    def print(self):
        self.reader.print()

    def valid(self):
        return self.reader.valid()

    def correct(self):
        return self.reader.correct()

    def humidity(self):
        return float(self.reader.humidity(self.reader.bits())) / 10.0

    def temperature(self):
        t = self.reader.temperature(self.reader.bits())
        if t & 0x8000:
            t = (~t + 1) | 0x8000
        return float(t) / 10.0

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

    def defectHigh(self):
        i = self.reader.defectHigh()
        if i >= 0:
            return i

    def defectLow(self):
        i = self.reader.defectLow()
        if i >= 0:
            return i
