#include "fieldCheckbox.hpp"

void
FieldCheckbox::clear() {
    entry.set_active(false);
}

void
FieldCheckbox::get(nlohmann::json &j) {
    j[tag] = entry.get_active();
}

void
FieldCheckbox::set(nlohmann::json &j) {
    if (!j.contains(tag) || !j[tag].is_boolean()) {
        return;
    }

    bool active = j[tag].get<bool>();
    entry.set_active(active);
}

bool
FieldCheckbox::validate() {
    // can't be invalid
    return true;
}

FieldCheckbox::FieldCheckbox(std::string tag, std::string label) : FieldBase(tag, label) {
    container.add(entry);
}
