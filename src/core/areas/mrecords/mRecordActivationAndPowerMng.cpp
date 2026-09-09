#include "mRecordActivationAndPowerMng.hpp"
#include "types.hpp"

//    ##        #######   ######     ###    ##
//    ##       ##     ## ##    ##   ## ##   ##
//    ##       ##     ## ##        ##   ##  ##
//    ##       ##     ## ##       ##     ## ##
//    ##       ##     ## ##       ######### ##
//    ##       ##     ## ##    ## ##     ## ##
//    ########  #######   ######  ##     ## ########

void
MRecordActivationAndPowerMng::debug_printOutVals() {
    std::cout << "=== " << label << " ===" << std::endl;
    std::cout << "record_id:       " << record_id << std::endl;
    std::cout << "picmg_record_id: " << picmg_record_id << std::endl;
    std::cout << "allowance_for_activation: " << SC_I(allowance_for_activation) << std::endl;

    for (size_t i = 0; i < entries.size(); i++) {
        ActivationAndPwrDescr &e = entries[i];
        std::cout << "entry[" << i << "]:" << std::endl;
        std::cout << "  hardware_address:        " << SC_I(e.hardware_address) << std::endl;
        std::cout << "  fru_device_id:           " << SC_I(e.fru_device_id) << std::endl;
        std::cout << "  max_fru_power_cap:       " << e.max_fru_power_cap << std::endl;
        std::cout << "  controlled_deactivation: "
                  << (e.controlled_deactivation ? "enabled" : "disabled") << std::endl;
        std::cout << "  controlled_activation:   "
                  << (e.controlled_activation ? "enabled" : "disabled") << std::endl;
        std::cout << "  next_power_on_delay:     " << SC_I(e.next_power_on_delay) << std::endl;
    }
}

void
MRecordActivationAndPowerMng::clear() {
    allowance_for_activation = 0;
    entries.clear();
}

uchar
MRecordActivationAndPowerMng::getLength() {
    uchar len  = PICMG_HEADER_LEN;
    len       += 1; // allowance for FRU activation readines byte
    len       += 1; // descriptor count byte
    // fru activation and power descriptors: 5 bytes each
    len       += static_cast<uchar>(entries.size() * 5);

    return len;
}

template <typename T>
bool
MRecordActivationAndPowerMng::tryParseDescrImpl(T v, ActivationAndPwrDescr &d, Errs &errs) {
    bool valid = true;

    uchar hardware_address;
    uchar fru_device_id;
    int   max_fru_power_cap;
    bool  controlled_deactivation;
    bool  controlled_activation;
    int   next_power_on_delay;

    if (!tryParseField_uchar(v, "hardware_address", hardware_address, errs)) {
        valid = false;
    }

    if (!tryParseField_uchar(v, "fru_device_id", fru_device_id, errs)) {
        valid = false;
    }

    if (!tryParseField_int(v, "max_fru_power_cap", max_fru_power_cap, errs)) {
        valid = false;
    } else {
        // only 2 bytes value
        if (max_fru_power_cap < 0 || max_fru_power_cap > 65535) {
            errs.append(tag, "max_fru_power_cap", "expected [0-65535] byte value");
            valid = false;
        }
    }

    if (!tryParseField_bool(v, "controlled_deactivation", controlled_deactivation, errs)) {
        valid = false;
    }

    if (!tryParseField_bool(v, "controlled_activation", controlled_activation, errs)) {
        valid = false;
    }

    if (!tryParseField_int(v, "next_power_on_delay", next_power_on_delay, errs)) {
        valid = false;
    } else {
        // only 5 bits value
        if (next_power_on_delay < 0 || next_power_on_delay > 31) {
            errs.append(tag, "next_power_on_delay", "expected [0-255] byte value");
            valid = false;
        }
    }

    if (!valid) {
        return false;
    }

    d.hardware_address        = hardware_address;
    d.fru_device_id           = fru_device_id;
    d.max_fru_power_cap       = static_cast<unsigned short>(max_fru_power_cap);
    d.controlled_deactivation = controlled_deactivation;
    d.controlled_activation   = controlled_activation;
    d.next_power_on_delay     = static_cast<uchar>(next_power_on_delay);

    return true;
}

bool
MRecordActivationAndPowerMng::tryParseDescr(biterator             &begin,
                                            ActivationAndPwrDescr &d,
                                            Errs                  &errs) {
    UNUSED(errs);

    d.hardware_address = DR_BYTE(begin + 0);
    d.fru_device_id    = DR_BYTE(begin + 1);

    d.max_fru_power_cap = DR_BYTE(begin + 2) | (DR_BYTE(begin + 3) << 8);

    uchar b = DR_BYTE(begin + 4);

    d.controlled_deactivation = bool(!(b & 128));
    d.controlled_activation   = bool(b & 64);
    d.next_power_on_delay     = static_cast<uchar>(b & MASK_5b);

    // alway 5 bytes long, move iterator accordingly
    begin += 5;
    return true;
}

void
MRecordActivationAndPowerMng::emitEntry(bytes &out_bin, ActivationAndPwrDescr &entry) {
    out_bin.emplace_back(std::byte{entry.hardware_address});
    out_bin.emplace_back(std::byte{entry.fru_device_id});

    unsigned int pcap = entry.max_fru_power_cap;
    out_bin.emplace_back(std::byte{static_cast<uchar>(pcap & 0xFF)});
    out_bin.emplace_back(std::byte{static_cast<uchar>((pcap >> 8) & 0xFF)});

    uchar b  = entry.next_power_on_delay & MASK_5b;
    b       |= entry.controlled_activation << 6;
    b       |= !entry.controlled_deactivation << 7;
    out_bin.emplace_back(std::byte{b});
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
MRecordActivationAndPowerMng::tryParseImpl(T v, Errs &errs) {
    bool valid = true;

    if (!tryParseField_uchar(v, "allowance_for_activation", allowance_for_activation, errs)) {
        valid = false;
    }

    T array;
    if (!tryParseField_arr(v, "entries", array, errs)) {
        return false;
    }

    for (size_t i = 0; i < array.size(); i++) {
        T                     entry = array[i];
        ActivationAndPwrDescr d;

        std::stringstream s;
        s << "slots[" << i << "]";
        if (!tryParseDescrImpl(entry, d, errs)) {
            valid = false;
            continue;
        }

        entries.push_back(d);
    }

    debug_printOutVals();
    return valid;
}

bool
MRecordActivationAndPowerMng::tryParse(nlohmann::json j, Errs &errs) {
    return tryParseImpl(j, errs);
}

bool
MRecordActivationAndPowerMng::tryParse(toml::value &t, Errs &errs) {
    return tryParseImpl(t, errs);
}

bool
MRecordActivationAndPowerMng::tryParseBinary(biterator begin, biterator end, Errs &errs) {
    UNUSED(end);

    begin += PICMG_HEADER_LEN;

    allowance_for_activation = DR_BYTE(begin++);

    uchar count = DR_BYTE(begin++);
    for (uchar i = 0; i < count; i++) {
        if (begin > end) {
            errs.append(tag, "common", " binary parsing out of bounds");
            return false;
        }

        ActivationAndPwrDescr d;
        if (!tryParseDescr(begin, d, errs)) {
            return false;
        }
        entries.push_back(d);
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
MRecordActivationAndPowerMng::emitJSON(nlohmann::json &j) {
    j["record_id"]       = record_id;
    j["picmg_record_id"] = picmg_record_id;

    j["allowance_for_activation"] = allowance_for_activation;

    json jentries;
    for (auto &&m : entries) {
        json jentry;
        jentry["hardware_address"]        = m.hardware_address;
        jentry["fru_device_id"]           = m.fru_device_id;
        jentry["max_fru_power_cap"]       = m.max_fru_power_cap;
        jentry["controlled_deactivation"] = m.controlled_deactivation;
        jentry["controlled_activation"]   = m.controlled_activation;
        jentry["next_power_on_delay"]     = m.next_power_on_delay;
        jentries.push_back(jentry);
    }

    j["entries"] = jentries;
}

void
MRecordActivationAndPowerMng::emitTOML(toml::table &t) {
    t["record_id"]       = toml::value(SC_I(record_id));
    t["picmg_record_id"] = toml::value(SC_I(picmg_record_id));

    t["allowance_for_activation"] = toml::value(allowance_for_activation);

    toml::array tentries;
    for (auto &&m : entries) {
        toml::table tentry;
        tentry["hardware_address"]        = toml::value(m.hardware_address);
        tentry["fru_device_id"]           = toml::value(m.fru_device_id);
        tentry["max_fru_power_cap"]       = toml::value(m.max_fru_power_cap);
        tentry["controlled_deactivation"] = toml::value(m.controlled_deactivation);
        tentry["controlled_activation"]   = toml::value(m.controlled_activation);
        tentry["next_power_on_delay"]     = toml::value(m.next_power_on_delay);
        tentries.push_back(toml::value(std::move(tentry)));
    }

    t["entries"] = toml::value(std::move(tentries));
}

bool
MRecordActivationAndPowerMng::emitBinary(bytes &out_bin, bool eol, Errs &errs) {
    UNUSED(errs);

    if (entries.size() == 0) {
        return true;
    }

    bytes header;
    bytes payload;
    bytes tmp;
    bytes tmppl;

    size_t i         = 0;
    uchar  out_count = 0; // count of power feed in current record

    while (i < entries.size()) {
        size_t len = tmp.size() + tmppl.size() + MRECORD_HEADER_LEN_PICMG + /*entry count*/ 1 +
                     /*allowance*/ 1;
        if (len >= MAX_AREA_LEN) {
            prependPICMGHeader(payload);
            payload.emplace_back(std::byte{allowance_for_activation});
            payload.emplace_back(std::byte{static_cast<uchar>(out_count - 1)});
            APPEND_BYTES(payload, tmppl);

            buildMRecordHeader(header, false, payload);
            APPEND_BYTES(out_bin, header);
            APPEND_BYTES(out_bin, payload);

            header.clear();
            payload.clear();
            tmppl.clear();
            out_count = 1; // we still have one buffered
        }

        APPEND_BYTES(tmppl, tmp);
        tmp.clear();

        emitEntry(tmp, entries[i]);
        out_count++;
        i++;
    }

    APPEND_BYTES(tmppl, tmp);
    tmp.clear();

    prependPICMGHeader(payload);
    payload.emplace_back(std::byte{allowance_for_activation});
    payload.emplace_back(std::byte{out_count});
    APPEND_BYTES(payload, tmppl);

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

MRecordActivationAndPowerMng::MRecordActivationAndPowerMng()
    : MRecordBase("activation_and_power_mng", "Activation and Power Management") {
    record_id       = MRECORD_PICMG_RECORD;
    picmg_record_id = PICMGREC_ACTIVATION_AND_POWER_MNG;
    record_ver      = 1;
}
