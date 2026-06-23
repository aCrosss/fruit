#include <iostream>

#include "section.hpp"

//    ##     ## ####  ######   ######
//    ###   ###  ##  ##    ## ##    ##
//    #### ####  ##  ##       ##
//    ## ### ##  ##   ######  ##
//    ##     ##  ##        ## ##
//    ##     ##  ##  ##    ## ##    ##
//    ##     ## ####  ######   ######

std::string
Section::getTag() {
    return tag;
}

bool
Section::isPresent() {
    return present;
}

bool
Section::tryEncodeStr(std::string ftag, encodedStr str, bytes &outb, Errs &errs) {
    std::string err;
    bytes       bs;

    if (!encode(str.str, str.enc, bs, err)) {
        errs.append(tag, ftag, err);
        return false;
    }

    std::byte head_byte;
    if (!makeTypeLengthByte(str.enc, bs.size(), head_byte, err)) {
        errs.append(tag, ftag, err);
        return false;
    }

    outb.emplace_back(static_cast<std::byte>(head_byte));
    outb.insert(outb.end(), bs.begin(), bs.end());
    return true;
}

//@brief Try read encoded string from binary
//@param &inb bytes::iterator pointing at type/length byte of encoded string in binary; will
// point at end of string +1 byte after successful read
//@param ftag std::string naming encoded string, for error text only
//@param &str out encoded string
//@param Err class object for errors output
bool
Section::tryDecodeStr(biterator &inb, std::string ftag, encodedStr &str, Errs &errs) {
    std::string err;

    if (!decode(str.str, str.enc, inb, err)) {
        errs.append(tag, ftag, err);
        return false;
    }

    return true;
}

std::byte
Section::calcZeroChecksum(bytes bs) {
    uchar sum = 0;

    for (auto &&b : bs) {
        sum += static_cast<uchar>(b);
    }

    if (sum == 0) {
        return std::byte{0};
    } else {
        return std::byte{static_cast<uchar>(256 - sum)};
    }
}

std::byte
Section::calcZeroChecksum(biterator begin, biterator end) {
    uchar sum = 0;

    for (biterator it = begin; it <= end; it++) {
        sum += static_cast<uchar>(*it);
    }

    if (sum == 0) {
        return std::byte{0};
    } else {
        return std::byte{static_cast<uchar>(256 - sum)};
    }
}

bool
Section::checkChecksums(biterator cs1p, biterator cs2beg, biterator cs2end, Errs &errs) {
    uchar checksum     = static_cast<uchar>(*(cs1p));
    uchar checksum_rec = static_cast<uchar>(calcZeroChecksum(cs2beg, cs2end));
    if (checksum != checksum_rec) {
        std::cout << "expected " << std::hex << static_cast<int>(checksum) << ", but got ";
        std::cout << std::hex << static_cast<int>(checksum_rec) << std::endl;
        errs.append(tag, "common", "checksum is invalid");
        return false;
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

//     #####   ####   ####  #
//     #    # #    # #    # #
//     #####  #    # #    # #
//     #    # #    # #    # #
//     #    # #    # #    # #
//     #####   ####   ####  ######

//@brief Try read value from JSON object into val. Return true on success and false and
// error in errs otherwise. Will handle missing field. Will handle missing field
//@param j nlohmann::json field object
//@param ftag std::string field tag
//@param val output bool value
//@param errs output error
bool
Section::tryParseField_bool(json j, std::string ftag, bool &val, Errs &errs) {
    FILED_MISSING_GUARD(j);

    json jval = j[ftag];

    if (!jval.is_boolean()) {
        errs.append(tag, ftag, "have invalid type: expected boolean");
        return false;
    }

    val = jval.get<bool>();
    return true;
}

//@brief Try read value from TOML object into val. Return true on success and false and
// error in errs otherwise. Will handle missing field. Will handle missing field
//@param t toml::value field object
//@param ftag std::string field tag
//@param val output bool value
//@param errs output error
bool
Section::tryParseField_bool(toml::value t, std::string ftag, bool &val, Errs &errs) {
    FILED_MISSING_GUARD(t);

    toml::value tval = t[ftag];

    if (!tval.is_boolean()) {
        errs.append(tag, ftag, "have invalid type: expected boolean");
        return false;
    }

    val = tval.as_boolean();
    return true;
}

//     # #    # #####
//     # ##   #   #
//     # # #  #   #
//     # #  # #   #
//     # #   ##   #
//     # #    #   #

//@brief Try read value from JSON object into val. Return true on success and false and
// error in errs otherwise. Will handle missing field. Will handle missing field
//@param j nlohmann::json field object
//@param ftag std::string field tag
//@param val output int value
//@param errs output error
bool
Section::tryParseField_int(json j, std::string ftag, int &val, Errs &errs) {
    FILED_MISSING_GUARD(j);

    json jval = j[ftag];

    if (!jval.is_number_integer()) {
        errs.append(tag, ftag, "have invalid type: expected integer");
        return false;
    }

    val = jval.get<int>();
    return true;
}

//@brief Try read value from TOML object into val. Return true on success and false and
// error in errs otherwise. Will handle missing field. Will handle missing field
//@param t toml::value field object
//@param ftag std::string field tag
//@param val output int value
//@param errs output error
bool
Section::tryParseField_int(toml::value t, std::string ftag, int &val, Errs &errs) {
    FILED_MISSING_GUARD(t);

    toml::value tval = t[ftag];

    if (!tval.is_integer()) {
        errs.append(tag, ftag, "have invalid type: expected integer");
        return false;
    }

    val = tval.as_integer();
    return true;
}

//      ####  #####       #
//     #    # #    #      #
//     #    # #####       #
//     #    # #    #      #
//     #    # #    # #    #
//      ####  #####   ####

//@brief Try read value from JSON object into val. Return true on success and false and
// error in errs otherwise. Will handle missing field. Will handle missing field
//@param j nlohmann::json field object
//@param ftag std::string field tag
//@param val output nlohmann::json object value
//@param errs output error
bool
Section::tryParseField_obj(json j, std::string ftag, json &val, Errs &errs) {
    FILED_MISSING_GUARD(j);

    json jval = j[ftag];

    if (!jval.is_object()) {
        errs.append(tag, ftag, "have invalid type: expected JSON object");
        return false;
    }

    val = jval.get<json>();
    return true;
}

//@brief Try read value from TOML object into val. Return true on success and false and
// error in errs otherwise. Will handle missing field. Will handle missing field
//@param t toml::value field object
//@param ftag std::string field tag
//@param val output toml::value table value
//@param errs output error
bool
Section::tryParseField_obj(toml::value t, std::string ftag, toml::value &val, Errs &errs) {
    FILED_MISSING_GUARD(t);

    toml::value tval = t[ftag];

    if (!tval.is_table()) {
        errs.append(tag, ftag, "have invalid type: expected TOML table");
        return false;
    }

    val = tval.as_table();
    return true;
}

//       ##   #####  #####    ##   #   #
//      #  #  #    # #    #  #  #   # #
//     #    # #    # #    # #    #   #
//     ###### #####  #####  ######   #
//     #    # #   #  #   #  #    #   #
//     #    # #    # #    # #    #   #

//@brief Try read value from JSON object into val. Return true on success and false and
// error in errs otherwise. Will handle missing field. Will handle missing field
//@param j nlohmann::json field object
//@param ftag std::string field tag
//@param val output nlohmann::json array value
//@param errs output error
bool
Section::tryParseField_arr(json j, std::string ftag, json &val, Errs &errs) {
    FILED_MISSING_GUARD(j);

    json jval = j[ftag];

    if (!jval.is_array()) {
        errs.append(tag, ftag, "have invalid type: expected JSON array");
        return false;
    }

    val = jval.get<nlohmann::json::array_t>();
    return true;
}

//@brief Try read value from TOML object into val. Return true on success and false and
// error in errs otherwise. Will handle missing field
//@param t toml::value field object
//@param ftag std::string field tag
//@param val output toml::value array value
//@param errs output error
bool
Section::tryParseField_arr(toml::value t, std::string ftag, toml::value &val, Errs &errs) {
    FILED_MISSING_GUARD(t);

    toml::value tval = t[ftag];

    if (!tval.is_array()) {
        errs.append(tag, ftag, "have invalid type: expected TOML array");
        return false;
    }

    val = tval.as_array();
    return true;
}

//      ####  ##### #####  # #    #  ####
//     #        #   #    # # ##   # #    #
//      ####    #   #    # # # #  # #
//          #   #   #####  # #  # # #  ###
//     #    #   #   #   #  # #   ## #    #
//      ####    #   #    # # #    #  ####

//@brief Try read value from JSON object into val. Return true on success and false and
// error in errs otherwise. Will handle missing field
//@param j nlohmann::json field object
//@param ftag std::string field tag
//@param val output string value
//@param errs output error
bool
Section::tryParseField_str(json j, std::string ftag, std::string &val, Errs &errs) {
    FILED_MISSING_GUARD(j);

    json jval = j[ftag];

    if (!jval.is_string()) {
        errs.append(tag, ftag, "have invalid type: expected string");
        return false;
    }

    val = jval.get<std::string>();
    return true;
}

//@brief Try read value from TOML object into val. Return true on success and false and
// error in errs otherwise. Will handle missing field
//@param t toml::value field object
//@param ftag std::string field tag
//@param val output string value
//@param errs output error
bool
Section::tryParseField_str(toml::value t, std::string ftag, std::string &val, Errs &errs) {
    FILED_MISSING_GUARD(t);

    toml::value tval = t[ftag];

    if (!tval.is_string()) {
        errs.append(tag, ftag, "have invalid type: expected string");
        return false;
    }

    val = tval.as_string();
    return true;
}

//     ###### #    #  ####   ####  #####  ###### #####      ####  ##### #####
//     #      ##   # #    # #    # #    # #      #    #    #        #   #    #
//     #####  # #  # #      #    # #    # #####  #    #     ####    #   #    #
//     #      #  # # #      #    # #    # #      #    #         #   #   #####
//     #      #   ## #    # #    # #    # #      #    #    #    #   #   #   #
//     ###### #    #  ####   ####  #####  ###### #####      ####    #   #    #

//@brief Impementation of parsing single encoded string. Expects v, that already IS encoded
// string object. Field tag used only for error output
template <typename T>
inline bool
Section::tryParseEncStrImpl(T v, std::string ftag, encodedStr &val, Errs &errs) {
    std::stringstream s;
    if (!v.contains("type") || !v.contains("data")) {
        if (!v["type"].is_string() || !v["data"].is_string()) {
            s << "have invalid type: expected object of a type:";
            s << std::endl << "..." << std::endl;
            s << tag << ": {" << std::endl;
            s << "  type: \"<binary|bcdp|ascii6bit|langcode>\"," << std::endl;
            s << "  data: \"<data string>\"" << std::endl;
            s << "}" << std::endl;
            s << "...";

            errs.append(tag, ftag, s.str());
            return false;
        }
    }

    std::string enc_str;
    tryParseField_str(v, "type", enc_str, errs);
    std::string val_str;
    tryParseField_str(v, "data", val_str, errs);

    if (encoding_map.find(enc_str) == encoding_map.end()) {
        s << "invalid encoding type " << enc_str << " ";
        s << "expected <binary|bcdp|ascii6bit|langcode>";
        errs.append(tag, ftag, s.str());
        return false;
    }

    val.enc = encoding_map.at(enc_str);
    val.str = val_str;
    return true;
}

//@brief Try read value from JSON object into val. Return true on success and false and
// error in errs otherwise. Will handle missing field
//@param j nlohmann::json field encoded string object
//@param ftag std::string field tag
//@param val output encoded string value
//@param errs output error
bool
Section::tryParseField_encStr(json j, std::string ftag, encodedStr &val, Errs &errs) {
    FILED_MISSING_GUARD(j);

    json jval = j[ftag];

    return tryParseEncStrImpl(jval, ftag, val, errs);
}

//@brief Try read value from TOML object into val. Return true on success and false and
// error in errs otherwise. Will handle missing field
//@param t toml::value field encoded string object
//@param ftag std::string field tag
//@param val output encoded string value
//@param errs output error
bool
Section::tryParseField_encStr(toml::value t, std::string ftag, encodedStr &val, Errs &errs) {
    FILED_MISSING_GUARD(t);

    toml::value tval = t[ftag];

    return tryParseEncStrImpl(tval, ftag, val, errs);
}

//@brief Try read value from JSON object into val. Return true on success and false and
// error in errs otherwise. Will handle missing field. Encoded strings will be appended into the
// vector
//@param j nlohmann::json array of encoded strings object
//@param ftag std::string field tag
//@param val output vector of encoded strings value
//@param errs output error
bool
Section::tryParseField_encStrArr(json                     j,
                                 std::string              ftag,
                                 std::vector<encodedStr> &val,
                                 Errs                    &errs) {
    bool valid = true;

    json array;
    if (!tryParseField_arr(j, "custom", array, errs)) {
        return false;
    }

    for (size_t i = 0; i < array.size(); i++) {
        json       entry = array[i];
        encodedStr estr;

        std::stringstream s;
        s << ftag << "[" << i << "]";
        if (!tryParseEncStrImpl(entry, s.str(), estr, errs)) {
            valid = false;
            continue;
        }

        val.emplace_back(estr);
    }

    return valid;
}

//@brief Try read value from TOML object into val. Return true on success and false and
// error in errs otherwise. Will handle missing field. Encoded strings will be appended into the
// vector
//@param t toml::value array of encoded strings object
//@param ftag std::string field tag
//@param val output vector of encoded strings value
//@param errs output error
bool
Section::tryParseField_encStrArr(toml::value              t,
                                 std::string              ftag,
                                 std::vector<encodedStr> &val,
                                 Errs                    &errs) {
    bool valid = true;

    toml::value array;
    if (!tryParseField_arr(t, ftag, array, errs)) {
        return false;
    }

    for (size_t i = 0; i < array.size(); i++) {
        toml::value entry = array[i];
        encodedStr  estr;

        std::stringstream s;
        s << ftag << "[" << i << "]";
        if (!tryParseEncStrImpl(entry, s.str(), estr, errs)) {
            valid = false;
            continue;
        }

        val.emplace_back(estr);
    }

    return valid;
}

//    ######## ##    ##  ######   #######  ########  #### ##    ##  ######
//    ##       ###   ## ##    ## ##     ## ##     ##  ##  ###   ## ##    ##
//    ##       ####  ## ##       ##     ## ##     ##  ##  ####  ## ##
//    ######   ## ## ## ##       ##     ## ##     ##  ##  ## ## ## ##   ####
//    ##       ##  #### ##       ##     ## ##     ##  ##  ##  #### ##    ##
//    ##       ##   ### ##    ## ##     ## ##     ##  ##  ##   ### ##    ##
//    ######## ##    ##  ######   #######  ########  #### ##    ##  ######

void
Section::emitEncStr(json &j, std::string ftag, encodedStr s) {
    json jes;

    jes["type"] = encodingToString(s.enc);
    jes["data"] = s.str;

    j[ftag] = jes;
}

void
Section::emitEncStr(toml::table &t, std::string ftag, encodedStr s) {
    toml::table est;

    est["type"] = encodingToString(s.enc);
    est["data"] = s.str;

    t[ftag] = toml::value(std::move(est));
}

void
Section::emitEncStrArr(json &j, std::string ftag, std::vector<encodedStr> arr) {
    json jarr;

    if (arr.size() == 0) {
        // nothing to do
        return;
    }

    for (size_t i = 0; i < arr.size(); i++) {
        json        jentry;
        encodedStr &s = arr[i];

        jentry["type"] = encodingToString(s.enc);
        jentry["data"] = s.str;

        jarr[i] = jentry;
    }

    j[ftag] = jarr;
}

void
Section::emitEncStrArr(toml::table &t, std::string ftag, std::vector<encodedStr> arr) {
    toml::array tarr;

    if (arr.size() == 0) {
        // nothing to do
        return;
    }

    for (size_t i = 0; i < arr.size(); i++) {
        toml::table tentry;
        encodedStr &s = arr[i];

        tentry["type"] = encodingToString(s.enc);
        tentry["data"] = s.str;

        tarr.push_back(toml::value(std::move(tentry)));
    }

    t[ftag] = toml::value(std::move(tarr));
}

//    #### ##    ## #### ########
//     ##  ###   ##  ##     ##
//     ##  ####  ##  ##     ##
//     ##  ## ## ##  ##     ##
//     ##  ##  ####  ##     ##
//     ##  ##   ###  ##     ##
//    #### ##    ## ####    ##

Section::Section(std::string tag, std::string label) {
    this->tag   = tag;
    this->label = label;

    present = false;
}
