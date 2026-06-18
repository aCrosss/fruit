#pragma once

#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include "types.hpp"

#define ENCODED_MAX_BYTE_LENGTH 31

#define ASCII_6BIT_MAX_IND 0x3F // '_' aka 111111b

#define MASK_1b 0x01
#define MASK_2b 0x03
#define MASK_3b 0x07
#define MASK_4b 0x0F
#define MASK_5b 0x1F
#define MASK_6b 0x3F

// pack charachters c1-c4 into bytes b1-b3
// it supposed to work with charachers from ascii_6b_table!
#define ASCII_6BIT_PACK_CHARS(c1, c2, c3, c4, b1, b2, b3)          \
    b1 = std::byte((c1 & MASK_6b) | ((c2 & MASK_2b) << 6));        \
    b2 = std::byte(((c2 >> 2) & MASK_4b) | ((c3 & MASK_4b) << 4)); \
    b3 = std::byte(((c3 >> 4) & MASK_2b) | ((c4 & MASK_6b) << 2));

#define ASCII_6BIT_UNPACK_CHARS(b1, b2, b3, c1, c2, c3, c4) \
    c1 = b1 & MASK_6b;                                      \
    c2 = (b2 & MASK_4b) << 2 | ((b1 >> 6) & MASK_2b);       \
    c3 = (b3 & MASK_2b) << 4 | ((b2 >> 4) & MASK_4b);       \
    c4 = (b3 >> 2) & MASK_6b;

const std::map<const char, uchar> ascii_6b_table = {
    {' ', 0x00},  {'!', 0x01}, {'"', 0x02}, {'#', 0x03}, {'$', 0x04},  {'%', 0x05}, {'&', 0x06},
    {'\'', 0x07}, {'(', 0x08}, {')', 0x09}, {'*', 0x0A}, {'+', 0x0B},  {',', 0x0C}, {'-', 0x0D},
    {'.', 0x0E},  {'/', 0x0F}, {'0', 0x10}, {'1', 0x11}, {'2', 0x12},  {'3', 0x13}, {'4', 0x14},
    {'5', 0x15},  {'6', 0x16}, {'7', 0x17}, {'8', 0x18}, {'9', 0x19},  {':', 0x1A}, {';', 0x1B},
    {'<', 0x1C},  {'=', 0x1D}, {'>', 0x1E}, {'?', 0x1F}, {'@', 0x20},  {'A', 0x21}, {'B', 0x22},
    {'C', 0x23},  {'D', 0x24}, {'E', 0x25}, {'F', 0x26}, {'G', 0x27},  {'H', 0x28}, {'I', 0x29},
    {'J', 0x2A},  {'K', 0x2B}, {'L', 0x2C}, {'M', 0x2D}, {'N', 0x2E},  {'O', 0x2F}, {'P', 0x30},
    {'Q', 0x31},  {'R', 0x32}, {'S', 0x33}, {'T', 0x34}, {'U', 0x35},  {'V', 0x36}, {'W', 0x37},
    {'X', 0x38},  {'Y', 0x39}, {'Z', 0x3A}, {'[', 0x3B}, {'\\', 0x3C}, {']', 0x3D}, {'^', 0x3E},
    {'_', 0x3F},
};

const std::vector<char> ascii_6b_table_reverse = {
    ' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')', '*', '+', ',',  '-', '.', '/',
    '0', '1', '2', '3', '4', '5', '6', '7',  '8', '9', ':', ';', '<',  '=', '>', '?',
    '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G',  'H', 'I', 'J', 'K', 'L',  'M', 'N', 'O',
    'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W',  'X', 'Y', 'Z', '[', '\\', ']', '^', '_'};

bool
encode(std::string text, Encoding enc, bytes &outb, std::string &err);
bool
decode(std::string &text, Encoding &enc, biterator &inb, std::string &err);
bool
makeTypeLengthByte(Encoding enc, uchar byte_count, std::byte &outb, std::string &err);
bool
decodeTypeLengthByte(Encoding &enc, uchar &byte_count, std::byte inb, std::string &err);
int
precalcLength(std::string &text, Encoding &enc);
void
bytesToHexStr(bytes &bs, std::string &outs);
bool
hexStrToBytes(std::string &ins, bytes &bs, std::string &err);

void
debug_PrintByte(std::byte b);
