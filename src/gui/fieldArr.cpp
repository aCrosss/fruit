#include "fieldArr.hpp"
#include "fieldBase.hpp"
#include "fieldCheckbox.hpp"
#include "fieldEnum.hpp"
#include "fieldNum.hpp"
#include "guiAreaBase.hpp"

#define DR_AS(pointer, type) (*((type *)(pointer)))

void
FieldArr::clear() {
    if (entries.size() == 0) {
        return;
    }

    size_t size = entries.size();
    for (size_t i = 0; i < size; i++) {
        removeEntry(size - i - 1);
    }
}

void
FieldArr::get(nlohmann::json &j) {
    nlohmann::json jarray;

    // return empty json array if empty
    if (entries.size() == 0) {
        j[tag] = nlohmann::json::array_t{};
        return;
    }

    for (auto &&i : entries) {
        nlohmann::json jentry;
        for (auto &&f : i.fields) {
            f->get(jentry);
        }
        jarray.push_back(jentry);
    }

    j[tag] = jarray;
}

void
FieldArr::set(nlohmann::json &j) {
    if (!j.contains(tag)) {
        return;
    }

    nlohmann::json jval = j[tag];
    if (!jval.is_array()) {
        std::cout << tag << ": have invalid type: expected JSON array";
        return;
    }

    nlohmann::json jarray = jval.get<nlohmann::json::array_t>();
    for (size_t i = 0; i < jarray.size(); ++i) {
        nlohmann::json jentry = jarray[i];
        appendEntry(jentry);
    }
}

bool
FieldArr::validate() {
    bool valid = true;

    for (auto &&i : entries) {
        for (auto &&f : i.fields) {
            if (!f->validate()) {
                valid = false;
            }
        }
    }

    return valid;
}

void
FieldArr::reconnectSignals() {
    for (size_t i = 0; i < entries.size(); i++) {
        entries[i].btn_del_con.disconnect();
    }

    for (size_t i = 0; i < entries.size(); i++) {
        Gtk::Button &b = entries[i].btn_del;

        size_t ind = i;
        entries[i].btn_del_con =
            b.signal_clicked().connect([this, ind]() { this->removeEntry(ind); });
    }
}

void
FieldArr::removeEntry(size_t ind) {
    if (entries.size() == 0 || ind >= entries.size()) {
        return;
    }

    array_container.remove(entries[ind].subentry_container);
    entries.erase(entries.begin() + ind);
}

void
FieldArr::appendEntry() {
    entries.emplace_back();
    FArrayEntry &entry  = entries.back();
    size_t       ind    = entries.size() - 1;
    FieldsRef   &fields = entry.fields;

    entry.entry_container.set_orientation(Gtk::ORIENTATION_VERTICAL);
    entry.subentry_container.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
    entry.fields_container.set_orientation(Gtk::ORIENTATION_VERTICAL);

    entry.btn_del.set_label("x");
    entry.btn_del.set_vexpand(false);
    entry.btn_del.set_valign(Gtk::ALIGN_START);
    entry.btn_del_con =
        entry.btn_del.signal_clicked().connect([this, ind]() { this->removeEntry(ind); });
    entry.subentry_container.pack_end(entry.btn_del, Gtk::PACK_SHRINK);
    entry.subentry_container.set_homogeneous(false);

    for (auto &&d : array_description) {
        switch (d.type) {
        case FARRAY_FIELD_TYPE_ENUM:
            INIT_FIELD(
                FieldEnum, d.tag, d.label, DR_AS(d.arg1, enumVals), DR_AS(d.arg2, ValType));
            break;

        case FARRAY_FIELD_TYPE_INT:
            INIT_FIELD(FieldNum, d.tag, d.label, DR_AS(d.arg1, FieldNumProps));
            break;

        case FARRAY_FIELD_TYPE_ARRAY:
            INIT_FIELD(FieldArr, d.tag, d.label, DR_AS(d.arg1, FArrayDescr));
            break;

        case FARRAY_FIELD_TYPE_CHECKBOX: INIT_FIELD(FieldCheckbox, d.tag, d.label); break;
        }
    }

    for (auto &&i : entry.fields) {
        entry.fields_container.pack_start(*i->getTopContainer());
    }
    entry.subentry_container.pack_start(entry.fields_container);
    array_container.pack_start(entry.subentry_container);
    array_container.show_all();
}

void
FieldArr::appendEntry(nlohmann::json j) {
    appendEntry();
    FArrayEntry &entry = entries.back();

    for (auto &&i : entry.fields) {
        i->set(j);
    }
    array_container.show_all();
}

FieldArr::FieldArr(std::string tag, std::string label, FArrayDescr &array_description)
    : FieldBase(tag, label), array_description(array_description) {
    //
    btn_add.set_label("      +      ");
    btn_add.set_hexpand(false);
    btn_add.set_halign(Gtk::ALIGN_END);
    btn_add.signal_clicked().connect([this]() { this->appendEntry(); });
    btn_container.pack_start(btn_add, Gtk::PACK_SHRINK);

    array_container.set_orientation(Gtk::ORIENTATION_VERTICAL);
    array_container.set_spacing(2);
    array_container.pack_end(btn_container);

    container.add(array_container);
    container.reorder_child(array_container, 1);
}

FieldArr::~FieldArr() {
    for (auto &&e : entries) {
        for (auto &&f : e.fields) {
            f.reset();
        }
    }
}
