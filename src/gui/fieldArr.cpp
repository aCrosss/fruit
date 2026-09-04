#include "fieldArr.hpp"
#include "fieldBase.hpp"
#include "fieldCheckbox.hpp"
#include "fieldEncStr.hpp"
#include "fieldEnum.hpp"
#include "fieldNum.hpp"
#include "fieldStr.hpp"
#include "guiAreaBase.hpp"
#include <string>

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
FieldArr::setWidthLevel(uchar level) {
    personal_width_level = level;

    for (auto &&e : entries) {
        for (auto &&i : e.fields) {
            i->setWidthLevel(personal_width_level + 1);
        }
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

    /* flat array path */
    if (flat) {
        for (auto &&i : entries) {
            nlohmann::json jfield;
            // only one field in plain array
            if (i.fields.empty()) {
                continue;
            }
            i.fields[0]->get(jfield);
            jarray.push_back(jfield[BLANK_TAG]);
        }

        j[tag] = jarray;
        return;
    }

    /* nested array path */
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
        entries[i].btn_up_con.disconnect();
        entries[i].btn_down_con.disconnect();
    }

    for (size_t i = 0; i < entries.size(); i++) {
        size_t ind = i;

        Gtk::Button &b = entries[i].btn_del;
        entries[i].btn_del_con =
            b.signal_clicked().connect([this, ind]() { this->removeEntry(ind); });

        Gtk::Button &bup = entries[i].btn_up;
        entries[i].btn_up_con =
            bup.signal_clicked().connect([this, ind]() { this->moveEntry(ind, -1); });

        Gtk::Button &bdown = entries[i].btn_down;
        entries[i].btn_down_con =
            bdown.signal_clicked().connect([this, ind]() { this->moveEntry(ind, 1); });
    }
}

void
FieldArr::moveEntry(int ind, int d) {
    int cap = static_cast<int>(entries.size());
    if (ind + d < 0 || ind + d >= cap) {
        return;
    }

    // can skip elements before swaping ones, near the end of array it's significantly faster
    size_t from = d < 0 ? ind + d : ind;

    for (size_t i = from; i < entries.size(); ++i) {
        auto &e = entries[i];

        if (i > 0) {
            array_container.remove(e.separator);
        }
        array_container.remove(e.subentry_container);
    }

    std::swap(entries[ind], entries[ind + d]);

    for (size_t i = from; i < entries.size(); ++i) {
        auto &e = entries[i];

        if (i > 0) {
            array_container.pack_start(e.separator);
        }
        array_container.pack_start(e.subentry_container);
    }

    array_container.show_all();
    reconnectSignals();
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

    entry.subentry_container.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
    entry.fields_container.set_orientation(Gtk::ORIENTATION_VERTICAL);

    // del button
    entry.btn_del.set_label("x");
    entry.btn_del.set_vexpand(false);
    entry.btn_del.set_valign(Gtk::ALIGN_START);
    entry.btn_del_con =
        entry.btn_del.signal_clicked().connect([this, ind]() { this->removeEntry(ind); });
    entry.subentry_container.pack_end(entry.btn_del, Gtk::PACK_SHRINK);
    // down  button
    entry.btn_down.set_label("▼");
    entry.btn_down.set_vexpand(false);
    entry.btn_down.set_valign(Gtk::ALIGN_START);
    entry.btn_down_con =
        entry.btn_down.signal_clicked().connect([this, ind]() { this->moveEntry(ind, 1); });
    entry.subentry_container.pack_end(entry.btn_down, Gtk::PACK_SHRINK);
    // up button
    entry.btn_up.set_label("▲");
    entry.btn_up.set_vexpand(false);
    entry.btn_up.set_valign(Gtk::ALIGN_START);
    entry.btn_up_con =
        entry.btn_up.signal_clicked().connect([this, ind]() { this->moveEntry(ind, -1); });
    entry.subentry_container.pack_end(entry.btn_up, Gtk::PACK_SHRINK);

    entry.subentry_container.set_homogeneous(false);

    for (auto &&d : array_description) {
        std::string tag = flat ? BLANK_TAG : d.tag;

        switch (d.type) {
        case FARRAY_FIELD_TYPE_ENUM:
            INIT_FIELD(
                FieldEnum, tag, d.label, DR_AS(d.arg1, enumVals), DR_AS(d.arg2, ValType));
            break;

        case FARRAY_FIELD_TYPE_INT:
            INIT_FIELD(FieldNum, tag, d.label, DR_AS(d.arg1, FieldNumProps));
            break;

        case FARRAY_FIELD_TYPE_ARRAY:
            INIT_FIELD(FieldArr, tag, d.label, DR_AS(d.arg1, FArrayDescr));
            break;

        case FARRAY_FIELD_TYPE_STR:
            INIT_FIELD(FieldStr, tag, d.label, DR_AS(d.arg1, FStrType));
            break;

        case FARRAY_FIELD_TYPE_ENC_STR: INIT_FIELD(FieldEncStr, tag, d.label); break;

        case FARRAY_FIELD_TYPE_CHECKBOX: INIT_FIELD(FieldCheckbox, tag, d.label); break;
        }
    }

    for (auto &&i : entry.fields) {
        entry.fields_container.pack_start(*i->getTopContainer());
        i->setWidthLevel(personal_width_level + 1);
    }
    entry.subentry_container.pack_start(entry.fields_container);
    entry.separator.set_margin_bottom(16);
    if (ind > 0) {
        array_container.pack_start(entry.separator);
    }
    array_container.pack_start(entry.subentry_container);
    array_container.show_all();
}

void
FieldArr::appendEntry(nlohmann::json j) {
    appendEntry();
    FArrayEntry &entry = entries.back();

    /* flat array path */
    if (flat) {
        if (entry.fields.empty()) {
            return;
        }

        nlohmann::json jfield;
        jfield[BLANK_TAG] = j;

        entry.fields[0]->set(jfield);

        array_container.show_all();
        return;
    }

    /* nested array path */
    for (auto &&i : entry.fields) {
        i->set(j);
    }
    array_container.show_all();
}

FieldArr::FieldArr(std::string  tag,
                   std::string  label,
                   FArrayDescr &array_description,
                   bool         flat)
    : FieldBase(tag, label), flat(flat), array_description(array_description) {
    //
    btn_add.set_label("      +      ");
    btn_add.set_hexpand(false);
    btn_add.set_halign(Gtk::ALIGN_END);
    btn_add.signal_clicked().connect([this]() { this->appendEntry(); });
    btn_container.pack_start(btn_add, Gtk::PACK_SHRINK);

    array_container.set_orientation(Gtk::ORIENTATION_VERTICAL);
    array_container.set_spacing(2);
    array_container.pack_end(btn_container);

    personal_width_level = 0;

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
