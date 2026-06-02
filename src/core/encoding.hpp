#pragma once

#include <map>
#include <sstream>
#include <string>

#include "types.hpp"

#define ENCODED_MAX_BYTE_LENGTH 31

#define ASCII_6BIT_MAX_IND 0x3F // '_' aka 111111b
// pack charachters c1-c4 into bytes b1-b3
// it supposed to work with charachers from ascii_6b_table!
#define ASCII_6BIT_PACK_CHARS(c1, c2, c3, c4, b1, b2, b3)           \
    b1 = std::byte((c1 & ASCII_6BIT_MAX_IND) | ((c2 & 0x03) << 6)); \
    b2 = std::byte(((c2 >> 2) & 0x0F) | ((c3 & 0x0F) << 4));        \
    b3 = std::byte(((c3 >> 4) & 0x03) | ((c4 & ASCII_6BIT_MAX_IND) << 2))

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

bool
encode(std::string text, Encoding enc, bytes &outb, std::string &err);
bool
makeTypeLengthByte(Encoding enc, uchar byte_count, std::byte &outb, std::string &err);
