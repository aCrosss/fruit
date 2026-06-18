#include <iostream>

#include "areaInternalUse.hpp"

//    ##        #######   ######     ###    ##
//    ##       ##     ## ##    ##   ## ##   ##
//    ##       ##     ## ##        ##   ##  ##
//    ##       ##     ## ##       ##     ## ##
//    ##       ##     ## ##       ######### ##
//    ##       ##     ## ##    ## ##     ## ##
//    ########  #######   ######  ##     ## ########

void
AreaInternalUse::debug_printOutVals() {
    std::cout << "=== " << label << " ===" << std::endl;
    for (size_t i = 0; i < internal_data.size(); i++) {
        debug_PrintByte(internal_data[i]);
        std::cout << " ";
    }
    std::cout << std::endl;
}

uchar
AreaInternalUse::getLength() {
    ssize_t length  = const_len;
    length         += internal_data.size();

    // total length
    return ROUND_LEN_TO_8_BYTES_MULTPL(length);
}

void
AreaInternalUse::clear() {
    internal_data.clear();
}

//    ########     ###    ########   ######  #### ##    ##  ######
//    ##     ##   ## ##   ##     ## ##    ##  ##  ###   ## ##    ##
//    ##     ##  ##   ##  ##     ## ##        ##  ####  ## ##
//    ########  ##     ## ########   ######   ##  ## ## ## ##   ####
//    ##        ######### ##   ##         ##  ##  ##  #### ##    ##
//    ##        ##     ## ##    ##  ##    ##  ##  ##   ### ##    ##
//    ##        ##     ## ##     ##  ######  #### ##    ##  ######

template <typename T>
bool
AreaInternalUse::tryParseImpl(T v, Errs &errs) {
    std::string err;
    std::string hex;

    if (!tryParseField_str(v, "data", hex, errs)) {
        return false;
    }

    if (!hexStrToBytes(hex, internal_data, err)) {
        errs.append(tag, "data", err);
        return false;
    }

    present = true;
    return true;
}

bool
AreaInternalUse::tryParseJSON(nlohmann::json j, Errs &errs) {
    clear();

    return tryParseImpl(j, errs);
}

bool
AreaInternalUse::tryParseTOML(toml::value &t, Errs &errs) {
    clear();

    return tryParseImpl(t, errs);
}

bool
AreaInternalUse::tryParseBinary(biterator begin, biterator end, Errs &errs) {
    if (begin >= end) {
        errs.append(tag, "data", "out of bounds");
        return false;
    }

    clear();

    internal_data.insert(internal_data.begin(), begin + 1, end);

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
AreaInternalUse::emitJSON(nlohmann::json &j) {
    std::string hex;

    if (!present) {
        return;
    }

    if (internal_data.size() == 0) {
        // nothing to do
        return;
    }

    bytesToHexStr(internal_data, hex);
    j["data"] = hex;
}

void
AreaInternalUse::emitTOML(toml::table &t) {
    std::string hex;

    if (!present) {
        return;
    }

    if (internal_data.size() == 0) {
        // nothing to do
        return;
    }

    bytesToHexStr(internal_data, hex);
    t["data"] = toml::value(hex);
}

bool
AreaInternalUse::emitBinary(bytes &out_bin, Errs &errs) {
    UNUSED(errs);

    if (!present) {
        return true;
    }

    out_bin.emplace_back(DEFAULT_SECTION_HEADER_BYTE);

    out_bin.insert(out_bin.end(), internal_data.begin(), internal_data.end());

    uchar total  = getLength();
    // byte count of zero filled unused space
    uchar unused = IPMI_TO_REAL_LEN(total) - (1 + internal_data.size());
    out_bin.insert(out_bin.end(), unused, std::byte{0});

    return true;
}

//    #### ##    ## #### ########
//     ##  ###   ##  ##     ##
//     ##  ####  ##  ##     ##
//     ##  ## ## ##  ##     ##
//     ##  ##  ####  ##     ##
//     ##  ##   ###  ##     ##
//    #### ##    ## ####    ##

AreaInternalUse::AreaInternalUse() : Section("internal_use", "Internal Use Area") {
}
AreaInternalUse::~AreaInternalUse() {
}
