#include <wiringPi.h>
#include <cstdint>

class CTimer
{
    private:
        const uint32_t m_t0;

    public:

        CTimer() : m_t0(micros()) {}

        uint32_t duration() const {
            const uint32_t t = micros();
            // check if we wrapped
            if (t < m_t0)
                return 0xFFFFFFFF - m_t0 + t + 1;
            else
                return t - m_t0;
        }
};

template <typename T>
struct CBitLength { static const uint8_t bitLength = 0U; };

template <>
struct CBitLength<uint8_t> { static const uint8_t bitLength = 8U; };

template <>
struct CBitLength<uint16_t> { static const uint8_t bitLength = 16U; };

class CReader
{
    public:
        int m_pin;

        uint32_t m_start1, m_start2, m_start3;

        uint16_t m_temperature, m_humidity;
        uint8_t m_parity;

        bool m_tempDone, m_humidityDone, m_parityDone;

        int m_awaitLevel;
        int m_awaitBit;
        uint32_t m_awaitDuration;

        CReader()
            : m_pin(0)
            , m_start1(0U)
            , m_start2(0U)
            , m_start3(0U)
            , m_temperature(0U)
            , m_humidity(0U)
            , m_parity(0U)
            , m_tempDone(false)
            , m_humidityDone(false)
            , m_parityDone(false)
            , m_awaitLevel(-1)
            , m_awaitBit(-1)
            , m_awaitDuration(0U)
        {}

    private:

        bool await(int level, uint32_t min, uint32_t max, uint32_t *count = nullptr) {
            const CTimer l_timer;
            while (l_timer.duration() < max) {
                if (digitalRead(m_pin) == level) {
                    const auto duration = l_timer.duration();
                    if (count != nullptr)
                        *count = duration;
                    if (duration >= min)
                        return true;
                    else {
                        // store debug info
                        m_awaitLevel = level;
                        m_awaitDuration = duration;
                        return false;
                    }
                }
                delayMicroseconds(1);
            }

            // store debug info
            m_awaitLevel = level;
            m_awaitDuration = l_timer.duration();
            return false;
        }

        void low() { digitalWrite(m_pin, LOW); }
        void high() { digitalWrite(m_pin, HIGH); }
        void input() { pinMode(m_pin, INPUT); }
        void output() { pinMode(m_pin, OUTPUT); }

        template <typename T>
        bool receive(T *f_val) {
            for (int i = 0; i < CBitLength<T>::bitLength; ++i) {
                if (!await(HIGH, 0U, 100U)) {
                    m_awaitBit = i;
                    return false;
                }

                uint32_t l_count;
                if (!await(LOW, 0U, 100U, &l_count))
                    return false;

                if (l_count > 50)
                    *f_val |= (1U << (CBitLength<T>::bitLength - 1 - i));
            }
            return true;
        }

        bool start()
        {
            output();
            low();
            delay(3);
            input();
            return await(LOW, 0U, 100U, &m_start1)
                && await(HIGH, 50U, 120U, &m_start2)
                && await(LOW, 50U, 120U, &m_start3);
        }

        bool receiveTemp() {
            const bool l_res = receive(&m_temperature);
            m_tempDone = true;
            return l_res;
        }
        bool receiveHumidity() {
            const bool l_res = receive(&m_humidity);
            m_humidityDone = true;
            return l_res;
        }
        bool receiveParity() {
            const bool l_res = receive(&m_parity);
            m_parityDone = true;
            return l_res;
        }

    public:

        bool run() {
            return start()
                && receiveHumidity()
                && receiveTemp()
                && receiveParity();
        }
};
