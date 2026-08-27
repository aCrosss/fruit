#include "mRecordFanMapping.hpp"

//    ##        #######   ######     ###    ##
//    ##       ##     ## ##    ##   ## ##   ##
//    ##       ##     ## ##        ##   ##  ##
//    ##       ##     ## ##       ##     ## ##
//    ##       ##     ## ##       ######### ##
//    ##       ##     ## ##    ## ##     ## ##
//    ########  #######   ######  ##     ## ########

void
MRecordFanMapping::debug_printOutVals() {
    std::cout << "=== " << label << " ===" << std::endl;
    std::cout << "record_id:       " << record_id << std::endl;
    std::cout << "picmg_record_id: " << picmg_record_id << std::endl;

    for (size_t i = 0; i < entries.size(); i++) {
        TableEntry &e = entries[i];
        std::cout << "enty[" << i << "]:" << std::endl;
        std::cout << "  hardware_address: " << SC_I(e.hardware_address) << std::endl;
        std::cout << "  fru_device_id:    " << SC_I(e.fru_device_id) << std::endl;
        std::cout << "  site_number:      " << SC_I(e.site_number) << std::endl;
        std::cout << "  site_type:        " << SC_I(e.site_type) << std::endl;
    }
}

void
MRecordFanMapping::clear() {
    entries.clear();
}

uchar
MRecordFanMapping::getLength() {
    uchar len  = PICMG_HEADER_LEN;
    len       += 1;                                      // 1 byte entry count
    len       += static_cast<uchar>(entries.size() * 4); // 4 bytes each entry

    return len;
}

template <typename T>
bool
MRecordFanMapping::tryParseEntry(T v, TableEntry &e, Errs &errs) {
    bool valid = true;

    uchar hardware_address;
    uchar fru_device_id;
    uchar site_number;
    uchar site_type;

    if (!tryParseField_uchar(v, "hardware_address", hardware_address, errs)) {
        valid = false;
    }

    if (!tryParseField_uchar(v, "fru_device_id", fru_device_id, errs)) {
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

    e.hardware_address = hardware_address;
    e.fru_device_id    = fru_device_id;
    e.site_number      = site_number;
    e.site_type        = site_type;

    return true;
}

bool
MRecordFanMapping::tryParseEntry(biterator begin, TableEntry &e, Errs &errs) {
    UNUSED(errs);

    e.hardware_address = DR_BYTE(begin + 0);
    e.fru_device_id    = DR_BYTE(begin + 1);
    e.site_number      = DR_BYTE(begin + 2);
    e.site_type        = DR_BYTE(begin + 3);

    begin += 4;
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
bool
MRecordFanMapping::tryParseImpl(T v, Errs &errs) {
    clear();
    bool valid = true;

    T array;
    if (!tryParseField_arr(v, "entries", array, errs)) {
        return false;
    }

    for (size_t i = 0; i < array.size(); i++) {
        T          entry = array[i];
        TableEntry te;

        std::stringstream s;
        s << "slots[" << i << "]";
        if (!tryParseEntry(entry, te, errs)) {
            valid = false;
            continue;
        }

        entries.push_back(te);
    }

    debug_printOutVals();
    return valid;
}

bool
MRecordFanMapping::tryParse(nlohmann::json j, Errs &errs) {
    clear();

    return tryParseImpl(j, errs);
}

bool
MRecordFanMapping::tryParse(toml::value &t, Errs &errs) {
    clear();

    return tryParseImpl(t, errs);
}

bool
MRecordFanMapping::tryParseBinary(biterator begin, biterator end, Errs &errs) {
    UNUSED(end);

    begin               += PICMG_HEADER_LEN;
    uchar entries_count  = DR_BYTE(begin++);

    for (uchar i = 0; i < entries_count; i++) {
        TableEntry te;
        if (!tryParseEntry(begin, te, errs)) {
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
MRecordFanMapping::emitJSON(nlohmann::json &j) {
    j["record_id"]       = record_id;
    j["picmg_record_id"] = picmg_record_id;

    json jentries;
    for (auto &&e : entries) {
        json je;
        je["hardware_address"] = e.hardware_address;
        je["fru_device_id"]    = e.fru_device_id;
        je["site_number"]      = e.site_number;
        je["site_type"]        = e.site_type;

        jentries.push_back(je);
    }

    j["entries"] = jentries;
}

void
MRecordFanMapping::emitTOML(toml::table &t) {
    t["record_id"]       = toml::value(SC_I(record_id));
    t["picmg_record_id"] = toml::value(SC_I(picmg_record_id));

    toml::array tentries;
    for (auto &&e : entries) {
        toml::table te;
        te["hardware_address"] = toml::value(e.hardware_address);
        te["fru_device_id"]    = toml::value(e.fru_device_id);
        te["site_number"]      = toml::value(e.site_number);
        te["site_type"]        = toml::value(e.site_type);

        tentries.push_back(toml::value(std::move(te)));
    }

    t["entries"] = toml::value(std::move(tentries));
}

bool
MRecordFanMapping::emitBinary(bytes &out_bin, Errs &errs) {
    UNUSED(errs);

    bytes header;
    bytes payload;

    prependPICMGHeader(payload);

    uchar entries_count = static_cast<uchar>(entries.size());
    payload.emplace_back(std::byte{entries_count});

    for (auto &&e : entries) {
        payload.emplace_back(std::byte{e.hardware_address});
        payload.emplace_back(std::byte{e.fru_device_id});
        payload.emplace_back(std::byte{e.site_number});
        payload.emplace_back(std::byte{e.site_type});
    }

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

MRecordFanMapping::MRecordFanMapping()
    : MRecordBase("shelf_fan_geography", "Shelf Fan Geography") {
    record_id       = MRECORD_PICMG_RECORD;
    picmg_record_id = PICMGREC_FAN_GEOGRAPHY;
}
