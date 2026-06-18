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
debug_PrintBytes(bytes &bs) {
    for (size_t i = 0; i < bs.size(); i++) {
        char b  = static_cast<uchar>(bs[i]);
        char hi = nibble_to_hex((b >> 4) & 0x0F);
        char lo = nibble_to_hex((b >> 0) & 0x0F);
        std::cout << hi << lo << " ";
    }
    std::cout << std::endl;
}

void
debug_PrintByte(std::byte byte) {
    char b  = static_cast<uchar>(byte);
    char hi = nibble_to_hex((b >> 4) & 0x0F);
    char lo = nibble_to_hex((b >> 0) & 0x0F);
    std::cout << hi << lo;
}

//    ######## ##    ##  ######   #######  ########  #### ##    ##  ######
//    ##       ###   ## ##    ## ##     ## ##     ##  ##  ###   ## ##    ##
//    ##       ####  ## ##       ##     ## ##     ##  ##  ####  ## ##
//    ######   ## ## ## ##       ##     ## ##     ##  ##  ## ## ## ##   ####
//    ##       ##  #### ##       ##     ## ##     ##  ##  ##  #### ##    ##
//    ##       ##   ### ##    ## ##     ## ##     ##  ##  ##   ### ##    ##
//    ######## ##    ##  ######   #######  ########  #### ##    ##  ######

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

    debug_PrintBytes(out);
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

    debug_PrintBytes(out);
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

    debug_PrintBytes(out);
    return true;
}

bool
encode_unicode(std::string text, bytes &out, std::string &err) {
    uchar *raw = reinterpret_cast<uchar *>(text.data());
    for (size_t i = 0; i < text.size(); i++) {
        out.emplace_back(std::byte{raw[i]});
    }

    return true;
}

bool
encode(std::string text, Encoding enc, bytes &outb, std::string &err) {
    switch (enc) {
    case ENCODING_BINARY_UNSPEC: return encode_binary(text, outb, err);
    case ENCODING_BCDp         : return encode_bcdp(text, outb, err);
    case ENCODING_ASCII_6b     : return encode_ascii6bit(text, outb, err);
    case ENCODING_UNOCODE      : return encode_unicode(text, outb, err);

    // TODO: ASCII+Latin, fn unicode
    default: err = "encode: unknown encoding"; return false;
    }
}

//    ########  ########  ######   #######  ########  #### ##    ##  ######
//    ##     ## ##       ##    ## ##     ## ##     ##  ##  ###   ## ##    ##
//    ##     ## ##       ##       ##     ## ##     ##  ##  ####  ## ##
//    ##     ## ######   ##       ##     ## ##     ##  ##  ## ## ## ##   ####
//    ##     ## ##       ##       ##     ## ##     ##  ##  ##  #### ##    ##
//    ##     ## ##       ##    ## ##     ## ##     ##  ##  ##   ### ##    ##
//    ########  ########  ######   #######  ########  #### ##    ##  ######

bool
decode_binary(std::string &text, biterator &inb, uchar byte_count, std::string &err) {
    for (size_t i = 0; i < byte_count; i++, inb++) {
        uchar hi = (static_cast<uchar>(*inb) >> 4) & 0x0F;
        uchar lo = (static_cast<uchar>(*inb) >> 0) & 0x0F;

        text += nibble_to_hex(hi);
        text += nibble_to_hex(lo);
    }

    return true;
}

bool
decode_bcdp(std::string &text, biterator &inb, uchar byte_count, std::string &err) {
    for (size_t i = 0; i < byte_count; i++, inb++) {
        uchar b = static_cast<uchar>(*inb);
        uchar c;

        switch (b) {
        case 0x0: c = '0'; break;
        case 0x1: c = '1'; break;
        case 0x2: c = '2'; break;
        case 0x3: c = '3'; break;
        case 0x4: c = '4'; break;
        case 0x5: c = '5'; break;
        case 0x6: c = '6'; break;
        case 0x7: c = '7'; break;
        case 0x8: c = '8'; break;
        case 0x9: c = '9'; break;
        case 0xA: c = ' '; break;
        case 0xB: c = '-'; break;
        case 0xC: c = '.'; break;

        // other symbols not supported
        default:
            std::stringstream s;
            s << "unsupported charachter " << std::hex << b;
            err += s.str();
            return false;
        }

        text += c;
    }

    return true;
}

bool
decode_ascii6bit(std::string &text, biterator &inb, uchar byte_count, std::string &err) {
    // encoded by 3 bytes
    if ((byte_count) % 3) {
        err = "6 bit ascii text must be encoded in groups of 3 bytes";
        return false;
    }

    std::string buff;
    for (size_t i = 0; i < byte_count; i += 3, inb += 3) {
        uchar b1   = static_cast<uchar>(inb[0]);
        uchar b2   = static_cast<uchar>(inb[1]);
        uchar b3   = static_cast<uchar>(inb[2]);
        uchar c[4] = {0};

        ASCII_6BIT_UNPACK_CHARS(b1, b2, b3, c[0], c[1], c[2], c[3])

        for (size_t j = 0; j < 4; j++) {
            if (c[j] >= ascii_6b_table_reverse.size()) {
                std::stringstream s;
                s << "unsupported charachter " << std::hex << c[j];
                err += s.str();
                return false;
            }

            buff += ascii_6b_table_reverse[c[j]];
        }
    }

    // remove trailing spaces if they present
    if (*(buff.end() - 1) == ' ') {
        size_t sp_pos = buff.find_last_not_of(' ');
        text          = buff.substr(0, sp_pos + 1);
    } else {
        text = buff;
    }

    return true;
}

bool
decode_unicode(std::string &text, biterator &inb, uchar byte_count, std::string &err) {
    bytes bs;

    bs.insert(bs.begin(), inb, inb + byte_count);

    text  = std::string(reinterpret_cast<char *>(bs.data()), bs.size());
    inb  += byte_count;
    return true;
}

bool
decode(std::string &text, Encoding &enc, biterator &inb, std::string &err) {
    std::string str;
    uchar       byte_count = 0;

    if (!decodeTypeLengthByte(enc, byte_count, *(inb++), err)) {
        return false;
    }

    switch (enc) {
    case ENCODING_BINARY_UNSPEC: return decode_binary(text, inb, byte_count, err);
    case ENCODING_BCDp         : return decode_bcdp(text, inb, byte_count, err);
    case ENCODING_ASCII_6b     : return decode_ascii6bit(text, inb, byte_count, err);
    case ENCODING_UNOCODE      : return decode_unicode(text, inb, byte_count, err);

    // TODO: ASCII+Latin, fn unicode
    default:
        std::stringstream s;
        s << "unknown encoding " << static_cast<int>(enc);
        err = s.str();
        return false;
    }
}

//    ##     ## ####  ######   ######
//    ###   ###  ##  ##    ## ##    ##
//    #### ####  ##  ##       ##
//    ## ### ##  ##   ######  ##
//    ##     ##  ##        ## ##
//    ##     ##  ##  ##    ## ##    ##
//    ##     ## ####  ######   ######

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
    case ENCODING_UNOCODE      : type_code = static_cast<uchar>(enc); break;
    default                    : err = "unknown type code"; return false;
    }

    uchar byte = static_cast<uchar>(type_code << 6 | (byte_count & ENCODED_MAX_BYTE_LENGTH));
    outb       = std::byte{byte};
    return true;
}

bool
decodeTypeLengthByte(Encoding &enc, uchar &byte_count, std::byte inb, std::string &err) {
    uchar byte = static_cast<uchar>(inb);

    int type_code = (byte >> 6) & 0x03;
    int length    = byte & ENCODED_MAX_BYTE_LENGTH;

    if (type_code < ENCODING_BINARY_UNSPEC || type_code > ENCODING_UNOCODE) {
        std::stringstream s;
        s << "invalid type code " << type_code;
        err += s.str();
        return false;
    }

    if (length < 0 || length > ENCODED_MAX_BYTE_LENGTH) {
        std::stringstream s;
        s << "invalid byte count " << length;
        err += s.str();
        return false;
    }

    enc        = Encoding(type_code);
    byte_count = static_cast<uchar>(length);
    return true;
}

//@brief Calculates actual string length in bytes in given encoding + 1 byte for type/length
// byte
//@param &text std::string which length will be calculated
//@param &enc encoding of given text
int
precalcLength(std::string &text, Encoding &enc) {
    switch (enc) {
    // internal binary string SHOULDN'T be not in multiples of 2
    case ENCODING_BINARY_UNSPEC: return text.length() / 2 + 1;
    case ENCODING_BCDp         : return text.length() + 1;
    case ENCODING_ASCII_6b     : return ((text.length() + 3) / 4) * 3 + 1;
    case ENCODING_UNOCODE      : return text.size() + 1;
    default                    : return -1;
    }
}

void
bytesToHexStr(bytes &bs, std::string &outs) {
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (const auto &byte : bs) {
        oss << std::setw(2) << static_cast<int>(byte);
    }

    outs = oss.str();
}

bool
hexStrToBytes(std::string &ins, bytes &bs, std::string &err) {
    return encode_binary(ins, bs, err);
}
