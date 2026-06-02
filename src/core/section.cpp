#include <iostream>

#include "section.hpp"

//    ##     ## ####  ######   ######
//    ###   ###  ##  ##    ## ##    ##
//    #### ####  ##  ##       ##
//    ## ### ##  ##   ######  ##
//    ##     ##  ##        ## ##
//    ##     ##  ##  ##    ## ##    ##
//    ##     ## ####  ######   ######

bool
Section::tryEncodeStr(std::string ftag, encodedStr str, bytes &outb, FRU_errs &errs) {
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

//          ##  ######   #######  ##    ##    ########  ########   ######  ########
//          ## ##    ## ##     ## ###   ##    ##     ## ##     ## ##    ## ##     ##
//          ## ##       ##     ## ####  ##    ##     ## ##     ## ##       ##     ##
//          ##  ######  ##     ## ## ## ##    ########  ########   ######  ########
//    ##    ##       ## ##     ## ##  ####    ##        ##   ##         ## ##   ##
//    ##    ## ##    ## ##     ## ##   ###    ##        ##    ##  ##    ## ##    ##
//     ######   ######   #######  ##    ##    ##        ##     ##  ######  ##     ##

//@brief Try read value from JSON object into val. Return true on success and false and
// error string in err otherwise
//@param j nlohmann::json field object
//@param val output bool value
//@param err output error string
bool
Section::tryParseFieldJSON_bool(json j, bool &val, std::string &err) {
    std::stringstream s;

    if (!j.is_boolean()) {
        s << "have invalid type: expected boolean";
        err += s.str();
        return false;
    }

    val = j.get<bool>();
    return true;
}

//@brief Try read value from JSON object into val. Return true on success and false and
// error string in err otherwise
//@param j nlohmann::json field object
//@param val output int value
//@param err output error string
bool
Section::tryParseFieldJSON_int(json j, int &val, std::string &err) {
    std::stringstream s;

    if (!j.is_number_integer()) {
        s << "have invalid type: expected integer";
        err += s.str();
        return false;
    }

    val = j.get<int>();
    return true;
}

//@brief Try read value from JSON object into val. Return true on success and false and
// error string in err otherwise
//@param j nlohmann::json field object
//@param val output nlohmann::json value
//@param err output error string
bool
Section::tryParseFieldJSON_obj(json j, json &val, std::string &err) {
    std::stringstream s;

    if (!j.is_object()) {
        s << "have invalid type: expected JSON object";
        err += s.str();
        return false;
    }

    val = j.get<json>();
    return true;
}

//@brief Try read value from JSON object into val. Return true on success and false and
// error string in err otherwise
//@param j nlohmann::json field object
//@param val output std::vector<nlohmann::json> value
//@param err output error string
bool
Section::tryParseFieldJSON_arr(json j, jarray &val, std::string &err) {
    std::stringstream s;

    if (!j.is_array()) {
        s << "have invalid type: expected JSON array";
        err += s.str();
        return false;
    }

    val = j.get<jarray>();
    return true;
}

//@brief Try read value from JSON object into val. Return true on success and false and
// error string in err otherwise
//@param j nlohmann::json field object
//@param val output std::string value
//@param err output error string
bool
Section::tryParseFieldJSON_str(json j, std::string &val, std::string &err) {
    std::stringstream s;

    if (!j.is_string()) {
        s << "have invalid type: expected string";
        err += s.str();
        return false;
    }

    val = j.get<std::string>();
    return true;
}

//@brief Try read value from JSON object into val. Return true on success and false and
// error string in err otherwise
//@param j nlohmann::json field object
//@param val output encodedStr value
//@param err output error string
bool
Section::tryParseFieldJSON_encStr(json j, encodedStr &val, std::string &err) {
    std::stringstream s;

    if (!j.is_object() || !j.contains("type") || !j.contains("data")) {
        if (!j["type"].is_string() || !j["data"].is_string()) {
            s << "have invalid type: expected JSON object of type";
            s << std::endl << "..." << std::endl;
            s << tag << ": {" << std::endl;
            s << "  type: \"<binary|bcdp|ascii6bit|langcode>\"," << std::endl;
            s << "  data: \"<data string>\"" << std::endl;
            s << "}" << std::endl;
            s << "...";

            err += s.str();
            return false;
        }
    }

    std::string enc_str = j["type"].get<std::string>();
    if (encoding_map.find(enc_str) == encoding_map.end()) {
        s << "invalid encoding type " << enc_str << " ";
        s << "expected <binary|bcdp|ascii6bit|langcode>";
        err += s.str();
        return false;
    }

    val.enc = encoding_map.at(enc_str);
    val.str = j["data"].get<std::string>();

    return true;
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

Section::~Section() {
}
