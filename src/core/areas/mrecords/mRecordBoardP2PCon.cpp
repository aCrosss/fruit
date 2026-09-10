#include "mRecordBoardP2PCon.hpp"

//    ##        #######   ######     ###    ##
//    ##       ##     ## ##    ##   ## ##   ##
//    ##       ##     ## ##        ##   ##  ##
//    ##       ##     ## ##       ##     ## ##
//    ##       ##     ## ##       ######### ##
//    ##       ##     ## ##    ## ##     ## ##
//    ########  #######   ######  ##     ## ########

std::string
MRecordBoardP2PCon::IFaceToStr(Interface iface) {
    switch (iface) {
    case IFACE_BASE    : return "base";
    case IFACE_FABRIC  : return "fabric";
    case IFACE_UCHANNEL: return "update_channel";

    default: return "undefined";
    }
}

void
MRecordBoardP2PCon::debug_printOutVals() {
    std::cout << "=== " << label << " ===" << std::endl;
    std::cout << "record_id:       " << record_id << std::endl;
    std::cout << "picmg_record_id: " << picmg_record_id << std::endl;

    for (size_t i = 0; i < guids.size(); i++) {
        std::string guid_str;
        GUIDToStr(guids[i], guid_str);
        std::cout << "guids[" << i << "]: " << guid_str << std::endl;
    }

    for (size_t i = 0; i < link_descriptors.size(); i++) {
        LinkDescriptor &ld = link_descriptors[i];
        std::cout << "link_descriptors[" << i << "]:" << std::endl;

        std::cout << "  link_grouping_id:    " << static_cast<int>(ld.link_grouping_id)
                  << std::endl;
        std::cout << "  link_type_extension: " << static_cast<int>(ld.link_type_extension)
                  << std::endl;
        std::cout << "  link_type:           " << static_cast<int>(ld.link_type) << std::endl;
        std::cout << "  link_designator:" << std::endl;
        // LinkDesignator &ld = ld.link_designator;
        std::cout << "    port_3:         " << (ld.port_3 ? "included" : "excluded")
                  << std::endl;
        std::cout << "    port_2:         " << (ld.port_2 ? "included" : "excluded")
                  << std::endl;
        std::cout << "    port_1:         " << (ld.port_1 ? "included" : "excluded")
                  << std::endl;
        std::cout << "    port_0:         " << (ld.port_0 ? "included" : "excluded")
                  << std::endl;
        std::cout << "    interface:      " << IFaceToStr(ld.iface) << std::endl;
        std::cout << "    channel_number: " << static_cast<int>(ld.ch_number) << std::endl;
    }
}

void
MRecordBoardP2PCon::clear() {
    guids.clear();
    link_descriptors.clear();
}

uchar
MRecordBoardP2PCon::getLength() {
    uchar len  = PICMG_HEADER_LEN;
    len       += 1; // guid count byte
    len       += static_cast<uchar>(guids.size() * 16);
    len       += static_cast<uchar>(link_descriptors.size() * 4);

    return len;
}

//  #      # #    # #    #    #####  ######  ####   ####  #####  # #####  #####  ####  #####
//  #      # ##   # #   #     #    # #      #      #    # #    # # #    #   #   #    # #    #
//  #      # # #  # ####      #    # #####   ####  #      #    # # #    #   #   #    # #    #
//  #      # #  # # #  #      #    # #           # #      #####  # #####    #   #    # #####
//  #      # #   ## #   #     #    # #      #    # #    # #   #  # #        #   #    # #   #
//  ###### # #    # #    #    #####  ######  ####   ####  #    # # #        #    ####  #    #

template <typename T>
bool
MRecordBoardP2PCon::tryParseLDescriptor(T v, LinkDescriptor &ld, Errs &errs) {
    bool valid = true;

    if (!tryParseField_uchar(v, "link_grouping_id", ld.link_grouping_id, errs)) {
        valid = false;
    }

    if (!tryParseField_uchar(v, "link_type_extension", ld.link_type_extension, errs)) {
        valid = false;
    }

    if (!tryParseField_uchar(v, "link_type", ld.link_type, errs)) {
        valid = false;
    }

    if (!tryParseField_bool(v, "port_3", ld.port_3, errs)) {
        valid = false;
    }

    if (!tryParseField_bool(v, "port_2", ld.port_2, errs)) {
        valid = false;
    }

    if (!tryParseField_bool(v, "port_1", ld.port_1, errs)) {
        valid = false;
    }

    if (!tryParseField_bool(v, "port_0", ld.port_0, errs)) {
        valid = false;
    }

    std::string iface;
    if (tryParseField_str(v, "interface", iface, errs)) {
        if (iface.compare("base") == 0) {
            ld.iface = IFACE_BASE;
        } else if (iface.compare("fabric") == 0) {
            ld.iface = IFACE_FABRIC;
        } else if (iface.compare("update_channel") == 0) {
            ld.iface = IFACE_UCHANNEL;
        } else {
            std::stringstream ss;
            ss << "unknown interface \"" << iface << "\"";
            errs.append(tag, "interface", ss.str());
            valid = false;
        }
    } else {
        valid = false;
    }

    uchar channel_number;
    if (!tryParseField_uchar(v, "channel_number", channel_number, errs)) {
        valid = false;
    }

    switch (ld.iface) {
    case IFACE_BASE    : CHECK_BOUNDS(channel_number, 0x01, 0x10, errs); break;
    case IFACE_FABRIC  : CHECK_BOUNDS(channel_number, 0x01, 0x0F, errs); break;
    case IFACE_UCHANNEL: CHECK_BOUNDS(channel_number, 0x01, 0x07, errs); break;

    // iface wasn't handled correctly
    default: return false;
    }

    ld.ch_number = channel_number;

    return valid;
}

bool
MRecordBoardP2PCon::tryParseLDescriptor(ibytes begin, LinkDescriptor &ld, Errs &errs) {
    UNUSED(errs);

    // lsb first
    int n = DR_BYTE(begin + 0) << 0 | DR_BYTE(begin + 1) << 8 | DR_BYTE(begin + 2) << 16 |
            DR_BYTE(begin + 3) << 24;

    // 5:0
    ld.ch_number = n & MASK_5b;
    n            = n >> 6;

    // 7:6
    ld.iface = Interface(n & MASK_2b);
    n        = n >> 2;

    // 8
    ld.port_0 = bool(n & MASK_1b);
    n         = n >> 1;

    // 9
    ld.port_1 = bool(n & MASK_1b);
    n         = n >> 1;

    // 10
    ld.port_2 = bool(n & MASK_1b);
    n         = n >> 1;

    // 11
    ld.port_3 = bool(n & MASK_1b);
    n         = n >> 1;

    // 19:12
    ld.link_type = n & MASK_8b;
    n            = n >> 8;

    // 23:20
    ld.link_type_extension = n & MASK_4b;
    n                      = n >> 4;

    // 31:24
    ld.link_grouping_id = n & MASK_8b;

    return true;
}

void
MRecordBoardP2PCon::emitLDescriptor(nlohmann::json &j, LinkDescriptor &ld) {
    j["link_grouping_id"]    = ld.link_grouping_id;
    j["link_type_extension"] = ld.link_type_extension;
    j["link_type"]           = ld.link_type;
    j["port_3"]              = ld.port_3;
    j["port_2"]              = ld.port_2;
    j["port_1"]              = ld.port_1;
    j["port_0"]              = ld.port_0;
    j["interface"]           = IFaceToStr(ld.iface);
    j["channel_number"]      = ld.ch_number;
}

void
MRecordBoardP2PCon::emitLDescriptor(toml::table &t, LinkDescriptor &ld) {
    t["link_grouping_id"]    = toml::value(ld.link_grouping_id);
    t["link_type_extension"] = toml::value(ld.link_type_extension);
    t["link_type"]           = toml::value(ld.link_type);
    t["port_3"]              = toml::value(ld.port_3);
    t["port_2"]              = toml::value(ld.port_2);
    t["port_1"]              = toml::value(ld.port_1);
    t["port_0"]              = toml::value(ld.port_0);
    t["interface"]           = toml::value(IFaceToStr(ld.iface));
    t["channel_number"]      = toml::value(ld.ch_number);
}

void
MRecordBoardP2PCon::emitLDescriptor(bytes &out_bin, LinkDescriptor &ld) {
    int b  = 0;
    b     |= ld.ch_number & MASK_5b;
    b     |= (static_cast<int>(ld.iface) & MASK_2b) << 6;
    out_bin.emplace_back(std::byte{static_cast<uchar>(b)});
    // first byte out

    b  = 0;
    b |= (static_cast<int>(ld.port_0) & MASK_1b) << 0;
    b |= (static_cast<int>(ld.port_1) & MASK_1b) << 1;
    b |= (static_cast<int>(ld.port_2) & MASK_1b) << 2;
    b |= (static_cast<int>(ld.port_3) & MASK_1b) << 3;
    b |= (ld.link_type & MASK_4b) << 4;
    out_bin.emplace_back(std::byte{static_cast<uchar>(b)});
    // second byte out

    b  = 0;
    b |= (ld.link_type >> 4) & MASK_4b;
    b |= (ld.link_type_extension & MASK_4b) << 4;
    out_bin.emplace_back(std::byte{static_cast<uchar>(b)});
    // third byte out

    out_bin.emplace_back(std::byte{ld.link_grouping_id});
    // fourth byte out, we're done
}

//   ####  #    # # #####
//  #    # #    # # #    #
//  #      #    # # #    #
//  #  ### #    # # #    #
//  #    # #    # # #    #
//   ####   ####  # #####

bool
MRecordBoardP2PCon::strToGUID(std::string s, bytes &guid, std::string err) {
    // guid - 16 bytes - 32 chars w/o '-' symbols at minimum
    if (s.length() < GUID_BYTE_LEN * 2) {
        err = "not enough hex symbols in GUID string";
        return false;
    }

    if (!hexStrToBytes(s, guid, err)) {
        return false;
    }

    if (guid.size() != GUID_BYTE_LEN) {
        err = "not enough bytes in GUID";
        return false;
    }

    return true;
}

void
MRecordBoardP2PCon::GUIDToStr(bytes &guid, std::string &out) {
    std::string hex;
    bytesToHexStr(guid, hex);

    hex.insert(hex.begin() + 20, '-');
    hex.insert(hex.begin() + 16, '-');
    hex.insert(hex.begin() + 12, '-');
    hex.insert(hex.begin() + 8, '-');
    out = hex;
}

//    ########     ###    ########   ######  #### ##    ##  ######
//    ##     ##   ## ##   ##     ## ##    ##  ##  ###   ## ##    ##
//    ##     ##  ##   ##  ##     ## ##        ##  ####  ## ##
//    ########  ##     ## ########   ######   ##  ## ## ## ##   ####
//    ##        ######### ##   ##         ##  ##  ##  #### ##    ##
//    ##        ##     ## ##    ##  ##    ##  ##  ##   ### ##    ##
//    ##        ##     ## ##     ##  ######  #### ##    ##  ######

inline std::string
strFromObj(nlohmann::json j) {
    return j.get<std::string>();
}

inline std::string
strFromObj(toml::value &t) {
    return t.as_string();
}

template <typename T>
bool
MRecordBoardP2PCon::tryParseImpl(T v, Errs &errs) {
    bool valid = true;

    T guids_array;
    if (!tryParseField_arr(v, "guids", guids_array, errs)) {
        return false;
    }

    for (size_t i = 0; i < guids_array.size(); i++) {
        std::string guid_str = strFromObj(guids_array[i]);

        std::string err;
        bytes       guid;
        if (!strToGUID(guid_str, guid, err)) {
            valid = false;
            continue;
        } else {
            guids.push_back(guid);
        }
    }

    T ld_array;
    if (!tryParseField_arr(v, "link_descriptors", ld_array, errs)) {
        return false;
    }

    for (size_t i = 0; i < ld_array.size(); i++) {
        T              entry = ld_array[i];
        LinkDescriptor ld;

        std::stringstream s;
        s << "link_descriptors[" << i << "]";
        if (!tryParseLDescriptor(entry, ld, errs)) {
            valid = false;
            continue;
        }

        link_descriptors.push_back(ld);
    }

    debug_printOutVals();
    return valid;
}

bool
MRecordBoardP2PCon::tryParse(nlohmann::json j, Errs &errs) {
    return tryParseImpl(j, errs);
}

bool
MRecordBoardP2PCon::tryParse(toml::value &t, Errs &errs) {
    return tryParseImpl(t, errs);
}

bool
MRecordBoardP2PCon::tryParseBinary(ibytes begin, ibytes end, Errs &errs) {
    begin += PICMG_HEADER_LEN;

    uchar count = DR_BYTE(begin++);
    for (uchar i = 0; i < count; i++) {
        bytes guid;

        OUT_OF_BOUNDS_GUARD_OFFSET("guids", GUID_BYTE_LEN);
        guid.insert(guid.begin(), begin, begin + GUID_BYTE_LEN);
        guids.push_back(guid);

        begin += GUID_BYTE_LEN;
        OUT_OF_BOUNDS_GUARD("guids")
    }

    count = end - begin;
    if (count % LINK_DESCR_BYTE_LEN != 0) {
        errs.append(
            tag,
            "link_descriptors",
            "leftover bytes count is not multiple of 4: can't divide into link descriptors");
        return false;
    }

    for (uchar i = 0; i < count / LINK_DESCR_BYTE_LEN; i++) {
        LinkDescriptor ld;
        if (!tryParseLDescriptor(begin, ld, errs)) {
            return false;
        }

        begin += LINK_DESCR_BYTE_LEN;
        OUT_OF_BOUNDS_GUARD("link_descriptors")
        link_descriptors.push_back(ld);
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
MRecordBoardP2PCon::emitJSON(nlohmann::json &j) {
    j["record_id"]       = record_id;
    j["picmg_record_id"] = picmg_record_id;

    json jguids;
    for (auto &&i : guids) {
        std::string str;
        GUIDToStr(i, str);
        jguids.push_back(str);
    }
    j["guids"] = jguids;

    json array;
    for (auto &&i : link_descriptors) {
        json entry;
        emitLDescriptor(entry, i);
        array.push_back(entry);
    }

    j["link_descriptors"] = array;
}

void
MRecordBoardP2PCon::emitTOML(toml::table &t) {
    t["record_id"]       = toml::value(SC_I(record_id));
    t["picmg_record_id"] = toml::value(SC_I(picmg_record_id));

    toml::array tguids;
    for (auto &&i : guids) {
        std::string str;
        GUIDToStr(i, str);
        tguids.push_back(toml::value(str));
    }
    t["guids"] = toml::value(std::move(tguids));

    toml::array array;
    for (auto &&i : link_descriptors) {
        toml::table entry;
        emitLDescriptor(entry, i);
        array.push_back(entry);
    }

    t["link_descriptors"] = toml::value(std::move(array));
}

bool
MRecordBoardP2PCon::emitBinary(bytes &out_bin, bool eol, Errs &errs) {
    UNUSED(errs);

    bytes header;
    bytes payload;

    prependPICMGHeader(payload);

    payload.emplace_back(std::byte{static_cast<uchar>(guids.size())});
    for (auto &&i : guids) {
        APPEND_BYTES(payload, i);
    }

    for (auto &&i : link_descriptors) {
        bytes bs;
        emitLDescriptor(bs, i);
        APPEND_BYTES(payload, bs);
    }

    buildMRecordHeader(header, eol, payload);

    APPEND_BYTES(out_bin, header);
    APPEND_BYTES(out_bin, payload);

    return true;
}

//    #### ##    ## #### ########
//     ##  ###   ##  ##     ##
//     ##  ####  ##  ##     ##
//     ##  ## ## ##  ##     ##
//     ##  ##  ####  ##     ##
//     ##  ##   ###  ##     ##
//    #### ##    ## ####    ##

MRecordBoardP2PCon::MRecordBoardP2PCon()
    : MRecordBase("board_p2p_con", "Board Point-to-Point Connectivity") {
    record_id       = MRECORD_PICMG_RECORD;
    picmg_record_id = PICMGREC_BOARD_P2PCON;
}
