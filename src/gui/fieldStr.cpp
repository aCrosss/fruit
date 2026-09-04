#include <sstream>
#include <string>

#include "encoding.hpp"
#include "fieldBase.hpp"
#include "fieldStr.hpp"
#include "types.hpp"

void
FieldStr::clear() {
    entry.set_text("");
    label_error.set_text("");
}

void
FieldStr::setWidthLevel(uchar level) {
    uchar chars_width = FLEN_CALC_CHARS(level);
    entry.set_width_chars(chars_width);
    entry.set_max_width_chars(chars_width);
}

void
FieldStr::get(nlohmann::json &j) {
    j[tag] = entry.get_text();
}

void
FieldStr::set(nlohmann::json &j) {
    if (!j.contains(tag)) {
        return;
    }

    entry.set_text(j[tag].get<std::string>());
}

static bool
validate_ip(std::string ip, std::string &err) {
    char dot;
    int  bs[4];

    std::istringstream iss(ip);
    if (iss >> bs[0] >> dot >> bs[1] >> dot >> bs[2] >> dot >> bs[3]) {
        for (size_t i = 0; i < 4; i++) {
            if (bs[i] < 0 || bs[i] > 255) {
                err = "ip must be in [0-255].[0-255].[0-255].[0-255] format";
                return false;
            }
        }
    } else {
        err = "ip must be in [0-255].[0-255].[0-255].[0-255] format";
        return false;
    }

    return true;
}

static bool
validate_guid(std::string s, std::string &err) {
    //           1         2         3
    // 012345678901234567890123456789012345
    // xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx

    // 32 chars without '-'
    if (s.length() < 32) {
        err = "not enough hex symbols in GUID string";
        return false;
    }

    bytes tmp;
    if (!hexStrToBytes(s, tmp, err)) {
        return false;
    }

    if (tmp.size() != 16) {
        err = "not enough bytes in GUID";
        return false;
    }

    return true;
}

static bool
validate_hex(std::string s, size_t expected_len, std::string &err) {
    bytes tmp;

    if (!hexStrToBytes(s, tmp, err)) {
        return false;
    }

    if (expected_len > 0 && tmp.size() != expected_len) {
        std::stringstream ss;
        ss << "expected " << expected_len << " bytes len, but got " << tmp.size();
        err = ss.str();
        return false;
    }

    return true;
}

bool
FieldStr::validate() {
    label_error.set_text("");

    std::string err;

    switch (type) {
    case FSTR_TYPE_IP:
        if (!validate_ip(entry.get_text(), err)) {
            label_error.set_text(err);
            return false;
        }
        break;

    case FSTR_TYPE_GUID:
        if (!validate_guid(entry.get_text(), err)) {
            label_error.set_text(err);
            return false;
        }
        break;

    case FSTR_TYPE_HEX:
        if (!validate_hex(entry.get_text(), expected_len, err)) {
            label_error.set_text(err);
            return false;
        }
        break;
    }

    return true;
}

FieldStr::FieldStr(std::string tag, std::string label, FStrType type, int add_prop)
    : FieldBase(tag, label), type(type) {
    //
    switch (type) {
    case FSTR_TYPE_IP:
        entry.set_placeholder_text("0.0.0.0");
        expected_len = 0;
        break;

    case FSTR_TYPE_GUID:
        entry.set_placeholder_text("xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx");
        expected_len = 0;
        break;

    case FSTR_TYPE_HEX:
        entry.set_placeholder_text("0A0B0C");
        expected_len = add_prop > 0 ? add_prop : 0;
        break;
    }

    entry.set_has_frame(false);
    entry.set_width_chars(FLEN_BASE_LEN_CHARS);
    entry.set_max_width_chars(FLEN_BASE_LEN_CHARS);
    entry.signal_changed().connect([this]() { this->validate(); });
    container.add(entry);
}
