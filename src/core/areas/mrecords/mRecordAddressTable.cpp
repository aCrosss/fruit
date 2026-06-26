#include "mRecordAddressTable.hpp"

//    ##        #######   ######     ###    ##
//    ##       ##     ## ##    ##   ## ##   ##
//    ##       ##     ## ##        ##   ##  ##
//    ##       ##     ## ##       ##     ## ##
//    ##       ##     ## ##       ######### ##
//    ##       ##     ## ##    ## ##     ## ##
//    ########  #######   ######  ##     ## ########

void
MRecordAddressTable::debug_printOutVals() {
    std::cout << "=== " << label << " ===" << std::endl;
    std::cout << "record_id:       " << record_id << std::endl;
    std::cout << "picmg_record_id: " << picmg_record_id << std::endl;

    std::cout << "shelf_address {data=\'" << shelf_address.str << "'"
              << " encoding=" << encodingToString(shelf_address.enc) << "}" << std::endl;

    for (size_t i = 0; i < entries.size(); i++) {
        TableEntry &e = entries[i];
        std::cout << "enty[" << i << "]:" << std::endl;
        std::cout << "  hardware_address: " << SC_I(e.hardware_address) << std::endl;
        std::cout << "  site_number:      " << SC_I(e.site_number) << std::endl;
        std::cout << "  site_type:        " << SC_I(e.site_type) << std::endl;
    }
}

void
MRecordAddressTable::clear() {
    CLEAR_ENC_STR(shelf_address);
    entries.clear();
}

uchar
MRecordAddressTable::getLength() {
    uchar len  = PICMG_HEADER_LEN;
    len       += 31; // constant size shelf_address string
    len       += 1;  // entry count byte
    len       += static_cast<uchar>(entries.size() * 3);

    return len;
}

template <typename T>
bool
MRecordAddressTable::tryParseTableEntryImpl(T v, TableEntry &te, Errs &errs) {
    bool valid = true;

    uchar hardware_address;
    uchar site_number;
    uchar site_type;

    if (!tryParseField_uchar(v, "hardware_address", hardware_address, errs)) {
        valid = false;
    }

    if (!tryParseField_uchar(v, "site_number", site_number, errs)) {
        valid = false;
    }

    if (!tryParseField_uchar(v, "site_type", site_type, errs)) {
        valid = false;
    }

    if (!valid) {
        return false;
    }

    te.hardware_address = hardware_address;
    te.site_number      = site_number;
    te.site_type        = site_type;

    return true;
}

bool
MRecordAddressTable::tryParseTableEntry(biterator &begin, TableEntry &te, Errs &errs) {
    UNUSED(errs);

    te.hardware_address = DR_BYTE(begin + 0);
    te.site_number      = DR_BYTE(begin + 1);
    te.site_type        = DR_BYTE(begin + 2);

    begin += 3;
    return true;
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
MRecordAddressTable::tryParseImpl(T v, Errs &errs) {
    clear();
    bool valid = true;

    if (!tryParseField_encStr(v, "shelf_address", shelf_address, errs)) {
        valid = false;
    }

    T array;
    if (!tryParseField_arr(v, "entries", array, errs)) {
        return false;
    }

    for (size_t i = 0; i < array.size(); i++) {
        T          entry = array[i];
        TableEntry te;

        std::stringstream s;
        s << "slots[" << i << "]";
        if (!tryParseTableEntryImpl(entry, te, errs)) {
            valid = false;
            continue;
        }

        entries.push_back(te);
    }

    debug_printOutVals();
    return valid;
}

bool
MRecordAddressTable::tryParse(nlohmann::json j, Errs &errs) {
    clear();

    return tryParseImpl(j, errs);
}

bool
MRecordAddressTable::tryParse(toml::value &t, Errs &errs) {
    clear();

    return tryParseImpl(t, errs);
}

bool
MRecordAddressTable::tryParseBinary(biterator begin, biterator end, Errs &errs) {
    UNUSED(end);

    biterator b = begin + PICMG_HEADER_LEN;
    if (!tryDecodeStr(b, "shelf_address", shelf_address, errs)) {
        return false;
    }

    begin               += PICMG_HEADER_LEN;
    uchar entries_count  = DR_BYTE(begin + 31);

    begin += 32;
    for (uchar i = 0; i < entries_count; i++) {
        TableEntry te;
        if (!tryParseTableEntry(begin, te, errs)) {
            return false;
        }
        entries.push_back(te);
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
MRecordAddressTable::emitJSON(nlohmann::json &j) {
    j["record_id"]       = record_id;
    j["picmg_record_id"] = picmg_record_id;

    emitEncStr(j, "shelf_address", shelf_address);

    json jentries;
    for (auto &&e : entries) {
        json je;
        je["hardware_address"] = e.hardware_address;
        je["site_number"]      = e.site_number;
        je["site_type"]        = e.site_type;

        jentries.push_back(je);
    }

    j["entries"] = jentries;
}

void
MRecordAddressTable::emitTOML(toml::table &t) {
    t["record_id"]       = toml::value(SC_I(record_id));
    t["picmg_record_id"] = toml::value(SC_I(picmg_record_id));

    emitEncStr(t, "shelf_address", shelf_address);

    toml::array tentries;
    for (auto &&e : entries) {
        toml::table te;
        te["hardware_address"] = toml::value(e.hardware_address);
        te["site_number"]      = toml::value(e.site_number);
        te["site_type"]        = toml::value(e.site_type);

        tentries.push_back(toml::value(std::move(te)));
    }

    t["entries"] = toml::value(std::move(tentries));
}

bool
MRecordAddressTable::emitBinary(bytes &out_bin, Errs &errs) {
    bytes header;
    bytes payload;

    prependPICMGHeader(payload);

    bytes shelf_address_bs;
    if (!tryEncodeStr("shelf_address", shelf_address, shelf_address_bs, errs)) {
        return false;
    }

    // in Address Table record shelf addres bytes is fixed sized field
    // type/length byte + 30 bytes of bytes field should be 31
    // values of unused space bytes are undefined, we will use zeroes
    if (shelf_address_bs.size() < 31) {
        size_t len = 31 - shelf_address_bs.size();
        shelf_address_bs.insert(shelf_address_bs.end(), len, std::byte{0});
    }

    bytes entries_bs;
    uchar entries_count = static_cast<uchar>(entries.size());
    entries_bs.emplace_back(std::byte{entries_count});

    for (auto &&e : entries) {
        entries_bs.emplace_back(std::byte{e.hardware_address});
        entries_bs.emplace_back(std::byte{e.site_number});
        entries_bs.emplace_back(std::byte{e.site_type});
    }

    APPEND_BYTES(payload, shelf_address_bs);
    APPEND_BYTES(payload, entries_bs);

    buildMRecordHeader(header, false, payload);

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

MRecordAddressTable::MRecordAddressTable(/* args */)
    : MRecordBase("address_tabe", "Address Table") {
    record_id       = MRECORD_PICMG_RECORD;
    picmg_record_id = PICMGREC_ADDRESS_TABLE;
}

MRecordAddressTable::~MRecordAddressTable() {
}
