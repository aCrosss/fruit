#include <ctime>
#include <iostream>

#include "areaBoard.hpp"

// seconds at 0:00 hrs 1/1/96 since epoch
#define SECONDS_AT_96 820454400

void
AreaBoard::clear() {
}

//    ##        #######   ######     ###    ##
//    ##       ##     ## ##    ##   ## ##   ##
//    ##       ##     ## ##        ##   ##  ##
//    ##       ##     ## ##       ##     ## ##
//    ##       ##     ## ##       ######### ##
//    ##       ##     ## ##    ## ##     ## ##
//    ########  #######   ######  ##     ## ########

//@brief Get minutes from 00:00 01-01-1996 from string
//@param dtime std::string date/time in format of 'YYYY-mm-DD HH:MM'
//@param &err output error
//@return minutes from 00:00 01-01-1996 or -1 on error
int
parseDateTime(std::string dtime, std::string &err) {
    struct tm tm;

    if (dtime.size() == 0) {
        err = "string is empty";
        return -1;
    }

    std::cout << "parsing '" << dtime << "' date/time string" << std::endl;

    char *errchar = strptime(dtime.c_str(), "%Y-%m-%d %H:%M", &tm);
    if (*errchar != '\0') {
        std::stringstream s;
        s << "failed at symbol '" << *errchar << "', expected 'YYYY-MM-DD HH:mm' string";
        err = s.str();
        return -1;
    }

    time_t seconds = timegm(&tm);
    if (seconds - SECONDS_AT_96 < 0) {
        err = "date/time can't be less than 00:00 01-01-1996";
        return -1;
    }

    // return minutes
    return (seconds - SECONDS_AT_96) / 60;
}

//@brief Get minutes from 00:00 01-01-1996 byte vector
//@param &bs area bytes
//@param dtbegin bytes::iterator poining at first out of three date/time bytes
//@param &err output error
//@return minutes from 00:00 01-01-1996 or -1 on error
int
parseDateTime(bytes::iterator dtb, std::string &err) {
    int dtime = 0;

    for (size_t i = 0; i < 3; i++) {
        dtime += static_cast<int>(*(dtb + i)) << (i * 8);
    }

    return dtime;
}

bool
encodeDateTime(std::string &outs, int minutes, std::string &err) {
    struct tm *tm;

    time_t t = minutes * 60 + SECONDS_AT_96;
    tm       = gmtime(&t);

    if (!tm) {
        err = "minutes to date/time parsing failed";
        return false;
    }

    std::stringstream s;
    s << tm->tm_year + 1900 << "-" << tm->tm_mon + 1 << "-" << tm->tm_mday << " ";
    s << tm->tm_hour << ":" << tm->tm_min;
    outs = s.str();
    return true;
}

bool
encodeDateTime(bytes &bs, int minutes, std::string &err) {
    std::byte b1, b2, b3;

    b1 = std::byte{minutes >> 16 & 0xFF};
    b2 = std::byte{minutes >> 8 & 0xFF};
    b3 = std::byte{minutes >> 0 & 0xFF};

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

    if (v.contains("language_code")) {
        T lv = v["language_code"];
        if (tryParseField_int(lv, language_code, err)) {
            if (language_code > 136) {
                errs.append(tag, "language_code", "lanugage codes capped at 136");
                valid = false;
            }
        } else {
            valid = false;
        }
    } else {
        errs.append(tag, "language_code", "field is missing");
    }

    if (v.contains("date_time")) {
        std::string dtime_str;
        if (tryParseField_str(v["date_time"], dtime_str, err)) {
            date_time = parseDateTime(dtime_str, err);
            if (date_time < 0) {
                valid = false;
                errs.append(tag, "date_time", err);
            }
        } else {
            valid = false;
            errs.append(tag, "date_time", err);
        }
    } else {
        errs.append(tag, "date_time", "field is missing");
    }

    if (!tryDecodeStr(v, "manufacturer", manufacturer, errs)) {
        valid = false;
    }

    if (!tryDecodeStr(v, "product_name", product_name, errs)) {
        valid = false;
    }

    if (!tryDecodeStr(v, "serial_number", serial_number, errs)) {
        valid = false;
    }

    if (!tryDecodeStr(v, "file_id", file_id, errs)) {
        valid = false;
    }

    // custom field is optional
    if (!v.contains("custom")) {
        debug_printOutVals();
        return valid;
    }

    T array;
    if (!tryParseField_arr(v["custom"], array, err)) {
        errs.append(tag, "custom", err);
        err.clear();
    }

    for (size_t i = 0; i < array.size(); i++) {
        T          entry = array[i];
        encodedStr estr;

        std::stringstream s;
        s << "custom[" << i << "]";
        if (!tryDecodeStr(entry, s.str(), estr, errs)) {
            valid = false;
            continue;
        }

        custom.emplace_back(estr);
    }

    debug_printOutVals();
    return valid;
}

bool
AreaBoard::tryParseJSON(nlohmann::json j, Errs &errs) {
    clear();

    return tryParseImpl(j, errs);
}

bool
AreaBoard::tryParseTOML(toml::value &t, Errs &errs) {
    clear();

    return tryParseImpl(t, errs);
}

bool
AreaBoard::tryParseBinary(bytes::iterator in_bin, Errs &errs) {
    clear();

    bytes::iterator begin = in_bin;

    // get area length byte at index 1
    uchar length = static_cast<uchar>(*(++in_bin));

    // TODO: Boilerplate code
    uchar checksum     = static_cast<uchar>(*(begin + length - 1));
    uchar checksum_rec = static_cast<uchar>(calcZeroChecksum(begin, begin + length - 1));
    if (checksum != checksum_rec) {
        std::cout << std::hex << checksum << std::endl;
        std::cout << std::hex << checksum_rec << std::endl;
        errs.append(tag, "common", "checksum is invalid");
        return false;
    }

    language_code = static_cast<int>(*(in_bin + 2));
    if (language_code > LANG_CODE_CAP) {
        errs.append(tag, "language_code", "lanugage codes capped at 136");
        return false;
    }

    std::string err;
    date_time = parseDateTime(in_bin, err);
    if (date_time < 0) {
        errs.append(tag, "date_time", err);
        return false;
    }

    if (!tryDecodeStr(++in_bin, "manufacturer", manufacturer, errs)) {
        return false;
    }

    if (!tryDecodeStr(in_bin, "part_number", product_name, errs)) {
        return false;
    }

    if (!tryDecodeStr(in_bin, "serial_number", serial_number, errs)) {
        return false;
    }

    if (!tryDecodeStr(in_bin, "file_id", file_id, errs)) {
        return false;
    }

    size_t i = 0;
    while (*(in_bin) != END_OF_FIELDS_BYTE) {
        encodedStr        es;
        std::stringstream s;
        s << "custom[" << i++ << "]";

        if (!tryDecodeStr(in_bin, s.str(), es, errs)) {
            return false;
        }

        custom.emplace_back(es);
    }

    std::cout << "Parsed binary:" << std::endl;
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
AreaBoard::emitJSON(nlohmann::json &j) {
}

void
AreaBoard::emitTOML() {
}

bool
AreaBoard::emitBinary(bytes &out_bin, Errs &errs) {
    bytes bs;

    std::string err;
    bool        valid = true;

    bytes date_time_bs;
    bytes manufacturer_bs;
    bytes product_name_bs;
    bytes serial_number_bs;
    bytes file_id_bs;
    bytes custom_bs;

    std::cout << date_time << std::endl;
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
    uchar total  = (length / 8 + 1) * 8;
    // byte count of zero filled unused space
    uchar unused = total - length;

    // header
    bs.emplace_back(DEFAULT_SECTION_HEADER_BYTE);
    // total length (in multiples of 8 bytes)
    bs.emplace_back(std::byte{total});
    // language code
    bs.emplace_back(std::byte{static_cast<uchar>(language_code)});

    bs.insert(bs.end(), date_time_bs.begin(), date_time_bs.end());
    bs.insert(bs.end(), manufacturer_bs.begin(), manufacturer_bs.end());
    bs.insert(bs.end(), product_name_bs.begin(), product_name_bs.end());
    bs.insert(bs.end(), serial_number_bs.begin(), serial_number_bs.end());
    bs.insert(bs.end(), file_id_bs.begin(), file_id_bs.end());
    bs.insert(bs.end(), custom_bs.begin(), custom_bs.end());

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

AreaBoard::~AreaBoard() {
    //
}
