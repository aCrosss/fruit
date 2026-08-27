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
    }

    return true;
}

FieldStr::FieldStr(std::string tag, std::string label, FStrType type)
    : FieldBase(tag, label), type(type) {
    //
    switch (type) {
    case FSTR_TYPE_IP: entry.set_placeholder_text("0.0.0.0"); break;

    case FSTR_TYPE_GUID:
        entry.set_placeholder_text("xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx");
        break;
    }

    entry.set_has_frame(false);
    entry.set_width_chars(64);
    entry.signal_changed().connect([this]() { this->validate(); });
    container.add(entry);
}
