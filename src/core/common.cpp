#include <ctime>
#include <sstream>

#include "common.hpp"

int
parseDateTime(std::string dtime, std::string &err) {
    struct tm tm;

    if (dtime.size() == 0) {
        err = "string is empty";
        return -1;
    }

    char *errchar = strptime(dtime.c_str(), "%Y-%m-%d %H:%M", &tm);
    if (*errchar != '\0') {
        std::stringstream s;
        s << "failed at symbol '" << *errchar << "', expected 'YYYY-MM-DD HH:mm' string";
        err = s.str();
        return -1;
    }

    time_t seconds = timegm(&tm);
    if (seconds - SECONDS_AT_96 < 0) {
        err = "date/time can't be less than 00:00 01-01-1996";
        return -1;
    }

    // return minutes
    return (seconds - SECONDS_AT_96) / 60;
}