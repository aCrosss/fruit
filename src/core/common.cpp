#include <ctime>
#include <sstream>

#include "common.hpp"

long long
parseDateTime(std::string dtime, std::string &err) {
#if _WIN32
    struct tm tm = {0, 0, 0, 0, 0, 0, 0, 0, 0};
#else
    struct tm tm = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
#endif

    if (dtime.size() == 0) {
        err = "string is empty";
        return -1;
    }

#if _WIN32
    char drop;
    int  year, month, day, hour, minute;

    std::istringstream iss(dtime);
    if (iss >> year >> drop >> month >> drop >> day >> drop >> hour >> drop >> minute) {
        if (month < 1 || month > 12 || day < 1 || day > 31 || hour < 0 || hour > 23 ||
            minute < 0 || minute > 59) {
            err = "invalid date/time values";
            return -1;
        }
    } else {
        err = "expected 'YYYY-MM-DD HH:mm' string";
        return -1;
    }

    tm.tm_year  = year - 1900;
    tm.tm_mon   = month - 1;
    tm.tm_mday  = day;
    tm.tm_hour  = hour;
    tm.tm_min   = minute;
    tm.tm_sec   = 0;
    tm.tm_isdst = 0;
#else
    char *errchar = strptime(dtime.c_str(), "%Y-%m-%d %H:%M", &tm);
    if (!errchar) {
        err = "expected 'YYYY-MM-DD HH:mm' string";
        return -1;
    }

    if (*errchar != '\0') {
        std::stringstream s;
        s << "failed at symbol '" << *errchar << "', expected 'YYYY-MM-DD HH:mm' string";
        err = s.str();
        return -1;
    }
#endif

    time_t seconds;
#ifdef _WIN32
    seconds = _mkgmtime(&tm);
#else
    seconds = timegm(&tm);
#endif

    if (seconds - SECONDS_AT_96 < 0) {
        err = "date/time can't be less than 00:00 01-01-1996";
        return -1;
    }

    // return minutes
    return (seconds - SECONDS_AT_96) / 60;
}
