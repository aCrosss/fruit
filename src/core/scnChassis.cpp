#include <iostream>

#include "scnChassis.hpp"

// nlohmann::json j, FRU_errs &errs and std::string err must be provided
#define TRY_PARSE_ENC_STR(ftag, field)                     \
    if (j.contains(ftag)) {                                \
        json jval = j[ftag];                               \
        if (!tryParseFieldJSON_encStr(jval, field, err)) { \
            valid = false;                                 \
            errs.append(tag, ftag, err);                   \
            err.clear();                                   \
        }                                                  \
    } else {                                               \
        std::stringstream s;                               \
        s << "field '" << ftag << "' is missing";          \
        errs.append(tag, ftag, s.str());                   \
        valid = false;                                     \
    }

void
scnChassis::debug_printOutVals() {
    std::cout << "type: {data=" << type.str << "; encoding=" << encodingToString(type.enc)
              << "}" << std::endl;
    std::cout << "part_number: {data=" << part_number.str
              << "; encoding=" << encodingToString(part_number.enc) << "}" << std::endl;
    std::cout << "serial_number: {data=" << serial_number.str
              << "; encoding=" << encodingToString(serial_number.enc) << "}" << std::endl;

    std::cout << "custom:" << std::endl;
    for (size_t i = 0; i < custom.size(); i++) {
        encodedStr es = custom[i];
        std::cout << "  [" << i << "]: {data=" << es.str
                  << "; encoding=" << encodingToString(es.enc) << "}" << std::endl;
    }
}

bool
scnChassis::validate() {
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
scnChassis::tryParseJSON(nlohmann::json j, FRU_errs &errs) {

    std::string err;
    bool        valid = true;

    TRY_PARSE_ENC_STR("type", type)
    TRY_PARSE_ENC_STR("part_number", part_number)
    TRY_PARSE_ENC_STR("serial_number", serial_number)

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
        if (!tryParseFieldJSON_encStr(jentry, estr, err)) {
            valid = false;
            std::stringstream s;
            s << "custom[" << i << "]";
            errs.append(tag, s.str(), err);
            err.clear();
            continue;
        }

        custom.emplace_back(estr);
    }

    debug_printOutVals();
    return valid;
}

bool
scnChassis::tryParseTOML() {
    return false;
}

bool
scnChassis::tryParseBinary(bytes &out_bin) {
    return false;
}

//    ######## ##     ## #### ######## #### ##    ##  ######
//    ##       ###   ###  ##     ##     ##  ###   ## ##    ##
//    ##       #### ####  ##     ##     ##  ####  ## ##
//    ######   ## ### ##  ##     ##     ##  ## ## ## ##   ####
//    ##       ##     ##  ##     ##     ##  ##  #### ##    ##
//    ##       ##     ##  ##     ##     ##  ##   ### ##    ##
//    ######## ##     ## ####    ##    #### ##    ##  ######

void
scnChassis::emitJSON(nlohmann::json &j) {
}

void
scnChassis::emitTOML() {
}

bool
scnChassis::emitBinary(bytes &out_bin, FRU_errs &errs) {
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
scnChassis::getByteLen() {
    //
}

scnChassis::scnChassis(/* args */) : Section("chassis", "Chassis Info Area") {
    //
}

scnChassis::~scnChassis() {
    //
}
