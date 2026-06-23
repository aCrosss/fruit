#include <iostream>

#include "areaCommonHeader.hpp"

//    ##        #######   ######     ###    ##
//    ##       ##     ## ##    ##   ## ##   ##
//    ##       ##     ## ##        ##   ##  ##
//    ##       ##     ## ##       ##     ## ##
//    ##       ##     ## ##       ######### ##
//    ##       ##     ## ##    ## ##     ## ##
//    ########  #######   ######  ##     ## ########

// Common Header, Internal Use, Chassis Info, Board, Product Info, MultiRecord
#define AREAS_COUNT   6
// made for offsets calculations, doesn't include common header itself
// i.e. goes from index 1 to total areas count
#define LOOP_AREAS    for (size_t i = 1; i < AREAS_COUNT; i++)
// because we skipped common header area offset to area index is i - 1
#define OFFSET_IND(i) (i - 1)
//
#define LOOP_OFFSETS  for (size_t i = 0; i < AREAS_COUNT - 1; i++)

void
AreaCommonHeader::debug_printOutVals() {
    std::cout << "=== " << label << " ===" << std::endl;
    std::cout << "internal_use: " << static_cast<int>(offsets[0]) << std::endl;
    std::cout << "chassis:      " << static_cast<int>(offsets[1]) << std::endl;
    std::cout << "board:        " << static_cast<int>(offsets[2]) << std::endl;
    std::cout << "product_info: " << static_cast<int>(offsets[3]) << std::endl;
    std::cout << "multi_record: " << static_cast<int>(offsets[4]) << std::endl;
}

uchar
AreaCommonHeader::getLength() {
    // length of common header in constant
    return uchar(1);
}

void
AreaCommonHeader::clear() {
    for (size_t i = 0; i < offsets.size(); i++) {
        offsets[i] = 0;
    }
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
    LOOP_AREAS {
        auto &&section = sections[i];

        if (section->isPresent()) {
            offsets[OFFSET_IND(i)]  = base;
            base                   += section->getLength();
        } else {
            offsets[OFFSET_IND(i)] = 0;
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
AreaCommonHeader::tryParse(nlohmann::json j, Errs &errs) {
    // common header generating automaticaly
    UNUSED(j);
    UNUSED(errs);
    return true;
}

bool
AreaCommonHeader::tryParse(toml::value &t, Errs &errs) {
    // common header generating automaticaly
    UNUSED(t);
    UNUSED(errs);
    return true;
}

bool
AreaCommonHeader::tryParseBinary(biterator begin, biterator end, Errs &errs) {
    UNUSED(end);

    clear();

    if (!checkChecksums(begin + const_len - 1, begin, begin + const_len - 2, errs)) {
        return false;
    }

    offsets.resize(AREAS_COUNT);

    begin++;
    LOOP_OFFSETS {
        offsets[i] = static_cast<uchar>(*(begin + i));
    }

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
    LOOP_OFFSETS {
        bs.emplace_back(std::byte{offsets[i]});
    }
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
