#pragma once

#include <string>

// seconds at 0:00 hrs 1/1/96 since epoch
#define SECONDS_AT_96 820454400

//@brief Get minutes from 00:00 01-01-1996 from string
//@param dtime std::string date/time in format of 'YYYY-mm-DD HH:MM'
//@param &err output error
//@return minutes from 00:00 01-01-1996 or -1 on error
int
parseDateTime(std::string dtime, std::string &err);