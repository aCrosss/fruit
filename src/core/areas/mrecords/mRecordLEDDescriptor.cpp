#include "mRecordLEDDescriptor.hpp"

//    ##        #######   ######     ###    ##
//    ##       ##     ## ##    ##   ## ##   ##
//    ##       ##     ## ##        ##   ##  ##
//    ##       ##     ## ##       ##     ## ##
//    ##       ##     ## ##       ######### ##
//    ##       ##     ## ##    ## ##     ## ##
//    ########  #######   ######  ##     ## ########

static void
debug_printEncStr(std::string tag, encodedStr str) {
    std::cout << tag << " {data=\'" << str.str << "'"
              << " encoding=" << encodingToString(str.enc) << "}" << std::endl;
}

void
MRecordLEDDescriptor::debug_printOutVals() {
    std::cout << "=== " << label << " ===" << std::endl;
    std::cout << "record_id:       " << record_id << std::endl;
    std::cout << "picmg_record_id: " << picmg_record_id << std::endl;

    for (size_t i = 0; i < entries.size(); i++) {
        LEDDescriptor &e = entries[i];
        std::cout << "entry[" << i << "]:" << std::endl;

        std::cout << "  led_id: " << static_cast<int>(e.led_id) << std::endl;
        std::cout << "  ";
        debug_printEncStr("led_legend", e.led_legend);
        std::cout << "  ";
        debug_printEncStr("led_symbol", e.led_symbol);
        std::cout << "  ";
        debug_printEncStr("led_description", e.led_description);
    }
}

void
MRecordLEDDescriptor::clear() {
    entries.clear();
}

uchar
MRecordLEDDescriptor::getLength() {
    uchar len  = PICMG_HEADER_LEN;
    len       += 1; // 1 entry count byte

    for (auto &&e : entries) {
        int l  = 0;
        l     += precalcLength(e.led_legend.str, e.led_legend.enc);
        l     += precalcLength(e.led_symbol.str, e.led_symbol.enc);
        l     += precalcLength(e.led_description.str, e.led_description.enc);
        l     += 1; // led_id byte
        len   += static_cast<uchar>(l);
    }

    return len;
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
MRecordLEDDescriptor::tryParseImpl(T v, Errs &errs) {
    bool valid = true;

    T array;
    if (!tryParseField_arr(v, "entries", array, errs)) {
        return false;
    }

    for (size_t i = 0; i < array.size(); i++) {
        T             entry = array[i];
        LEDDescriptor d;

        if (!tryParseField_uchar(entry, "led_id", d.led_id, errs)) {
            valid = false;
        }

        if (!tryParseField_encStr(entry, "led_legend", d.led_legend, errs)) {
            valid = false;
        }

        if (!tryParseField_encStr(entry, "led_symbol", d.led_symbol, errs)) {
            valid = false;
        }

        if (!tryParseField_encStr(entry, "led_description", d.led_description, errs)) {
            valid = false;
        }

        if (!valid) {
            continue;
        }

        // they *shall* implement type 11b
        d.led_legend.enc      = ENCODING_UNOCODE;
        d.led_symbol.enc      = ENCODING_UNOCODE;
        d.led_description.enc = ENCODING_UNOCODE;

        entries.push_back(d);
    }

    if (!valid) {
        return false;
    }

    debug_printOutVals();
    return true;
}

bool
MRecordLEDDescriptor::tryParse(nlohmann::json j, Errs &errs) {
    clear();

    return tryParseImpl(j, errs);
}

bool
MRecordLEDDescriptor::tryParse(toml::value &t, Errs &errs) {
    clear();

    return tryParseImpl(t, errs);
}

bool
MRecordLEDDescriptor::tryParseBinary(biterator begin, biterator end, Errs &errs) {
    UNUSED(end);

    begin += PICMG_HEADER_LEN;

    uchar count = DR_BYTE(begin++);

    for (uchar i = 0; i < count; i++) {
        LEDDescriptor d;

        d.led_id = DR_BYTE(begin++);

        if (!tryDecodeStr(begin, "led_legend", d.led_legend, errs)) {
            return false;
        }

        if (!tryDecodeStr(begin, "led_symbol", d.led_symbol, errs)) {
            return false;
        }

        if (!tryDecodeStr(begin, "led_description", d.led_description, errs)) {
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
MRecordLEDDescriptor::emitJSON(nlohmann::json &j) {
    j["record_id"]       = record_id;
    j["picmg_record_id"] = picmg_record_id;

    json array;
    for (auto &&e : entries) {
        json entry;
        entry["led_id"] = e.led_id;
        emitEncStr(entry, "led_legend", e.led_legend);
        emitEncStr(entry, "led_symbol", e.led_symbol);
        emitEncStr(entry, "led_description", e.led_description);

        array.push_back(entry);
    }

    j["entries"] = array;
}

void
MRecordLEDDescriptor::emitTOML(toml::table &t) {
    t["record_id"]       = toml::value(SC_I(record_id));
    t["picmg_record_id"] = toml::value(SC_I(picmg_record_id));

    toml::array array;
    for (auto &&e : entries) {
        toml::table entry;
        entry["led_id"] = toml::value(e.led_id);
        emitEncStr(entry, "led_legend", e.led_legend);
        emitEncStr(entry, "led_symbol", e.led_symbol);
        emitEncStr(entry, "led_description", e.led_description);

        array.push_back(toml::value(std::move(entry)));
    }

    t["entries"] = toml::value(std::move(array));
}

bool
MRecordLEDDescriptor::emitBinary(bytes &out_bin, Errs &errs) {
    bool valid = true;

    bytes header;
    bytes payload;

    prependPICMGHeader(payload);

    uchar entries_count = static_cast<uchar>(entries.size());
    payload.emplace_back(std::byte{entries_count});

    for (auto &&e : entries) {
        bytes led_legend_bs;
        bytes led_symbol_bs;
        bytes led_description_bs;

        if (!tryEncodeStr("led_legend", e.led_legend, led_legend_bs, errs)) {
            valid = false;
        }

        if (!tryEncodeStr("led_symbol", e.led_symbol, led_symbol_bs, errs)) {
            valid = false;
        }

        if (!tryEncodeStr("led_description", e.led_description, led_description_bs, errs)) {
            valid = false;
        }

        payload.emplace_back(std::byte{e.led_id});
        APPEND_BYTES(payload, led_legend_bs);
        APPEND_BYTES(payload, led_symbol_bs);
        APPEND_BYTES(payload, led_description_bs);
    }

    if (!valid) {
        return false;
    }

    buildMRecordHeader(header, payload);

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

MRecordLEDDescriptor::MRecordLEDDescriptor() : MRecordBase("led_descriptor", "LED Descriptor") {
    record_id       = MRECORD_PICMG_RECORD;
    picmg_record_id = PICMGREC_LED_DESCRIPTOR;
}
