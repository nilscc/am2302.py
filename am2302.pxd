from libc.stdint cimport uint8_t, uint16_t, uint32_t
from libcpp cimport bool

cdef extern from 'array':
    cdef cppclass array:
        uint32_t& operator[](uint32_t)

cdef extern from 'bitset':
    cdef cppclass bitset:
        unsigned size()
        unsigned count()
        bool test(unsigned)

cdef extern from 'wiringPi.h':
    void wiringPiSetup()

cdef extern from 'am2302_py.hpp':

    cdef cppclass StreamReader:
        int pin
        bitset bits

        void run()
        void fillBits()

        void print()
        bool valid()

        uint16_t temperature(const bitset &)
        uint16_t humidity(const bitset &)
        uint8_t parity(const bitset &)

        array timingsStart
        array timingsHigh
        array timingsLow

        bitset missingBits()
        bool tryCorrect()
