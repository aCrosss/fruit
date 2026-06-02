#include <iostream>

#include "encoding.hpp"

char
nibble_to_hex(uchar d) {
    char c = 0;

    if (d < 10) {
        c = d + '0';
    } else {
        c = d - 10 + 'A';
    }

    return c;
}

int
hex_to_nibble(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    return -1;
}

void
debugPrintBytes(bytes &bs) {
    for (size_t i = 0; i < bs.size(); i++) {
        char b  = static_cast<uchar>(bs[i]);
        char hi = nibble_to_hex((b >> 4) & 0x0F);
        char lo = nibble_to_hex((b >> 0) & 0x0F);
        std::cout << hi << lo << " ";
    }
    std::cout << std::endl;
}

bool
encode_binary(std::string text, bytes &out, std::string &err) {
    const char *str = text.c_str();

    // remove spaces and validate
    std::string buff;
    for (size_t i = 0; i < text.size(); i++, str++) {
        if (*str == ' ') {
            continue;
        }

        if (hex_to_nibble(*str) < 0) {
            std::stringstream s;
            s << "unsupported charachter " << *str;
            err += s.str();
            return false;
        }

        buff += *str;
    }

    // can't make binary from odd number of hex digits
    if (buff.size() % 2 != 0) {
        err = "uneven number of hexadeciamal charachters";
        return false;
    }

    str = buff.c_str();
    for (size_t i = 0; i < buff.size() / 2; i++) {
        uchar     hi = (uchar)hex_to_nibble(str[2 * i + 0]);
        uchar     lo = (uchar)hex_to_nibble(str[2 * i + 1]);
        std::byte b{static_cast<uchar>((hi << 4) | (lo << 0))};
        out.emplace_back(b);
    }

    debugPrintBytes(out);
    return true;
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
        uchar b;

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

    uchar     cpack[4] = {0};
    std::byte bytes[3];

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
        ASCII_6BIT_PACK_CHARS(
            cpack[0], cpack[1], cpack[2], cpack[3], bytes[0], bytes[1], bytes[2]);
        out.insert(out.end(), std::begin(bytes), std::end(bytes));
    }

    debugPrintBytes(out);
    return true;
}

bool
encode(std::string text, Encoding enc, bytes &outb, std::string &err) {
    switch (enc) {
    case ENCODING_BINARY_UNSPEC: return encode_binary(text, outb, err);
    case ENCODING_BCDp         : return encode_bcdp(text, outb, err);
    case ENCODING_ASCII_6b     : return encode_ascii6bit(text, outb, err);

    // TODO: ASCII+Latin, fn unicode
    default: std::cout << "encode: unknown encoding" << std::endl; return false;
    }
}

//@brief Make byte describing type/length of a encoded string. Must be less or equal than
// 31 bytes, otherwise will return false and err
//@param enc type code of encoded field
//@param byte_count number of bytes of ALREADY ENCODED text; must be less than 31
//@param &err out error string
bool
makeTypeLengthByte(Encoding enc, uchar byte_count, std::byte &outb, std::string &err) {
    uchar type_code = 0;

    // can't encode more than 31 bytes
    if (byte_count > ENCODED_MAX_BYTE_LENGTH) {
        err = "encoded more than 31 bytes";
        return false;
    }

    switch (enc) {
    case ENCODING_BINARY_UNSPEC:
    case ENCODING_BCDp:
    case ENCODING_ASCII_6b:
    case ENCODING_LANG_CODE    : type_code = static_cast<uchar>(enc); break;
    default                    : err = "unknown type code"; return false;
    }

    uchar byte = static_cast<uchar>(type_code << 6 | (byte_count & ENCODED_MAX_BYTE_LENGTH));
    outb       = std::byte{byte};
    return true;
}
