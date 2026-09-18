#include <iostream>

#include "areaChassis.hpp"

//    ##        #######   ######     ###    ##
//    ##       ##     ## ##    ##   ## ##   ##
//    ##       ##     ## ##        ##   ##  ##
//    ##       ##     ## ##       ##     ## ##
//    ##       ##     ## ##       ######### ##
//    ##       ##     ## ##    ## ##     ## ##
//    ########  #######   ######  ##     ## ########

void
debug_printEncStr(std::string tag, encodedStr str) {
    std::cout << tag << " {data=\'" << str.str << "'"
              << " encoding=" << encodingToString(str.enc) << "}" << std::endl;
}

void
AreaChassis::debug_printOutVals() {
    std::cout << "=== " << label << " ===" << std::endl;
    std::cout << "type=" << static_cast<int>(type) << std::endl;
    debug_printEncStr("part_number", part_number);
    debug_printEncStr("serial_number", serial_number);

    std::cout << "custom:" << std::endl;
    for (size_t i = 0; i < custom.size(); i++) {
        std::stringstream s;
        s << "  [" << i << "]";
        debug_printEncStr(s.str(), custom[i]);
    }
}

void
AreaChassis::clear() {
    type = 0;

    CLEAR_ENC_STR(part_number);
    CLEAR_ENC_STR(serial_number);
    custom.clear();

    present = false;
}

uchar
AreaChassis::getLength() {
    ssize_t length  = const_len;
    length         += precalcLength(part_number.str, part_number.enc);
    length         += precalcLength(serial_number.str, serial_number.enc);
    for (size_t i = 0; i < custom.size(); i++) {
        length += precalcLength(custom[i].str, custom[i].enc);
    }

    // total length
    return ROUND_LEN_TO_8_BYTES_MULTPL(length);
}

//    ########     ###    ########   ######  #### ##    ##  ######
//    ##     ##   ## ##   ##     ## ##    ##  ##  ###   ## ##    ##
//    ##     ##  ##   ##  ##     ## ##        ##  ####  ## ##
//    ########  ##     ## ########   ######   ##  ## ## ## ##   ####
//    ##        ######### ##   ##         ##  ##  ##  #### ##    ##
//    ##        ##     ## ##    ##  ##    ##  ##  ##   ### ##    ##
//    ##        ##     ## ##     ##  ######  #### ##    ##  ######

template <typename T>
inline bool
AreaChassis::tryParseImpl(T v, Errs &errs) {
    std::string err;
    bool        valid = true;

    int t = 0;
    if (!tryParseField_int(v, "type", t, errs)) {
        valid = false;
    }
    type = static_cast<uchar>(t);

    if (!tryParseField_encStr(v, "part_number", part_number, errs)) {
        valid = false;
    }

    if (!tryParseField_encStr(v, "serial_number", serial_number, errs)) {
        valid = false;
    }

    // custom field is optional
    if (!v.contains("custom")) {
        debug_printOutVals();
        present = true;
        return valid;
    }

    if (!tryParseField_encStrArr(v, "custom", custom, errs)) {
        return false;
    }

    debug_printOutVals();
    present = true;
    return valid;
}

bool
AreaChassis::tryParse(nlohmann::json j, Errs &errs) {
    clear();

    return tryParseImpl(j, errs);
}

bool
AreaChassis::tryParse(toml::value &t, Errs &errs) {
    clear();

    return tryParseImpl(t, errs);
}

bool
AreaChassis::tryParseBinary(ibytes begin, ibytes end, Errs &errs) {
    UNUSED(end);
    clear();

    ibytes beg = begin;

    // get area length byte at index 1
    uchar length = IPMI_TO_REAL_LEN(static_cast<uchar>(*(++begin)));

    if (!checkChecksums(beg + length - 1, beg, beg + length - 2, errs)) {
        return false;
    }

    // type kinda can be all way up to 0xFF
    type = static_cast<uchar>(*(++begin));

    begin += 1;
    if (!tryDecodeStr(begin, end, "part_number", part_number, errs)) {
        return false;
    }

    if (!tryDecodeStr(begin, end, "serial_number", serial_number, errs)) {
        return false;
    }

    size_t i = 0;
    while (*((begin)) != END_OF_FIELDS_BYTE) {
        encodedStr        es;
        std::stringstream s;
        s << "custom[" << i++ << "]";

        if (!tryDecodeStr(begin, end, s.str(), es, errs)) {
            return false;
        }

        custom.emplace_back(es);
    }

    debug_printOutVals();
    present = true;
    return true;
}

//    ######## ##     ## #### ######## #### ##    ##  ######
//    ##       ###   ###  ##     ##     ##  ###   ## ##    ##
//    ##       #### ####  ##     ##     ##  ####  ## ##
//    ######   ## ### ##  ##     ##     ##  ## ## ## ##   ####
//    ##       ##     ##  ##     ##     ##  ##  #### ##    ##
//    ##       ##     ##  ##     ##     ##  ##   ### ##    ##
//    ######## ##     ## ####    ##    #### ##    ##  ######

void
AreaChassis::emitJSON(nlohmann::json &j) {
    if (!present) {
        return;
    }

    j["type"] = type;

    emitEncStr(j, "part_number", part_number);
    emitEncStr(j, "serial_number", serial_number);
    emitEncStrArr(j, "custom", custom);
}

void
AreaChassis::emitTOML(toml::table &t) {
    if (!present) {
        return;
    }

    t["type"] = toml::value(type);

    emitEncStr(t, "part_number", part_number);
    emitEncStr(t, "serial_number", serial_number);
    emitEncStrArr(t, "custom", custom);
}

bool
AreaChassis::emitBinary(bytes &out_bin, Errs &errs) {
    if (!present) {
        return true;
    }

    bytes bs;

    std::string err;
    bool        valid = true;

    bytes part_number_bs;
    bytes serial_number_bs;
    bytes custom_bs;

    if (!tryEncodeStr("part_number", part_number, part_number_bs, errs)) {
        valid = false;
    }

    if (!tryEncodeStr("serial_number", serial_number, serial_number_bs, errs)) {
        valid = false;
    }

    for (size_t i = 0; i < custom.size(); i++) {
        auto              estr = custom[i];
        bytes             bs;
        std::stringstream ftag;
        ftag << "custom[" << i << "]";

        if (tryEncodeStr(ftag.str(), estr, bs, errs)) {
            custom_bs.insert(custom_bs.end(), bs.begin(), bs.end());
        } else {
            valid = false;
            continue;
        }
    }

    if (!valid) {
        return false;
    }

    uchar length =
        const_len + part_number_bs.size() + serial_number_bs.size() + custom_bs.size();
    // total length
    uchar total  = getLength();
    // byte count of zero filled unused space
    uchar unused = IPMI_TO_REAL_LEN(total) - length;

    // header
    bs.emplace_back(DEFAULT_SECTION_HEADER_BYTE);
    // total length (in multiples of 8 bytes)
    bs.emplace_back(std::byte{total});
    // type enum
    bs.emplace_back(std::byte{type});
    // encoded fields
    APPEND_BYTES(bs, part_number_bs);
    APPEND_BYTES(bs, serial_number_bs);
    APPEND_BYTES(bs, custom_bs);
    // end of fields
    bs.emplace_back(END_OF_FIELDS_BYTE);
    // fill unused space with zeroes
    bs.insert(bs.end(), unused, std::byte{0});
    // zero checksum
    bs.emplace_back(calcZeroChecksum(bs));

    // emmit
    out_bin.insert(out_bin.end(), bs.begin(), bs.end());
    return true;
}

//    #### ##    ## #### ########
//     ##  ###   ##  ##     ##
//     ##  ####  ##  ##     ##
//     ##  ## ## ##  ##     ##
//     ##  ##  ####  ##     ##
//     ##  ##   ###  ##     ##
//    #### ##    ## ####    ##

AreaChassis::AreaChassis(/* args */) : Section("chassis", "Chassis Info Area") {
    //
}
