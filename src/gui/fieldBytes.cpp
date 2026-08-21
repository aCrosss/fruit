#include "fieldBytes.hpp"
#include "encoding.hpp"
#include "fieldBase.hpp"
#include "types.hpp"

void
FieldBytes::clear() {
    if (text_buffer) {
        text_buffer->set_text("");
    }
}

void
FieldBytes::get(nlohmann::json &j) {
    j[tag] = text_buffer->get_text(false);
}

void
FieldBytes::set(nlohmann::json &j) {
    if (!j.contains(tag)) {
        return;
    }

    std::string txt = j[tag].get<std::string>();
    text_buffer->set_text(txt);
}

bool
FieldBytes::validate() {
    label_error.set_text("");

    std::string txt = text_buffer->get_text();

    bytes       bs;
    std::string err_str;
    if (!hexStrToBytes(txt, bs, err_str)) {
        label_error.set_text(err_str);
        return false;
    }

    return true;
}

FieldBytes::FieldBytes(std::string tag, std::string label) : FieldBase(tag, label) {
    entry.set_wrap_mode(Gtk::WRAP_WORD);

    text_buffer = entry.get_buffer();

    text_buffer->signal_changed().connect([this]() { this->validate(); });

    container.pack_end(entry);
}
