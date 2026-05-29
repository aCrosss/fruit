#pragma once

#include <vector>

typedef std::vector<std::byte> bytes;

enum valType {
    VAL_TYPE_NONE,
    VAL_TYPE_INT,
    VAL_TYPE_STRING,
};

enum Encoding {
    ENCODING_BINARY_UNSPEC = 0, // 00
    ENCODING_BCDp          = 1, // 01
    ENCODING_ASCII_6b      = 2, // 10
    ENCODING_LANG_CODE     = 3, // 11
};
