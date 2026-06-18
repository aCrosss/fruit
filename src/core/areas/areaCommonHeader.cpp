#include <iostream>

#include "areaCommonHeader.hpp"

//    ##        #######   ######     ###    ##
//    ##       ##     ## ##    ##   ## ##   ##
//    ##       ##     ## ##        ##   ##  ##
//    ##       ##     ## ##       ##     ## ##
//    ##       ##     ## ##       ######### ##
//    ##       ##     ## ##    ## ##     ## ##
//    ########  #######   ######  ##     ## ########

void
AreaCommonHeader::debug_printOutVals() {
    std::cout << "=== " << label << " ===" << std::endl;
    std::cout << "internal_use: " << static_cast<int>(offsets[0]) << std::endl;
    std::cout << "chassis:      " << static_cast<int>(offsets[1]) << std::endl;
    std::cout << "board:        " << static_cast<int>(offsets[2]) << std::endl;
    std::cout << "product_info: " << static_cast<int>(offsets[3]) << std::endl;
}

uchar
AreaCommonHeader::getLength() {
    // length of common header in constant
    return uchar(1);
}

void
AreaCommonHeader::clear() {
    offsets.clear();
    offsets.insert(offsets.begin(), 2, 0);
}

uchar
AreaCommonHeader::getOffset(Areas offset_ind) {
    if (offset_ind >= offsets.size()) {
        return 0;
    }

    return offsets[offset_ind];
}

uchar
AreaCommonHeader::getNextOffset(Areas offset_ind) {
    uchar ind = static_cast<uchar>(offset_ind) - 1;
    if (ind >= offsets.size()) {
        return 0;
    }

    for (uchar i = ind + 1; i < offsets.size(); i++) {
        if (offsets[i] > 0) {
            return offsets[i];
        }
    }

    return 0;
}

bool
AreaCommonHeader::setOffsets(std::vector<std::unique_ptr<Section>> &sections) {
    offsets.resize(sections.size());

    uchar base = 1;
    // skip itself
    for (size_t i = 1; i < sections.size(); i++) {
        auto &&section = sections[i];

        if (section->isPresent()) {
            offsets[i - 1]  = base;
            base           += section->getLength();
        } else {
            offsets[i - 1] = 0;
        }
    }

    return true;
}

//    ########     ###    ########   ######  #### ##    ##  ######
//    ##     ##   ## ##   ##     ## ##    ##  ##  ###   ## ##    ##
//    ##     ##  ##   ##  ##     ## ##        ##  ####  ## ##
//    ########  ##     ## ########   ######   ##  ## ## ## ##   ####
//    ##        ######### ##   ##         ##  ##  ##  #### ##    ##
//    ##        ##     ## ##    ##  ##    ##  ##  ##   ### ##    ##
//    ##        ##     ## ##     ##  ######  #### ##    ##  ######

// template <typename T>
// inline bool
// AreaCommonHeader::tryParseImpl(T v, Errs &errs) {
//     return false;
// }

bool
AreaCommonHeader::tryParseJSON(nlohmann::json j, Errs &errs) {
    // common header generating automaticaly
    UNUSED(j);
    UNUSED(errs);
    return true;
}

bool
AreaCommonHeader::tryParseTOML(toml::value &t, Errs &errs) {
    // common header generating automaticaly
    UNUSED(t);
    UNUSED(errs);
    return true;
}

bool
AreaCommonHeader::tryParseBinary(biterator begin, biterator end, Errs &errs) {
    UNUSED(end);

    clear();

    if (!checkChecksums(begin + const_len - 1, begin, begin + const_len - 1, errs)) {
        return false;
    }

    // TODO: rework/remove this line
    offsets.resize(4);

    offsets[0] = static_cast<uchar>(*(begin + 1)); // internal use area
    offsets[1] = static_cast<uchar>(*(begin + 2)); // chassis info area
    offsets[2] = static_cast<uchar>(*(begin + 3)); // board area
    offsets[3] = static_cast<uchar>(*(begin + 4)); // product info area

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
AreaCommonHeader::emitJSON(nlohmann::json &j) {
    // common header doesn't emit json
    UNUSED(j);
    return;
}

void
AreaCommonHeader::emitTOML(toml::table &t) {
    // common header doesn't emit toml
    UNUSED(t);
    return;
}

bool
AreaCommonHeader::emitBinary(bytes &out_bin, Errs &errs) {
    UNUSED(errs);

    bytes bs;

    // header
    bs.emplace_back(DEFAULT_SECTION_HEADER_BYTE);

    // TODO: rework this when all areas will be implemented
    bs.emplace_back(std::byte{offsets[0]});
    bs.emplace_back(std::byte{offsets[1]});
    bs.emplace_back(std::byte{offsets[2]});
    bs.emplace_back(std::byte{offsets[3]});
    // multirecords areas NIY
    bs.emplace_back(std::byte{0});
    // PAD, 0x00
    bs.emplace_back(std::byte{0});
    bs.emplace_back(calcZeroChecksum(bs));

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

AreaCommonHeader::AreaCommonHeader() : Section("common_header", "Common Header Area") {
    //
}

AreaCommonHeader::~AreaCommonHeader() {
    //
}
