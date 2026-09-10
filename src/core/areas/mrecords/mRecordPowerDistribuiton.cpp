#include "mRecordPowerDistribuiton.hpp"
#include "section.hpp"
#include "types.hpp"

#define IS_FLOAT_MULT_OF(v, divider) (std::fabs(std::fmod(v, divider)) < 1e-6f)

//    ##        #######   ######     ###    ##
//    ##       ##     ## ##    ##   ## ##   ##
//    ##       ##     ## ##        ##   ##  ##
//    ##       ##     ## ##       ##     ## ##
//    ##       ##     ## ##       ######### ##
//    ##       ##     ## ##    ## ##     ## ##
//    ########  #######   ######  ##     ## ########

inline void
MRecordPowerDistribuiton::bytesToFloat(ibytes b, float &out) {
    int v = static_cast<int>(b[0]) | static_cast<int>(b[1]) << 8;
    out   = static_cast<float>(v) / 10;
}

inline void
MRecordPowerDistribuiton::floatToBytes(float f, bytes &out) {
    int v = static_cast<int>(roundf(f * 10));

    out.emplace_back(std::byte{static_cast<uchar>(v & 0xFF)});
    out.emplace_back(std::byte{static_cast<uchar>((v >> 8) & 0xFF)});
}

void
MRecordPowerDistribuiton::debug_printOutVals() {
    std::cout << "=== " << label << " ===" << std::endl;
    std::cout << "record_id:       " << record_id << std::endl;
    std::cout << "picmg_record_id: " << picmg_record_id << std::endl;

    for (size_t i = 0; i < power_feeds.size(); i++) {
        Map &m = power_feeds[i];
        std::cout << "power_feeds[" << i << "]:" << std::endl;
        std::cout << "  max_external_current:  " << m.max_external_current << std::endl;
        std::cout << "  max_internal_current:  " << m.max_internal_current << std::endl;
        std::cout << "  min_expected_voltage: " << m.min_expected_voltage << std::endl;

        for (size_t j = 0; j < m.entries.size(); j++) {
            MapEntry &e = m.entries[j];
            std::cout << "  entry[" << i << "]:" << std::endl;
            std::cout << "    hardware_address: " << static_cast<int>(e.hardware_address)
                      << std::endl;
            std::cout << "    fru_device_id:    " << static_cast<int>(e.fru_device_id)
                      << std::endl;
        }
    }
}

void
MRecordPowerDistribuiton::clear() {
    power_feeds.clear();
}

uchar
MRecordPowerDistribuiton::getLength() {
    uchar len  = PICMG_HEADER_LEN;
    len       += 1; // number of power feed byte

    for (size_t i = 0; i < power_feeds.size(); i++) {
        len += 2; // max external available current bytes
        len += 2; // max internal current bytes
        len += 1; // minimum expected operating voltage byte
        len += 1; // mapping entries count byte
        // 1 hardware address and 1 fru device id bytes each
        len += power_feeds[i].entries.size() * 2;
    }

    return len;
}

template <typename T>
bool
MRecordPowerDistribuiton::tryParsePowerFeedsImpl(T v, Map &m, Errs &errs) {
    bool valid = true;

    float max_external_current;
    float max_internal_current;
    float min_expected_voltage;

    if (!tryParseField_float(v, "max_external_current", max_external_current, errs)) {
        valid = false;
    }

    if (!tryParseField_float(v, "max_internal_current", max_internal_current, errs)) {
        valid = false;
    }

    if (!tryParseField_float(v, "min_expected_voltage", min_expected_voltage, errs)) {
        valid = false;
    } else {
        if (min_expected_voltage < 36.0 || min_expected_voltage > 72.0) {
            errs.append(tag, "min_expected_voltage", "value must be in [36.0:72.0] range");
            valid = false;
        }

        if (!IS_FLOAT_MULT_OF(min_expected_voltage, 0.5f)) {
            errs.append(tag, "min_expected_voltage", "value is in 1/2 V increment");
            valid = false;
        }
    }

    T array;
    if (!tryParseField_arr(v, "entries", array, errs)) {
        return false;
    }

    for (size_t i = 0; i < array.size(); i++) {
        T        entry = array[i];
        MapEntry e;

        std::stringstream s;
        s << "channels[" << i << "]";

        uchar hardware_address;
        uchar fru_device_id;

        if (!tryParseField_uchar(entry, "hardware_address", hardware_address, errs)) {
            valid = false;
        }

        if (!tryParseField_uchar(entry, "fru_device_id", fru_device_id, errs)) {
            valid = false;
        }

        if (!valid) {
            continue;
        }

        e.hardware_address = hardware_address;
        e.fru_device_id    = fru_device_id;

        m.entries.push_back(e);
    }

    if (!valid) {
        return false;
    }

    m.max_external_current = max_external_current;
    m.max_internal_current = max_internal_current;
    m.min_expected_voltage = min_expected_voltage;

    return true;
}

bool
MRecordPowerDistribuiton::tryParsePowerFeeds(ibytes &begin, ibytes end, Map &m, Errs &errs) {
    OUT_OF_BOUNDS_GUARD_OFFSET("power_feeds", POWER_FEED_BASE_BYTE_LEN)

    bytesToFloat(begin + 0, m.max_external_current);
    bytesToFloat(begin + 2, m.max_internal_current);

    int min_ev = DR_INT(begin + 4);
    if (min_ev < MIN_VOLTAGE_HEX || min_ev > MAX_VOLTAGE_HEX) {
        std::stringstream s;
        s << "must be in range [" << static_cast<int>(MIN_VOLTAGE_HEX) << ":"
          << static_cast<int>(MAX_VOLTAGE_HEX) << "]";
        errs.append(tag, "min_expected_voltage", s.str());
        return false;
    }
    m.min_expected_voltage = 0.5f * min_ev;

    uchar entries_count = DR_BYTE(begin + 5);

    begin += POWER_FEED_BASE_BYTE_LEN;
    for (uchar i = 0; i < entries_count; i++) {
        OUT_OF_BOUNDS_GUARD_OFFSET("entries", MAP_ENTRY_BYTE_LEN)

        MapEntry e;
        e.hardware_address = DR_BYTE(begin++);
        e.fru_device_id    = DR_BYTE(begin++);
        m.entries.push_back(e);
    }

    return true;
}

void
MRecordPowerDistribuiton::emitPowerFeed(bytes &out_bin, Map &power_feed) {
    bytes ext_current_bs;
    bytes int_current_bs;

    floatToBytes(power_feed.max_external_current, ext_current_bs);
    floatToBytes(power_feed.max_internal_current, int_current_bs);

    APPEND_BYTES(out_bin, ext_current_bs);
    APPEND_BYTES(out_bin, int_current_bs);

    int min_volt = static_cast<int>(roundf(power_feed.min_expected_voltage / 0.5));
    out_bin.emplace_back(std::byte{static_cast<uchar>(min_volt)});

    out_bin.emplace_back(std::byte{static_cast<uchar>(power_feed.entries.size())});
    for (auto &&e : power_feed.entries) {
        out_bin.emplace_back(std::byte{e.hardware_address});
        out_bin.emplace_back(std::byte{e.fru_device_id});
    }
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
MRecordPowerDistribuiton::tryParseImpl(T v, Errs &errs) {
    bool valid = true;

    T array;
    if (!tryParseField_arr(v, "power_feeds", array, errs)) {
        return false;
    }

    for (size_t i = 0; i < array.size(); i++) {
        T   entry = array[i];
        Map m;

        std::stringstream s;
        s << "power_feeds[" << i << "]";
        if (!tryParsePowerFeedsImpl(entry, m, errs)) {
            valid = false;
            continue;
        }

        power_feeds.push_back(m);
    }

    debug_printOutVals();
    return valid;
}

bool
MRecordPowerDistribuiton::tryParse(nlohmann::json j, Errs &errs) {
    clear();

    return tryParseImpl(j, errs);
}

bool
MRecordPowerDistribuiton::tryParse(toml::value &t, Errs &errs) {
    clear();

    return tryParseImpl(t, errs);
}

bool
MRecordPowerDistribuiton::tryParseBinary(ibytes begin, ibytes end, Errs &errs) {
    // + feeds_count byte
    OUT_OF_BOUNDS_GUARD_OFFSET("common", PICMG_HEADER_LEN + 1)

    begin             += PICMG_HEADER_LEN;
    uchar feeds_count  = DR_BYTE(begin++);

    for (uchar i = 0; i < feeds_count; i++) {
        Map m;
        if (!tryParsePowerFeeds(begin, end, m, errs)) {
            return false;
        }

        power_feeds.push_back(m);
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
MRecordPowerDistribuiton::emitJSON(nlohmann::json &j) {
    j["record_id"]       = record_id;
    j["picmg_record_id"] = picmg_record_id;

    json jpower_feeds;
    for (auto &&m : power_feeds) {
        json jmap;
        jmap["max_external_current"] = m.max_external_current;
        jmap["max_internal_current"] = m.max_internal_current;
        jmap["min_expected_voltage"] = m.min_expected_voltage;

        json jentries;
        for (auto &&e : m.entries) {
            json jentry;
            jentry["hardware_address"] = e.hardware_address;
            jentry["fru_device_id"]    = e.fru_device_id;
            jentries.push_back(jentry);
        }

        jmap["entries"] = jentries;
        jpower_feeds.push_back(jmap);
    }

    j["power_feeds"] = jpower_feeds;
}

void
MRecordPowerDistribuiton::emitTOML(toml::table &t) {
    t["record_id"]       = toml::value(SC_I(record_id));
    t["picmg_record_id"] = toml::value(SC_I(picmg_record_id));

    toml::array tpower_feeds;
    for (auto &&m : power_feeds) {
        toml::table tmap;
        tmap["max_external_current"] = toml::value(m.max_external_current);
        tmap["max_internal_current"] = toml::value(m.max_internal_current);
        tmap["min_expected_voltage"] = toml::value(m.min_expected_voltage);

        toml::array tentries;
        for (auto &&e : m.entries) {
            toml::table tentry;
            tentry["hardware_address"] = toml::value(e.hardware_address);
            tentry["fru_device_id"]    = toml::value(e.fru_device_id);
            tentries.push_back(toml::value(std::move(tentry)));
        }

        tmap["entries"] = toml::value(std::move(tentries));
        tpower_feeds.push_back(toml::value(std::move(tmap)));
    }

    t["power_feeds"] = toml::value(std::move(tpower_feeds));
}

bool
MRecordPowerDistribuiton::emitBinary(bytes &out_bin, bool eol, Errs &errs) {
    UNUSED(errs);

    if (power_feeds.size() == 0) {
        return true;
    }

    bytes header;
    bytes payload;
    bytes tmp;
    bytes tmppl;

    size_t i         = 0;
    uchar  out_count = 0; // count of power feed in current record

    while (i < power_feeds.size()) {
        size_t len = tmp.size() + tmppl.size() + MRECORD_HEADER_LEN_PICMG + /*entry count*/ 1;
        if (len >= MAX_AREA_LEN) {
            prependPICMGHeader(payload);
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

        emitPowerFeed(tmp, power_feeds[i]);
        out_count++;
        i++;
    }

    APPEND_BYTES(tmppl, tmp);
    tmp.clear();

    prependPICMGHeader(payload);
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

MRecordPowerDistribuiton::MRecordPowerDistribuiton()
    : MRecordBase("power_distribution", "Power Distribution") {
    record_id       = MRECORD_PICMG_RECORD;
    picmg_record_id = PICMGREC_POWER_DISTRIBUTION;
}
