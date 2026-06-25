#include "mRecordPowerDistribuiton.hpp"

#define CHECK_BOUNDS(v, min, max, errs)                                                    \
    if (v < min || v > max) {                                                              \
        std::stringstream s;                                                               \
        s << "must be in range [" << static_cast<int>(min) << ":" << static_cast<int>(max) \
          << "]";                                                                          \
        errs.append(tag, #v, s.str());                                                     \
        valid = false;                                                                     \
    }

#define IS_FLOAT_MULT_OF(v, divider) (std::fabs(std::fmod(v, divider)) < 1e-6f)

//    ##        #######   ######     ###    ##
//    ##       ##     ## ##    ##   ## ##   ##
//    ##       ##     ## ##        ##   ##  ##
//    ##       ##     ## ##       ##     ## ##
//    ##       ##     ## ##       ######### ##
//    ##       ##     ## ##    ## ##     ## ##
//    ########  #######   ######  ##     ## ########

inline void
MRecordPowerDistribuiton::bytesToFloat(biterator b, float &out) {
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
MRecordPowerDistribuiton::tryParsePowerFeeds(biterator &begin, Map &m, Errs &errs) {
    // bool valid = true;

    bytesToFloat(begin + 0, m.max_external_current);
    bytesToFloat(begin + 2, m.max_internal_current);

    int min_ev = static_cast<int>(*(begin + 4));
    if (min_ev < MIN_VOLTAGE_HEX || min_ev > MAX_VOLTAGE_HEX) {
        std::stringstream s;
        s << "must be in range [" << static_cast<int>(MIN_VOLTAGE_HEX) << ":"
          << static_cast<int>(MAX_VOLTAGE_HEX) << "]";
        errs.append(tag, "min_expected_voltage", s.str());
        return false;
    }
    m.min_expected_voltage = 0.5f * min_ev;

    uchar entries_count = static_cast<uchar>(*(begin + 5));

    begin += 6;
    for (uchar i = 0; i < entries_count; i++) {
        MapEntry e;
        e.hardware_address = static_cast<uchar>(*(begin++));
        e.fru_device_id    = static_cast<uchar>(*(begin++));
        m.entries.push_back(e);
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
MRecordPowerDistribuiton::tryParseBinary(biterator begin, biterator end, Errs &errs) {
    UNUSED(end);

    begin             += PICMG_HEADER_LEN;
    uchar feeds_count  = static_cast<uchar>(*(begin++));

    for (uchar i = 0; i < feeds_count; i++) {
        Map m;
        if (!tryParsePowerFeeds(begin, m, errs)) {
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

        jpower_feeds.push_back(jmap);
    }

    j["power_feeds"] = jpower_feeds;
}

void
MRecordPowerDistribuiton::emitTOML(toml::table &t) {
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

        tpower_feeds.push_back(toml::value(std::move(tentries)));
    }

    t["power_feeds"] = toml::value(std::move(tpower_feeds));
}

bool
MRecordPowerDistribuiton::emitBinary(bytes &out_bin, Errs &errs) {
    UNUSED(errs);

    bytes header;
    bytes payload;

    prependPICMGHeader(payload);

    payload.emplace_back(std::byte{static_cast<uchar>(power_feeds.size())});
    for (auto &&m : power_feeds) {
        bytes ext_current_bs;
        bytes int_current_bs;

        floatToBytes(m.max_external_current, ext_current_bs);
        floatToBytes(m.max_internal_current, int_current_bs);

        APPEND_BYTES(payload, ext_current_bs);
        APPEND_BYTES(payload, int_current_bs);

        int min_volt = static_cast<int>(roundf(m.min_expected_voltage / 0.5));
        payload.emplace_back(std::byte{static_cast<uchar>(min_volt)});

        payload.emplace_back(std::byte{static_cast<uchar>(m.entries.size())});
        for (auto &&e : m.entries) {
            payload.emplace_back(std::byte{e.hardware_address});
            payload.emplace_back(std::byte{e.fru_device_id});
        }
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

MRecordPowerDistribuiton::MRecordPowerDistribuiton(/* args */)
    : MRecordBase("power_distribution", "Power Distribution") {
    record_id       = MRECORD_PICMG_RECORD;
    picmg_record_id = PICMGREC_POWER_DISTRIBUTION;
}

MRecordPowerDistribuiton::~MRecordPowerDistribuiton() {
}
