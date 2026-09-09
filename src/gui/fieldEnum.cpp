#include "fieldEnum.hpp"
#include "fieldBase.hpp"

void
FieldEnum::clear() {
    combo_box.unset_active();
}

void
FieldEnum::get(nlohmann::json &j) {
    int ind = combo_box.get_active_row_number();
    if (ind == -1) {
        ind = 0;
    }

    switch (type) {
    case VAL_TYPE_INT : j[tag] = static_cast<uint>(std::get<int>(out_vals[ind])); break;
    case VAL_TYPE_TEXT: j[tag] = std::get<std::string>(out_vals[ind]); break;

    default: break;
    }
}

void
FieldEnum::set(nlohmann::json &j) {
    if (!j.contains(tag)) {
        return;
    }

    enumValTyped v;

    switch (type) {
    case VAL_TYPE_INT : v = j[tag].get<int>(); break;
    case VAL_TYPE_TEXT: v = j[tag].get<std::string>(); break;

    default: return;
    }

    for (size_t i = 0; i < out_vals.size(); i++) {
        if (out_vals[i] == v) {
            combo_box.set_active(i);
            return;
        }
    }
}

bool
FieldEnum::validate() {
    // enums can't be invalid by itself
    return true;
}

FieldEnum::FieldEnum(std::string tag, std::string label, enumVals vars, ValType type)
    : FieldBase(tag, label), type(type) {

    for (auto &&i : vars) {
        combo_box.append(std::get<0>(i));
        out_vals.push_back(std::get<1>(i));
    }

    combo_box.set_hexpand(true);
    combo_box.set_halign(Gtk::ALIGN_FILL);

    container.add(combo_box);
    container.reorder_child(combo_box, 1);
}
