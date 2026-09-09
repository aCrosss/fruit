#include "fieldDateTime.hpp"
#include "common.hpp"
#include "fieldBase.hpp"

void
FieldDateTime::clear() {
    entry.set_text("");
    label_error.set_text("");
}

void
FieldDateTime::get(nlohmann::json &j) {
    j[tag] = entry.get_text();
}

void
FieldDateTime::set(nlohmann::json &j) {
    if (!j.contains(tag)) {
        return;
    }

    entry.set_text(j[tag].get<std::string>());
    validate();
}

bool
FieldDateTime::validate() {
    label_error.set_text("");

    std::string dtime = entry.get_text();

    std::string err;
    if (parseDateTime(dtime, err) < 0) {
        label_error.set_text(err);
        return false;
    }

    return true;
}

FieldDateTime::FieldDateTime(std::string tag, std::string label) : FieldBase(tag, label) {
#ifdef _WIN32
    entry.set_has_frame(true);
#else
    entry.set_has_frame(false);
#endif
    entry.set_hexpand(true);
    entry.signal_changed().connect([this]() { this->validate(); });
    entry.set_placeholder_text("YYYY-MM-DD HH:mm");

    container.pack_end(entry);
}
