#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#define UNUSED(x) (void)(x)

// 7:4 - reserved, written as 0000b
// 0:3 - format version number = 1h
#define DEFAULT_SECTION_HEADER_BYTE std::byte{0x01}

#define END_OF_FIELDS_BYTE std::byte{0xC1}

#define LANG_CODE_CAP 136 // Zulu!

typedef unsigned char            uchar;
typedef std::vector<std::byte>   bytes;
typedef bytes::iterator          biterator;
typedef std::vector<std::string> svector;

enum ValType {
    VAL_TYPE_NONE,
    VAL_TYPE_INT,
    VAL_TYPE_TEXT,
};

enum Encoding {
    ENCODING_BINARY_UNSPEC = 0, // 00b
    ENCODING_BCDp          = 1, // 01b
    ENCODING_ASCII_6b      = 2, // 10b
    ENCODING_UNOCODE       = 3, // 11b
};

//||ascii6bit|langcode
extern std::unordered_map<std::string, Encoding> encoding_map;

std::string
encodingToString(Encoding enc);
