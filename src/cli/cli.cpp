#include <iostream>
#include <string>

#include "../core/encoding.hpp"

int
main() {
    std::string s = "ipmi";
    bytes       b;
    std::string err;

    encode(s, ENCODING_ASCII_6b, b, err);
    std::cout << err;

    std::string s2 = "0123456789 -.";
    bytes       b2;
    encode(s2, ENCODING_BCDp, b2, err);
    std::cout << err;

    return 0;
}
