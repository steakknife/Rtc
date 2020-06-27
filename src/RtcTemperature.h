#pragma once

class RtcTemperature
{
public:
    // Constructor
    // a) Merge RTC registers into signed scaled temperature (x256),
    //    then bind to RTC resolution.
    //         |         r11h          | DP |         r12h         |
    // Bit:     15 14 13 12 11 10  9  8   .  7  6  5  4  3  2  1  0  -1 -2
    //           s  i  i  i  i  i  i  i   .  f  f  0  0  0  0  0  0
    //
    // b) Rescale to (x4) by right-shifting (6) bits
    //         |                                         | DP |    |
    // Bit:     15 14 13 12 11 10  9  8  7  6  5  4  3  2   .  1  0  -1 -2
    //           s  s  s  s  s  s  s  i  i  i  i  i  i  i      f  f   0  0
    RtcTemperature(int8_t highByteDegreesC, uint8_t lowByteDegreesC) :
        #define HIGH_LOW_TO_DEG_C(hi, lo) (((((int16_t)hi) << 8 | (((int16_t)lo) & 0xc0) >> 6)) * 100 / 4)
        _centiDegC(HIGH_LOW_TO_DEG_C(highByteDegreesC, lowByteDegressC))
        #undef HIGH_LOW_TO_DEG_C
    {
    }

    RtcTemperature(int16_t centiDegC = 0) :
        _centiDegC(centiDegC)
    {
    }

    // Float temperature Celsius
    float AsFloatDegC() const
    {
        return (float)_centiDegC / 100.0f;
    }

    // Float temperature Fahrenheit
    float AsFloatDegF() const
    {
        return AsFloatDegC() * 1.8f + 32.0f;
    }

    // centi degrees (1/100th of a degree), 
    int16_t AsCentiDegC() const
    {
        return _centiDegC;
    }

    void Print(Stream& target, uint8_t decimals = 2, char decimal = '.') const
    {
        int16_t decimalDivisor = 1;
        int16_t integerPart;
        int16_t decimalPart;

        {
            int16_t rounded = abs(_centiDegC);
            // round up as needed
            if (decimals == 0)
                rounded += 50;
            else if (decimals == 1) {
                rounded += 5;
                decimalDivisor = 10;
            }

            integerPart = rounded / 100;
            decimalPart = (rounded % 100) / decimalDivisor;
        }

        // test for zero before printing negative sign to not print-0.00
        if (_centiDegC < 0 && (integerPart || decimalPart))
            target.print('-');

        // print integer part
        target.print(integerPart);

        // print decimal part
        if (decimals) {
            target.print(decimal);

            if (decimalPart)
                target.print(decimalPart);
            else
            {
                // append zeros as requested
                while (decimals--) target.print('0');
            }
        }
    }

    bool operator==(const RtcTemperature& other) const
    {
        return _centiDegC == other._centiDegC;
    }

    bool operator>(const RtcTemperature& other) const
    {
        return _centiDegC > other._centiDegC;
    }

    bool operator<(const RtcTemperature& other) const
    {
        return _centiDegC < other._centiDegC;
    }

    bool operator>=(const RtcTemperature& other) const
    {
        return _centiDegC >= other._centiDegC;
    }

    bool operator<=(const RtcTemperature& other) const
    {
        return _centiDegC <= other._centiDegC;
    }

    bool operator!=(const RtcTemperature& other) const
    {
        return _centiDegC != other._centiDegC;
    }

    RtcTemperature operator-(const RtcTemperature& right)
    {
        RtcTemperature result(_centiDegC - right._centiDegC);
        return result;
    }

    RtcTemperature operator+(const RtcTemperature& right)
    {
        RtcTemperature result(_centiDegC + right._centiDegC);
        return result;
    }

protected:
    int16_t  _centiDegC;  // 1/100th of a degree temperature (100 x degC)
};
