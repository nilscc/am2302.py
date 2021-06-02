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
    template <typename T>
        bool inRange(T val, T min, T max) {
            return min <= val && val <= max;
        }

    template <typename T>
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
}


struct StreamReader
{
    int pin;

    std::array<uint32_t, 3>
        timingsStart;

    std::array<uint32_t, 40>
        timingsHigh,
        timingsLow;

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
        while (digitalRead(pin) != level && c++ < 1000)
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

    void run() {
        sendStart();
        receive();
    }

    std::bitset<40> bits() const {
        std::bitset<40> b;
        for (int i = 0; i < 40; ++i)
            b[39 - i] = timingsLow[i] > 50;
        return b;
    }

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

    static
    bool checkParity(const uint16_t h, const uint16_t t, const uint8_t p) {
        return p == (((h >> 8) + h + (t >> 8) + t) & 0xFF);
    }

    bool validStart() const {
        return inRange<uint32_t>(timingsStart[0], 0, 50)
            && inRange<uint32_t>(timingsStart[1], 50, 100)
            && inRange<uint32_t>(timingsStart[2], 50, 100);
    }

    bool validParity() const {
        const auto b = bits();

        const uint16_t h = humidity(b);
        const uint16_t t = temperature(b);
        const uint8_t p = parity(b);

        return checkParity(h,t,p);
    }

    bool valid() const {
        return validStart()
            && validParity();
    }

    int defectHigh() const {
        for (unsigned i = 0U; i < timingsHigh.size(); ++i) {
            if (!inRange<uint32_t>(timingsHigh[i], 50, 70))
                return i;
        }
        return -1;
    }

    static
    bool validLowValue(const uint32_t l) {
        return inRange<uint32_t>(l, 25, 30)
            || inRange<uint32_t>(l, 70, 75);
    }

    int defectLow() const {
        for (unsigned i = 0U; i < timingsLow.size(); ++i) {
            if (!validLowValue(timingsLow[i]))
                return i;
        }
        return -1;
    }

    /*
     * Error correction
     *
     */

    /// Correct a single defect. Returns true if correction was successful or
    /// false if correction was not possible.
    bool correct() {

        // check defect locations
        const int h = defectHigh();
        const int l = defectLow();

        if (h == -1 && l == -1)
            return true; // nothing to do

        // check if we have a late transition from low to high
        if (inRange(l, 0, 38) && l+1 == h)
            return fixLowToHigh(l);

        return false;
    }

    bool fixLowToHigh(const int l) {
        const uint32_t
            tlcur  = timingsLow[l],
            thnext = timingsHigh[l+1],
            tlnext = timingsLow[l+1];

        if (thnext < 50) {

            // calculate offset of current low value to either 73 or 26
            const uint32_t
                tlcur_toomuch73 = tlcur - 73,
                tlcur_toomuch26 = tlcur - 26;

            // calculate high value
            const uint32_t
                thnext_toomuch73 = thnext + tlcur_toomuch73 - 54,
                thnext_toomuch26 = thnext + tlcur_toomuch26 - 54;

            // check if the 'too much' of the new high value results in a valid
            // next low value
            if (validLowValue(tlnext + thnext_toomuch73)) {
                timingsLow[l] = 73;
                timingsHigh[l+1] = 54;
                timingsLow[l+1] = tlnext + thnext_toomuch73;
                return true;
            }
            else if (validLowValue(tlnext + thnext_toomuch26)) {
                timingsLow[l] = 26;
                timingsHigh[l+1] = 54;
                timingsLow[l+1] = tlnext + thnext_toomuch26;
                return true;
            }
        }

        return false;
    }






    void print() const {
        using namespace std;

        const auto b = bits();

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
