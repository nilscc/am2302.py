#include <wiringPi.h>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <array>
#include <bitset>

struct ScopedTimer
{
    const uint32_t t0;

    ScopedTimer() : t0(micros()) {}

    uint32_t duration() const {
        const uint32_t t = micros();

        // check if we wrapped
        if (t < t0)
            return 0xFFFFFFFF - t0 + t + 1;
        else
            return t - t0;
    }
};

namespace {

}


struct StreamReader
{
    int pin;

    std::array<uint32_t, 3>
        timingsStart;

    std::array<uint32_t, 40>
        timingsHigh,
        timingsLow;

    std::bitset<40>
        bits;

    StreamReader()
        : pin(0)
        , timingsStart()
        , timingsHigh()
        , timingsLow()
    {
        timingsStart.fill(0U);
        timingsHigh.fill(0U);
        timingsLow.fill(0U);
    }

    void await(const int level, uint32_t *count) const {
        ScopedTimer timer;

        uint32_t c = 0U;
        while (digitalRead(pin) != level && c++ < 250)
            delayMicroseconds(1);

        *count = timer.duration();
    }

    void sendStart() {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
        delay(2);
        pinMode(pin, INPUT);
        await(LOW,  &timingsStart[0]);
        await(HIGH, &timingsStart[1]);
        await(LOW,  &timingsStart[2]);
    }

    void receive() {
        for (unsigned i = 0U; i < timingsHigh.size(); ++i) {
            await(HIGH, &timingsHigh[i]);
            await(LOW, &timingsLow[i]);
        }
    }

    void fillBits() {
        for (int i = 0; i < 40; ++i)
            bits[39 - i] = timingsLow[i] > 50;
    }

    void run() {
        sendStart();
        receive();
        fillBits();
    }

    bool tryCorrect() {
        const auto m = missingBits();

        // do not attempt to fix an unreasonable number of missing bits
        if (m.count() > 5)
            return false;

        //if (tryCorrectIt(m, bits, 0))

        return false;
    }

    bool tryCorrectIt(
            const std::bitset<40> &missing,
            std::bitset<40> bits,
            const unsigned i) {
        using namespace std;

        // check if we reached the end of the bitset
        if (i >= missing.size())
            return validParity(bits);

        // check if current bit is missing at all
        if (!missing.test(i))
            return tryCorrectIt(missing, bits, i+1);
        else {

            // create bitset mask for shifting
            bitset<40> m;
            m.set();
            m >>= (40-i);

            // mask upper part of bits and shift to left
            const auto upper = (bits & ~m) << 1;

            // mask lower parts
            const auto lower = (bits & m);

            // update bits
            bits = upper | lower;

            // shift also missing by 2
            if (tryCorrectIt(missing << 1, bits, i+2))
                return true;

            // flip current bit
            bits.flip(i);
            if (tryCorrectIt(missing << 1, bits, i+2))
                return true;

            // flip next bit
            bits.flip(i+1);
            if (tryCorrectIt(missing << 1, bits, i+2))
                return true;

            // flip current bit back and try one last time
            bits.flip(i);
            if (tryCorrectIt(missing << 1, bits, i+2))
                return true;

            // we tried everything :(
            return false;
        }
    }

    /*
     * Results
     *
     */

    static
    uint16_t humidity(const std::bitset<40> bits) {
        return (bits.to_ullong() >> 24) & 0xFFFF;
    }

    static
    uint16_t temperature(const std::bitset<40> bits) {
        return (bits.to_ullong() >> 8) & 0xFFFF;
    }

    static
    uint8_t parity(const std::bitset<40> bits) {
        return bits.to_ullong() & 0xFF;
    }

    /*
     * Data validation
     *
     */

    template <typename T>
    static
    bool inRange(T val, T min, T max) {
        return min <= val && val <= max;
    }

    bool validStart() const {
        return inRange<uint32_t>(timingsStart[0], 0, 50)
            && inRange<uint32_t>(timingsStart[1], 50, 100)
            && inRange<uint32_t>(timingsStart[2], 50, 100);
    }

    static
    bool checkParity(const uint16_t h, const uint16_t t, const uint8_t p) {
        return p == (((h >> 8) + h + (t >> 8) + t) & 0xFF);
    }

    static
    bool validParity(std::bitset<40> bits) {
        const uint16_t h = humidity(bits);
        const uint16_t t = temperature(bits);
        const uint8_t p = parity(bits);

        return checkParity(h,t,p);
    }

    bool valid() const {
        return validStart()
            && validParity(bits);
    }


    /*
     * Error detection
     *
     */

    // Return bitmask with missing HIGH bits
    static
    std::bitset<40> _missingBits(const std::array<uint32_t, 40> &array) {
        std::bitset<40> mask;

        for (unsigned i = 0U; i < array.size(); ++i)
            mask[i] = inRange(array[i], 100U, 220U);

        return mask;
    }

    std::bitset<40> missingBits() const { return _missingBits(timingsHigh) | _missingBits(timingsLow); }

    



    /*
     * Debug output
     *
     */

    template <typename T>
    static
    std::string formatArray(const T &f_array) {
        std::stringstream ss;
        ss << "[";
        for (unsigned int i = 0; i < f_array.size(); ++i) {
            ss << f_array[i];
            if (i < f_array.size() - 1)
                ss << ", ";
        }
        ss << "]";
        return ss.str();
    }

    void print() const {
        using namespace std;

        const auto b = bits;

        const uint16_t h = humidity(b);
        const uint16_t t = temperature(b);
        const uint8_t p = parity(b);

        const bool parityOk = checkParity(h,t,p);

        cout
            << "Timings:" << endl
            << "\tS " << formatArray(timingsStart) << endl
            << "\tH " << formatArray(timingsHigh) << endl
            << "\tL " << formatArray(timingsLow) << endl
            << endl
            << "Bits:" << endl
            << "\t" << bitset<40>(b) << endl
            << endl
            << "Values:" << endl
            << "\tH " << h << " " << bitset<16>(h) << endl
            << "\tT " << t << " " << bitset<16>(t) << endl
            << "\tP     " << bitset<8>(p) << " (" << boolalpha << parityOk << ")" << endl
            << endl
            ;
    }
};
