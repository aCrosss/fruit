#include "fieldEncStr.hpp"

void
FieldEncStr::clear() {
    return;
}

void
FieldEncStr::show(Gtk::Box *parent) {
    // drawDefaultLayout(parent);
    parent->add(top_container);
}

void
FieldEncStr::hide(Gtk::Box *parent) {
    parent->remove(top_container);
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
    if (str_len > ENCODED_MAX_BYTE_LENGTH) {
        label_error.set_text("Text byte representation is more than 64 bytes");
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

FieldEncStr::FieldEncStr(std::string tag, std::string label) : FieldBase(tag, label) {
    subbox.set_orientation(Gtk::ORIENTATION_VERTICAL);

    entry.set_has_frame(false);
    entry.set_width_chars(64);
    entry.signal_changed().connect([this]() { this->validate(); });
    subbox.add(entry);

    encoding.append("binary");
    encoding.append("bcdp");
    encoding.append("ascii6bit");
    encoding.append("unicode");
    subbox.add(encoding);

    container.pack_end(subbox);
}
