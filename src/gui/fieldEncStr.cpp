#include "fieldEncStr.hpp"

void
FieldEncStr::draw(Gtk::Box *parent) {
    drawDefaultLayout(parent);

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

    container.add(subbox);
    container.reorder_child(subbox, 1);
}

void
FieldEncStr::clear(Gtk::Box *parent) {
}

void
FieldEncStr::get(nlohmann::json &j) {
    nlohmann::json jes;
    jes["type"] = encoding.get_active_text();
    jes["data"] = entry.get_text();
}

void
FieldEncStr::set(nlohmann::json &j) {
    std::cout << "tag=" << tag << std::endl;
    if (!j.contains(tag)) {
        return;
    }

    nlohmann::json enc_str = j[tag];

    entry.set_text(enc_str["data"].get<std::string>());
    encoding.set_active_text(enc_str["type"].get<std::string>());
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

FieldEncStr::FieldEncStr(std::string tag) : FieldBase(tag) {
    //
}

FieldEncStr::~FieldEncStr() {
    //
}
