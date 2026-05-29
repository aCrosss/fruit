#include <iostream>

#include "encoding.hpp"

char
nibble_to_hex(unsigned char d) {
    char c = 0;

    if (d < 10) {
        c = d + '0';
    } else {
        c = d - 10 + 'A';
    }

    return c;
}

void
debugPrintBytes(bytes &bs) {
    for (size_t i = 0; i < bs.size(); i++) {
        char b  = static_cast<unsigned char>(bs[i]);
        char hi = nibble_to_hex((b >> 4) & 0x0F);
        char lo = nibble_to_hex((b >> 0) & 0x0F);
        std::cout << hi << lo << " ";
    }
    std::cout << std::endl;
}

// 0h - 9h = digits 0 through 9
// Ah      = space
// Bh      = dash ‘-’
// Ch      = period ‘.’
// Dh      = reserved
// Eh      = reserved
// Fh      = reserved
bool
encode_bcdp(std::string text, bytes &out, std::string &err) {
    const char *str = text.c_str();

    while (*str != '\0') {
        unsigned char b;

        switch (*str) {
        case '0': b = 0x0; break;
        case '1': b = 0x1; break;
        case '2': b = 0x2; break;
        case '3': b = 0x3; break;
        case '4': b = 0x4; break;
        case '5': b = 0x5; break;
        case '6': b = 0x6; break;
        case '7': b = 0x7; break;
        case '8': b = 0x8; break;
        case '9': b = 0x9; break;
        case ' ': b = 0xA; break;
        case '-': b = 0xB; break;
        case '.': b = 0xC; break;

        // other symbols not supported
        default:
            std::stringstream s;
            s << "unsupported charachter " << *str;
            err += s.str();
            return false;
        }

        out.emplace_back(std::byte(b));
        str++;
    }

    debugPrintBytes(out);
    return true;
}

bool
encode_ascii6bit(std::string text, bytes &out, std::string &err) {
    const char *str = text.c_str();

    unsigned char cpack[4] = {0};
    std::byte     bytes[3];

    size_t i;
    for (i = 0; *str != '\0'; i++, str++) {
        const char c = toupper(*str);
        if (ascii_6b_table.find(c) == ascii_6b_table.end()) {
            std::stringstream s;
            s << "unsupported charachter " << c;
            err += s.str();
            return false;
        }

        cpack[i % 4] = ascii_6b_table.at(c);

        // we have our 4 chars, can perform packing
        if (i % 4 == 3) {
            ASCII_6BIT_PACK_CHARS(
                cpack[0], cpack[1], cpack[2], cpack[3], bytes[0], bytes[1], bytes[2]);

            out.insert(out.end(), std::begin(bytes), std::end(bytes));
            std::fill(std::begin(cpack), std::end(cpack), 0);
            std::fill(std::begin(bytes), std::end(bytes), std::byte{0});
        }
    }

    // have leftovers
    if (i % 4 != 0) {
        ASCII_6BIT_PACK_CHARS(cpack[0], cpack[1], cpack[2], cpack[3], bytes[0], bytes[1], bytes[2]);
        out.insert(out.end(), std::begin(bytes), std::end(bytes));
    }

    debugPrintBytes(out);
    return true;
}

bool
encode(std::string text, Encoding enc, bytes &outb, std::string &err) {
    switch (enc) {
    case ENCODING_BINARY_UNSPEC: return true;
    case ENCODING_BCDp         : return encode_bcdp(text, outb, err);
    case ENCODING_ASCII_6b     : return encode_ascii6bit(text, outb, err);

    // TODO: ASCII+Latin, fn unicode
    default: return false;
    }
}
