#include <iostream>

#include "areaProductInfo.hpp"

//    ##        #######   ######     ###    ##
//    ##       ##     ## ##    ##   ## ##   ##
//    ##       ##     ## ##        ##   ##  ##
//    ##       ##     ## ##       ##     ## ##
//    ##       ##     ## ##       ######### ##
//    ##       ##     ## ##    ## ##     ## ##
//    ########  #######   ######  ##     ## ########

uchar
AreaProductInfo::getLength() {
    ssize_t length  = const_len;
    length         += precalcLength(manufacturer.str, manufacturer.enc);
    length         += precalcLength(product_name.str, product_name.enc);
    length         += precalcLength(part.str, part.enc);
    length         += precalcLength(version.str, version.enc);
    length         += precalcLength(serial_number.str, serial_number.enc);
    length         += precalcLength(asset_tag.str, asset_tag.enc);
    length         += precalcLength(fru_file_id.str, fru_file_id.enc);
    for (size_t i = 0; i < custom.size(); i++) {
        length += precalcLength(custom[i].str, custom[i].enc);
    }

    // total length
    return ROUND_LEN_TO_8_BYTES_MULTPL(length);
}

void
AreaProductInfo::clear() {
    CLEAR_ENC_STR(manufacturer);
    CLEAR_ENC_STR(product_name);
    CLEAR_ENC_STR(part);
    CLEAR_ENC_STR(version);
    CLEAR_ENC_STR(serial_number);
    CLEAR_ENC_STR(asset_tag);
    CLEAR_ENC_STR(fru_file_id);
    custom.clear();
}

static void
debug_printEncStr(std::string tag, encodedStr str) {
    std::cout << tag << " {data=\'" << str.str << "'"
              << " encoding=" << encodingToString(str.enc) << "}" << std::endl;
}

void
AreaProductInfo::debug_printOutVals() {
    std::cout << "=== " << label << " ===" << std::endl;

    std::cout << "language_code: " << language_code << std::endl;
    debug_printEncStr("manufacturer", manufacturer);
    debug_printEncStr("product_name", product_name);
    debug_printEncStr("part", part);
    debug_printEncStr("version", version);
    debug_printEncStr("serial_number", serial_number);
    debug_printEncStr("asset_tag", asset_tag);
    debug_printEncStr("fru_file_id", fru_file_id);

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
AreaProductInfo::tryParseImpl(T v, Errs &errs) {
    std::string err;
    bool        valid = true;

    if (tryParseField_int(v, "language_code", language_code, errs)) {
        if (language_code > 136) {
            errs.append(tag, "language_code", "lanugage codes capped at 136");
            valid = false;
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

    if (!tryParseField_encStr(v, "part", part, errs)) {
        valid = false;
    }

    if (!tryParseField_encStr(v, "version", version, errs)) {
        valid = false;
    }

    if (!tryParseField_encStr(v, "serial_number", serial_number, errs)) {
        valid = false;
    }

    if (!tryParseField_encStr(v, "asset_tag", asset_tag, errs)) {
        valid = false;
    }

    if (!tryParseField_encStr(v, "fru_file_id", fru_file_id, errs)) {
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
AreaProductInfo::tryParse(nlohmann::json j, Errs &errs) {
    clear();

    return tryParseImpl(j, errs);
}

bool
AreaProductInfo::tryParse(toml::value &t, Errs &errs) {
    clear();

    return tryParseImpl(t, errs);
}

bool
AreaProductInfo::tryParseBinary(biterator begin, biterator end, Errs &errs) {
    UNUSED(end);
    clear();

    biterator beg = begin;

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

    begin += 2;
    if (!tryDecodeStr(begin, "manufacturer", manufacturer, errs)) {
        return false;
    }

    if (!tryDecodeStr(begin, "product_name", product_name, errs)) {
        return false;
    }

    if (!tryDecodeStr(begin, "part", part, errs)) {
        return false;
    }

    if (!tryDecodeStr(begin, "version", version, errs)) {
        return false;
    }

    if (!tryDecodeStr(begin, "serial_number", serial_number, errs)) {
        return false;
    }

    if (!tryDecodeStr(begin, "asset_tag", asset_tag, errs)) {
        return false;
    }

    if (!tryDecodeStr(begin, "fru_file_id", fru_file_id, errs)) {
        return false;
    }

    size_t i = 0;
    while (*(begin) != END_OF_FIELDS_BYTE) {
        encodedStr        es;
        std::stringstream s;
        s << "custom[" << i++ << "]";

        if (!tryDecodeStr(begin, s.str(), es, errs)) {
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
AreaProductInfo::emitJSON(nlohmann::json &j) {
    if (!present) {
        return;
    }

    j["language_code"] = language_code;

    emitEncStr(j, "manufacturer", manufacturer);
    emitEncStr(j, "product_name", product_name);
    emitEncStr(j, "part", part);
    emitEncStr(j, "version", version);
    emitEncStr(j, "serial_number", serial_number);
    emitEncStr(j, "asset_tag", asset_tag);
    emitEncStr(j, "fru_file_id", fru_file_id);
    emitEncStrArr(j, "custom", custom);
}

void
AreaProductInfo::emitTOML(toml::table &t) {
    if (!present) {
        return;
    }

    t["language_code"] = toml::value(language_code);

    emitEncStr(t, "manufacturer", manufacturer);
    emitEncStr(t, "product_name", product_name);
    emitEncStr(t, "part", part);
    emitEncStr(t, "version", version);
    emitEncStr(t, "serial_number", serial_number);
    emitEncStr(t, "asset_tag", asset_tag);
    emitEncStr(t, "fru_file_id", fru_file_id);
    emitEncStrArr(t, "custom", custom);
}

bool
AreaProductInfo::emitBinary(bytes &out_bin, Errs &errs) {
    if (!present) {
        return true;
    }

    bytes bs;

    std::string err;
    bool        valid = true;

    bytes manufacturer_bs;
    bytes product_name_bs;
    bytes part_bs;
    bytes version_bs;
    bytes serial_number_bs;
    bytes asset_tag_bs;
    bytes fru_file_id_bs;
    bytes custom_bs;

    if (!tryEncodeStr("manufacturer", manufacturer, manufacturer_bs, errs)) {
        valid = false;
    }

    if (!tryEncodeStr("product_name", product_name, product_name_bs, errs)) {
        valid = false;
    }

    if (!tryEncodeStr("part", part, part_bs, errs)) {
        valid = false;
    }

    if (!tryEncodeStr("version", version, version_bs, errs)) {
        valid = false;
    }

    if (!tryEncodeStr("serial_number", serial_number, serial_number_bs, errs)) {
        valid = false;
    }

    if (!tryEncodeStr("asset_tag", asset_tag, asset_tag_bs, errs)) {
        valid = false;
    }

    if (!tryEncodeStr("fru_file_id", fru_file_id, fru_file_id_bs, errs)) {
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
                   part_bs.size() + version_bs.size() + serial_number_bs.size() +
                   asset_tag_bs.size() + fru_file_id_bs.size() + custom_bs.size();
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
    APPEND_BYTES(bs, manufacturer_bs);
    APPEND_BYTES(bs, product_name_bs);
    APPEND_BYTES(bs, part_bs);
    APPEND_BYTES(bs, version_bs);
    APPEND_BYTES(bs, serial_number_bs);
    APPEND_BYTES(bs, asset_tag_bs);
    APPEND_BYTES(bs, fru_file_id_bs);
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

AreaProductInfo::AreaProductInfo() : Section("product_info", "Product Info Area") {
    //
}
