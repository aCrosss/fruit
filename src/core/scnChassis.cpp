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

void
scnChassis::emmitBinary(bytes &out_bin) {
    //
}

bool
scnChassis::decodeBinary(bytes &out_bin) {
    return false;
}

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
