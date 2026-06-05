#include <iostream>

#include "areaChassis.hpp"

void
debug_printEncStr(std::string tag, encodedStr str) {
    std::cout << tag << " {data=\'" << str.str << "'"
              << " encoding=" << encodingToString(str.enc) << "}" << std::endl;
}

void
AreaChassis::debug_printOutVals() {
    debug_printEncStr("type", type);
    debug_printEncStr("part_number", part_number);
    debug_printEncStr("serial_number", serial_number);

    std::cout << "custom:" << std::endl;
    for (size_t i = 0; i < custom.size(); i++) {
        std::stringstream s;
        s << "  [" << i << "]";
        debug_printEncStr(s.str(), custom[i]);
    }
}

bool
AreaChassis::validate() {
    return true;
}

//    ########     ###    ########   ######  #### ##    ##  ######
//    ##     ##   ## ##   ##     ## ##    ##  ##  ###   ## ##    ##
//    ##     ##  ##   ##  ##     ## ##        ##  ####  ## ##
//    ########  ##     ## ########   ######   ##  ## ## ## ##   ####
//    ##        ######### ##   ##         ##  ##  ##  #### ##    ##
//    ##        ##     ## ##    ##  ##    ##  ##  ##   ### ##    ##
//    ##        ##     ## ##     ##  ######  #### ##    ##  ######

bool
AreaChassis::tryParseJSON(nlohmann::json j, Errs &errs) {
    clear();

    std::string err;
    bool        valid = true;

    if (!tryDecodeStr(j, "type", type, errs)) {
        valid = false;
    }

    if (!tryDecodeStr(j, "part_number", part_number, errs)) {
        valid = false;
    }

    if (!tryDecodeStr(j, "serial_number", serial_number, errs)) {
        valid = false;
    }

    // custom field is optional
    if (!j.contains("custom")) {
        debug_printOutVals();
        return valid;
    }

    jarray jarray;
    if (!tryParseFieldJSON_arr(j["custom"], jarray, err)) {
        errs.append(tag, "custom", err);
        err.clear();
    }

    std::cout << err << std::endl;
    for (size_t i = 0; i < jarray.size(); i++) {
        json       jentry = jarray[i];
        encodedStr estr;

        std::stringstream s;
        s << "custom[" << i << "]";
        if (!tryDecodeStr(jentry, s.str(), estr, errs)) {
            valid = false;
            continue;
        }

        custom.emplace_back(estr);
    }

    debug_printOutVals();
    return valid;
}

bool
AreaChassis::tryParseTOML() {
    return false;
}

bool
AreaChassis::tryParseBinary(bytes::iterator in_bin, Errs &errs) {
    clear();

    bytes::iterator begin = in_bin;

    // get area length byte at index 1
    uchar length = static_cast<uchar>(*(++in_bin)) * 8;

    uchar checksum     = static_cast<uchar>(*(begin + length - 1));
    uchar checksum_rec = static_cast<uchar>(calcZeroChecksum(begin, begin + length - 1));
    if (checksum != checksum_rec) {
        std::cout << std::hex << checksum << std::endl;
        std::cout << std::hex << checksum_rec << std::endl;
        errs.append(tag, "common", "checksum is invalid");
        return false;
    }

    if (!tryDecodeStr(++in_bin, "type", type, errs)) {
        return false;
    }
    if (!tryDecodeStr(in_bin, "part_number", part_number, errs)) {
        return false;
    }

    if (!tryDecodeStr(in_bin, "serial_number", serial_number, errs)) {
        return false;
    }

    size_t i = 0;
    while (*(in_bin) != END_OF_FIELDS_BYTE) {
        encodedStr        es;
        std::stringstream s;
        s << "custom[" << i++ << "]";

        if (!tryDecodeStr(in_bin, s.str(), es, errs)) {
            return false;
        }

        custom.emplace_back(es);
    }

    std::cout << "Parsed binary:" << std::endl;
    debug_printOutVals();
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
}

void
AreaChassis::emitTOML() {
}

bool
AreaChassis::emitBinary(bytes &out_bin, Errs &errs) {
    bytes bs;

    std::string err;
    bool        valid = true;

    bytes type_bs;
    bytes part_number_bs;
    bytes serial_number_bs;
    bytes custom_bs;

    if (!tryEncodeStr("type", type, type_bs, errs)) {
        valid = false;
    }

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

    uchar length = 1 /*header byte*/ + 1 /*length*/ + type_bs.size() + part_number_bs.size() +
                   serial_number_bs.size() + custom_bs.size() + 1 /*end of fields*/ + 1 /*crc*/;
    // total length
    uchar total  = length / 8 + 1;
    // byte count of zero filled unused space
    uchar unused = total * 8 - length;

    // header
    bs.emplace_back(DEFAULT_SECTION_HEADER_BYTE);
    // total length (in multiples of 8 bytes)
    bs.emplace_back(std::byte{total});
    // encoded fields
    bs.insert(bs.end(), type_bs.begin(), type_bs.end());
    bs.insert(bs.end(), part_number_bs.begin(), part_number_bs.end());
    bs.insert(bs.end(), serial_number_bs.begin(), serial_number_bs.end());
    bs.insert(bs.end(), custom_bs.begin(), custom_bs.end());
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

size_t
AreaChassis::getByteLen() {
    return 0;
}

void
AreaChassis::clear() {
    type.str.clear();
    type.enc = ENCODING_BINARY_UNSPEC;

    part_number.str.clear();
    part_number.enc = ENCODING_BINARY_UNSPEC;

    serial_number.str.clear();
    serial_number.enc = ENCODING_BINARY_UNSPEC;

    custom.clear();
}

AreaChassis::AreaChassis(/* args */) : Section("chassis", "Chassis Info Area") {
    //
}

AreaChassis::~AreaChassis() {
    //
}
