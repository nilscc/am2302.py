from libc.stdint cimport uint8_t, uint16_t, uint32_t
from libcpp cimport bool

cdef extern from 'wiringPi.h':
    int wiringPiSetup()

    int INPUT
    int OUTPUT
    void pinMode(int, int)

    int HIGH
    int LOW
    void digitalWrite(int, int)
    int digitalRead(int)

    void delay(unsigned int)
    void delayMicroseconds(unsigned int)

cdef extern from 'am2302_py.hpp':

    cdef cppclass CReader:
        int m_pin
        uint32_t m_start1, m_start2, m_start3
        uint16_t m_temperature, m_humidity
        uint8_t m_parity
        bool m_tempDone, m_humidityDone, m_parityDone
        int m_awaitLevel
        int m_awaitBit
        uint32_t m_awaitDuration

        bool run()
