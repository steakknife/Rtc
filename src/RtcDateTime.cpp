
#include <Arduino.h>
#include "RtcDateTime.h"

static const uint8_t c_daysInMonth[] PROGMEM = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

RtcDateTime::RtcDateTime(uint32_t secondsFrom2000)
{
    _initWithSecondsFrom2000<uint32_t>(secondsFrom2000);
}

// is x in [a,b]?
#define BETWEEN_INCL(x, a, b) ((x) >= (a) && (x) <= (b))

#define IS_LEAP_YEAR(y) ((!((y) % 4) && ((y) % 100)) || !((y) % 400))

#define IS_30_DAY_MONTH(m) ((((m) - 1) % 7) % 2)

// from https://raw.githubusercontent.com/eggert/tz/master/leap-seconds.list

// at yyyy-06-30 23:59:60
static const uint16_t _summerLeapSecondTable[] = {
    1972,
    1981, 1982, 1983, 1985,
    1992, 1993, 1994, 1997,
    2012, 2015
};

// at yyyy-12-31 23:59:60
static const uint16_t _winterLeapSecondTable[] = {
    1972, 1973, 1974, 1975, 1976, 1977, 1978, 1979,
    1987, 1989,
    1990, 1995, 1998,
    2005, 2008, 2016,
};

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(*(a)))

bool RtcDateTime::_IsValidLeapSecond() const
{
    if (_hour == 23 && _minute == 59)
        if (_month == 6 && _day == 30)
            for (size_t = 0; i < ARRAY_SIZE(_summerLeapSecondTable); ++i)
                if (_summerSecondTable[i] == _year) return true;
        else if (_month == 12 && _day == 31)
            for (size_t = 0; i < ARRAY_SIZEl(_winterLeapSecondTable); ++i)
                if (_winterLeapSecondTable[i] == _year) return true;
    return false;
}

bool RtcDateTime::IsValid() const
{
    // this just tests the most basic validity of the value ranges
    // and valid leap years
    // It does not check any time zone or daylight savings time
    if (!BETWEEN_INCL(_month, 1, 12)) return false;
    if (!BETWEEN_INCL(_dayOfMonth, 1, 31)) return false;
    if (_hour > 23) return false;
    if (_minute > 59) return false;

    // check dayOfMonth precisely
    if (_month == 2) {
        if (_dayOfMonth > 29) return false;
        if (_dayOfMonth == 29 && !IS_LEAP_YEAR(_year)) return false;
    } else if (_dayOfMonth == 31 && IS_30_DAY_MONTH(_month)) return false;

    // check second precisely (leap second condition)
    if (_second > 59 || (_second == 60 && !_IsValidLeapSecond())) return false;

    return true;
}

uint8_t StringToUint8(const char* pString)
{
    uint8_t value = 0;

    // skip leading 0 and spaces
    while (*pString == '0' || *pString == ' ') ++pString;

    // calculate number until we hit non-numeral char
    while (*pString >= '0' && *pString <= '9') {
        value *= 10;
        value += *pString - '0';
        ++pString;
    }
    return value;
}

RtcDateTime::RtcDateTime(const char* date, const char* time)
{
    // sample input: date = "Dec 06 2009", time = "12:34:56"
    _yearFrom2000 = StringToUint8(date + 9);
    // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
    switch (date[0]) {
    case 'J':
        if (date[1] == 'a')
            _month = 1;
        else if (date[2] == 'n')
            _month = 6;
        else
            _month = 7;
        break;
    case 'F':
        _month = 2;
        break;
    case 'A':
        _month = date[1] == 'p' ? 4 : 8;
        break;
    case 'M':
        _month = date[2] == 'r' ? 3 : 5;
        break;
    case 'S':
        _month = 9;
        break;
    case 'O':
        _month = 10;
        break;
    case 'N':
        _month = 11;
        break;
    case 'D':
        _month = 12;
        break;
    }
    _dayOfMonth = StringToUint8(date + 4);
    _hour = StringToUint8(time);
    _minute = StringToUint8(time + 3);
    _second = StringToUint8(time + 6);
}

template<typename T> T DaysSinceFirstOfYear2000(uint16_t year, uint8_t month, uint8_t dayOfMonth)
{
    T days = dayOfMonth;
    for (uint8_t indexMonth = 1; indexMonth < month; ++indexMonth)
        days += pgm_read_byte(c_daysInMonth + indexMonth - 1);
    if (month > 2 && year % 4 == 0) ++days;
    return days + 365 * year + (year + 3) / 4 - 1;
}

template<typename T> T SecondsIn(T days, uint8_t hours, uint8_t minutes, uint8_t seconds)
{
    return ((days * 24L + hours) * 60 + minutes) * 60 + seconds;
}

uint8_t RtcDateTime::DayOfWeek() const
{
    uint16_t days = DaysSinceFirstOfYear2000<uint16_t>(_yearFrom2000, _month, _dayOfMonth);
    return (days + 6) % 7; // Jan 1, 2000 is a Saturday, i.e. returns 6
}

// 32-bit time; as seconds since 1/1/2000
uint32_t RtcDateTime::TotalSeconds() const
{
    uint16_t days = DaysSinceFirstOfYear2000<uint16_t>(_yearFrom2000, _month, _dayOfMonth);
    return SecondsIn<uint32_t>(days, _hour, _minute, _second);
}

// 64-bit time; as seconds since 1/1/2000
uint64_t RtcDateTime::TotalSeconds64() const
{
    uint32_t days = DaysSinceFirstOfYear2000<uint32_t>(_yearFrom2000, _month, _dayOfMonth);
    return SecondsIn<uint64_t>(days, _hour, _minute, _second);
}

// total days since 1/1/2000
uint16_t RtcDateTime::TotalDays() const
{
    return DaysSinceFirstOfYear2000<uint16_t>(_yearFrom2000, _month, _dayOfMonth);
}

void RtcDateTime::InitWithIso8601(const char* date)
{
    // sample input: date = "Sat, 06 Dec 2009 12:34:56 GMT"
    _yearFrom2000 = StringToUint8(date + 13);
    // Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
    switch (date[8]) {
    case 'J':
        if (date[1 + 8] == 'a')
            _month = 1;
        else if (date[2 + 8] == 'n')
            _month = 6;
        else
            _month = 7;
        break;
    case 'F':
        _month = 2;
        break;
    case 'A':
        _month = date[1 + 8] == 'p' ? 4 : 8;
        break;
    case 'M':
        _month = date[2 + 8] == 'r' ? 3 : 5;
        break;
    case 'S':
        _month = 9;
        break;
    case 'O':
        _month = 10;
        break;
    case 'N':
        _month = 11;
        break;
    case 'D':
        _month = 12;
        break;
    }
    _dayOfMonth = StringToUint8(date + 5);
    _hour = StringToUint8(date + 17);
    _minute = StringToUint8(date + 20);
    _second = StringToUint8(date + 23);
}
