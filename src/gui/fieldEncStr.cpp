#include "fieldEncStr.hpp"
#include "fieldBase.hpp"
#include "types.hpp"
#include <sstream>

void
FieldEncStr::clear() {
    entry.set_text("");
    encoding.set_active(3);
    label_error.set_text("");
}

void
FieldEncStr::get(nlohmann::json &j) {
    nlohmann::json jes;
    jes["type"] = encoding.get_active_text();
    jes["data"] = entry.get_text();

    j[tag] = jes;
}

void
FieldEncStr::set(nlohmann::json &j) {
    if (!j.contains(tag)) {
        return;
    }

    nlohmann::json enc_str = j[tag];

    encoding.set_active_text(enc_str["type"].get<std::string>());
    entry.set_text(enc_str["data"].get<std::string>());
}

bool
FieldEncStr::validate() {
    label_error.set_text("");

    Encoding    enc     = encoding_map[encoding.get_active_text()];
    std::string txt     = entry.get_text();
    int         str_len = precalcLength(txt, enc) - 1;
    int         cap     = byte_cap > 0 ? byte_cap : ENCODED_MAX_BYTE_LENGTH;
    if (str_len > cap) {
        std::stringstream ss;
        ss << "Text byte representation is more than " << cap << " bytes";
        label_error.set_text(ss.str());
        return false;
    }

    bytes       bs;
    std::string err_str;
    if (!encode(txt, enc, bs, err_str)) {
        label_error.set_text(err_str);
        return false;
    }

    return true;
}

FieldEncStr::FieldEncStr(std::string tag, std::string label, uchar byte_cap)
    : FieldBase(tag, label), byte_cap(byte_cap) {
    //
    subbox.set_orientation(Gtk::ORIENTATION_VERTICAL);

#ifdef _WIN32
    entry.set_has_frame(true);
#else
    entry.set_has_frame(false);
#endif
    entry.set_halign(Gtk::ALIGN_FILL);
    entry.set_hexpand(true);
    entry.signal_changed().connect([this]() { this->validate(); });
    subbox.add(entry);

    encoding.append("binary");
    encoding.append("bcdp");
    encoding.append("ascii6bit");
    encoding.append("unicode");
    encoding.set_active(3);
    subbox.add(encoding);

    container.pack_end(subbox);
}
