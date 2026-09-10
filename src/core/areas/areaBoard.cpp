#include <iostream>

#include "areaBoard.hpp"
#include "common.hpp"

//    ##        #######   ######     ###    ##
//    ##       ##     ## ##    ##   ## ##   ##
//    ##       ##     ## ##        ##   ##  ##
//    ##       ##     ## ##       ##     ## ##
//    ##       ##     ## ##       ######### ##
//    ##       ##     ## ##    ## ##     ## ##
//    ########  #######   ######  ##     ## ########

//@brief Get minutes from 00:00 01-01-1996 byte vector
//@param &bs area bytes
//@param dtbegin bytes::iterator poining at first out of three date/time bytes
//@param &err output error
//@return minutes from 00:00 01-01-1996 or -1 on error
int
parseDateTime(ibytes dtb, std::string &err) {
    UNUSED(err);

    int dtime = 0;

    for (size_t i = 0; i < 3; i++) {
        dtime += static_cast<int>(*(dtb + i)) << (i * 8);
    }

    return dtime;
}

bool
encodeDateTime(std::string &outs, int minutes, std::string &err) {
    struct tm *tm;

    time_t t = (time_t)minutes * 60 + SECONDS_AT_96;
    tm       = gmtime(&t);

    if (!tm) {
        err = "minutes to date/time parsing failed";
        return false;
    }

    std::stringstream s;
    s << std::setfill('0');
    s << std::setw(4) << tm->tm_year + 1900 << "-";
    s << std::setw(2) << tm->tm_mon + 1 << "-";
    s << std::setw(2) << tm->tm_mday << " ";
    s << std::setw(2) << tm->tm_hour << ":";
    s << std::setw(2) << tm->tm_min;
    outs = s.str();
    return true;
}

bool
encodeDateTime(bytes &bs, int minutes, std::string &err) {
    UNUSED(err);

    std::byte b1, b2, b3;

    b1 = std::byte{static_cast<uchar>(minutes >> 0 & 0xFF)};
    b2 = std::byte{static_cast<uchar>(minutes >> 8 & 0xFF)};
    b3 = std::byte{static_cast<uchar>(minutes >> 16 & 0xFF)};

    bs.emplace_back(b1);
    bs.emplace_back(b2);
    bs.emplace_back(b3);
    return true;
}

static void
debug_printEncStr(std::string tag, encodedStr str) {
    std::cout << tag << " {data=\'" << str.str << "'"
              << " encoding=" << encodingToString(str.enc) << "}" << std::endl;
}

void
AreaBoard::debug_printOutVals() {
    std::cout << "=== " << label << " ===" << std::endl;

    std::cout << "language_code: " << language_code << std::endl;
    std::string dtime;
    std::string err;
    encodeDateTime(dtime, date_time, err);
    std::cout << "date_time: " << dtime << std::endl;
    debug_printEncStr("manufacturer", manufacturer);
    debug_printEncStr("product_name", product_name);
    debug_printEncStr("serial_number", serial_number);
    debug_printEncStr("file_id", file_id);

    std::cout << "custom:" << std::endl;
    for (size_t i = 0; i < custom.size(); i++) {
        std::stringstream s;
        s << "  [" << i << "]";
        debug_printEncStr(s.str(), custom[i]);
    }
}

void
AreaBoard::clear() {
    CLEAR_ENC_STR(manufacturer);
    CLEAR_ENC_STR(product_name);
    CLEAR_ENC_STR(serial_number);
    CLEAR_ENC_STR(file_id);
    custom.clear();
    present = false;
}

uchar
AreaBoard::getLength() {
    ssize_t length  = const_len;
    length         += precalcLength(manufacturer.str, manufacturer.enc);
    length         += precalcLength(product_name.str, product_name.enc);
    length         += precalcLength(serial_number.str, serial_number.enc);
    length         += precalcLength(file_id.str, file_id.enc);
    for (size_t i = 0; i < custom.size(); i++) {
        length += precalcLength(custom[i].str, custom[i].enc);
    }

    // total length
    return ROUND_LEN_TO_8_BYTES_MULTPL(length);
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
AreaBoard::tryParseImpl(T v, Errs &errs) {
    std::string err;
    bool        valid = true;

    if (tryParseField_int(v, "language_code", language_code, errs)) {
        if (language_code > LANG_CODE_CAP) {
            errs.append(tag, "language_code", "lanugage codes capped at 136");
            valid = false;
        }
    } else {
        valid = false;
    }

    std::string dtime_str;
    if (tryParseField_str(v, "date_time", dtime_str, errs)) {
        date_time = parseDateTime(dtime_str, err);
        if (date_time < 0) {
            valid = false;
            errs.append(tag, "date_time", err);
        }
    } else {
        valid = false;
    }

    if (!tryParseField_encStr(v, "manufacturer", manufacturer, errs)) {
        valid = false;
    }

    if (!tryParseField_encStr(v, "product_name", product_name, errs)) {
        valid = false;
    }

    if (!tryParseField_encStr(v, "serial_number", serial_number, errs)) {
        valid = false;
    }

    if (!tryParseField_encStr(v, "file_id", file_id, errs)) {
        valid = false;
    }

    // custom field is optional
    if (!v.contains("custom")) {
        debug_printOutVals();
        return valid;
    }

    if (!tryParseField_encStrArr(v, "custom", custom, errs)) {
        return false;
    }

    debug_printOutVals();
    present = true;
    return valid;
}

bool
AreaBoard::tryParse(nlohmann::json j, Errs &errs) {
    clear();

    return tryParseImpl(j, errs);
}

bool
AreaBoard::tryParse(toml::value &t, Errs &errs) {
    clear();

    return tryParseImpl(t, errs);
}

bool
AreaBoard::tryParseBinary(ibytes begin, ibytes end, Errs &errs) {
    clear();

    ibytes beg = begin;

    // get area length byte at index 1
    uchar length = IPMI_TO_REAL_LEN(static_cast<uchar>(*(++begin)));

    if (!checkChecksums(beg + length - 1, beg, beg + length - 2, errs)) {
        return false;
    }

    language_code = static_cast<int>(*(begin + 1));
    if (language_code > LANG_CODE_CAP) {
        errs.append(tag, "language_code", "lanugage codes capped at 136");
        return false;
    }

    std::string err;
    date_time = parseDateTime(begin + 2, err);
    if (date_time < 0) {
        errs.append(tag, "date_time", err);
        return false;
    }

    begin += 5;
    if (!tryDecodeStr(begin, end, "manufacturer", manufacturer, errs)) {
        return false;
    }

    if (!tryDecodeStr(begin, end, "part_number", product_name, errs)) {
        return false;
    }

    if (!tryDecodeStr(begin, end, "serial_number", serial_number, errs)) {
        return false;
    }

    if (!tryDecodeStr(begin, end, "file_id", file_id, errs)) {
        return false;
    }

    size_t i = 0;
    while (*(begin) != END_OF_FIELDS_BYTE) {
        encodedStr        es;
        std::stringstream s;
        s << "custom[" << i++ << "]";

        if (!tryDecodeStr(begin, end, s.str(), es, errs)) {
            return false;
        }

        custom.emplace_back(es);
    }

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
AreaBoard::emitJSON(nlohmann::json &j) {
    if (!present) {
        return;
    }

    j["language_code"] = language_code;

    std::string ts;
    std::string err;
    encodeDateTime(ts, date_time, err);
    j["date_time"] = ts;

    emitEncStr(j, "manufacturer", manufacturer);
    emitEncStr(j, "product_name", product_name);
    emitEncStr(j, "serial_number", serial_number);
    emitEncStr(j, "file_id", file_id);
    emitEncStrArr(j, "custom", custom);
}

void
AreaBoard::emitTOML(toml::table &t) {
    if (!present) {
        return;
    }

    t["language_code"] = toml::value(language_code);

    std::string ts;
    std::string err;
    encodeDateTime(ts, date_time, err);
    t["date_time"] = toml::value(ts);

    emitEncStr(t, "manufacturer", manufacturer);
    emitEncStr(t, "product_name", product_name);
    emitEncStr(t, "serial_number", serial_number);
    emitEncStr(t, "file_id", file_id);
    emitEncStrArr(t, "custom", custom);
}

bool
AreaBoard::emitBinary(bytes &out_bin, Errs &errs) {
    if (!present) {
        return true;
    }

    bytes bs;

    std::string err;
    bool        valid = true;

    bytes date_time_bs;
    bytes manufacturer_bs;
    bytes product_name_bs;
    bytes serial_number_bs;
    bytes file_id_bs;
    bytes custom_bs;

    if (!encodeDateTime(date_time_bs, date_time, err)) {
        errs.append(tag, "date_time", err);
        valid = false;
    }

    if (!tryEncodeStr("manufacturer", manufacturer, manufacturer_bs, errs)) {
        valid = false;
    }

    if (!tryEncodeStr("product_name", product_name, product_name_bs, errs)) {
        valid = false;
    }

    if (!tryEncodeStr("serial_number", serial_number, serial_number_bs, errs)) {
        valid = false;
    }

    if (!tryEncodeStr("file_id", file_id, file_id_bs, errs)) {
        valid = false;
    }

    for (size_t i = 0; i < custom.size(); i++) {
        auto              estr = custom[i];
        bytes             bs;
        std::stringstream ftag;
        ftag << "custom[" << i << "]";

        if (tryEncodeStr(ftag.str(), estr, bs, errs)) {
            custom_bs.insert(custom_bs.end(), bs.begin(), bs.end());
        } else {
            valid = false;
            continue;
        }
    }

    if (!valid) {
        return false;
    }

    uchar length = const_len + manufacturer_bs.size() + product_name_bs.size() +
                   serial_number_bs.size() + file_id_bs.size() + custom_bs.size();
    // total length
    uchar total  = getLength();
    // byte count of zero filled unused space
    uchar unused = IPMI_TO_REAL_LEN(total) - length;

    // header
    bs.emplace_back(DEFAULT_SECTION_HEADER_BYTE);
    // total length (in multiples of 8 bytes)
    bs.emplace_back(std::byte{total});
    // language code
    bs.emplace_back(std::byte{static_cast<uchar>(language_code)});

    // encoded strings
    APPEND_BYTES(bs, date_time_bs);
    APPEND_BYTES(bs, manufacturer_bs);
    APPEND_BYTES(bs, product_name_bs);
    APPEND_BYTES(bs, serial_number_bs);
    APPEND_BYTES(bs, file_id_bs);
    APPEND_BYTES(bs, custom_bs);

    // end of fields
    bs.emplace_back(END_OF_FIELDS_BYTE);
    // fill unused space with zeroes
    bs.insert(bs.end(), unused, std::byte{0});
    // zero checksum
    bs.emplace_back(calcZeroChecksum(bs));

    // emmit
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

AreaBoard::AreaBoard() : Section("board", "Board Info Area") {
    //
}
