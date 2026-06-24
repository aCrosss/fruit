#include "types.hpp"

std::unordered_map<std::string, Encoding> encoding_map{
    {"binary", ENCODING_BINARY_UNSPEC},
    {"bcdp", ENCODING_BCDp},
    {"ascii6bit", ENCODING_ASCII_6b},
    {"unicode", ENCODING_UNOCODE},
};

std::string
encodingToString(Encoding enc) {
    switch (enc) {
    case ENCODING_BINARY_UNSPEC: return "binary";
    case ENCODING_BCDp         : return "bcdp";
    case ENCODING_ASCII_6b     : return "ascii6bit";
    case ENCODING_UNOCODE      : return "unicode";

    default: return "unknown";
    }
}
