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

cdef class Reader:
    cdef am2302.CReader m_reader

    def __init__(self, pin=7):
        self.m_reader.m_pin = pin

    def run(self):
        return self.m_reader.run()

    # Main output properties

    @property
    def temperature(self):
        t = self.m_reader.m_temperature
        if t & 0x8000:
            t = (~t + 1) | 0x8000
        return float(t) / 10.0

    @property
    def humidity(self):
        return float(self.m_reader.m_humidity) / 10.0

    @property
    def parity(self):
        return self.m_reader.m_parity

    @property
    def valid(self):
        h = self.m_reader.m_humidity
        t = self.m_reader.m_temperature
        p = self.m_reader.m_parity
        return (((h >> 8) + h + (t >> 8) + t) & 0xFF) == p

    # Debug properties

    @property
    def start1(self):
        return self.m_reader.m_start1
    @property
    def start2(self):
        return self.m_reader.m_start2
    @property
    def start3(self):
        return self.m_reader.m_start3

    @property
    def tempDone(self):
        return self.m_reader.m_tempDone
    @property
    def humidityDone(self):
        return self.m_reader.m_humidityDone
    @property
    def parityDone(self):
        return self.m_reader.m_parityDone

    @property
    def awaitLevel(self):
        return self.m_reader.m_awaitLevel

    @property
    def awaitDuration(self):
        return self.m_reader.m_awaitDuration

    @property
    def awaitBit(self):
        return self.m_reader.m_awaitBit

class am3202_data:
    def __init__(self, temperature, humidity):
        self.temperature = temperature
        self.humidity = humidity

def read(pin=7, retries=10):
    setup()
    reader = Reader(pin)
    for r in range(retries):
        if reader.run() and reader.valid:
            return am3202_data(
                temperature=reader.temperature, 
                humidity=reader.humidity,
            )

        # wait before checking sensor again
        time.sleep(1)
