#include <sstream>

#include "fieldEncStrArr.hpp"
#include "json.hpp"

void
FieldEncStrArr::validateSingleEncStr(size_t ind) {
    // std::cout << ind << std::endl;

    if (ind >= array.size()) {
        return;
    }

    EncStr &es = array[ind];

    es.label_error.set_text("");

    Encoding    enc     = encoding_map[es.encoding.get_active_text()];
    std::string txt     = es.entry.get_text();
    int         str_len = precalcLength(txt, enc) - 1;
    if (str_len > ENCODED_MAX_BYTE_LENGTH) {
        es.label_error.set_text("Text byte representation is more than 64 bytes");
        return;
    }

    bytes       bs;
    std::string err_str;
    if (!encode(txt, enc, bs, err_str)) {
        es.label_error.set_text(err_str);
        return;
    }
}

void
FieldEncStrArr::reconnectSignals() {
    for (size_t i = 0; i < array.size(); i++) {
        array[i].btn_del_con.disconnect();
    }

    for (size_t i = 0; i < array.size(); i++) {
        Gtk::Button &b = array[i].btn_del;

        size_t ind = i;
        array[i].btn_del_con =
            b.signal_clicked().connect([this, ind]() { this->removeEncStr(ind); });
    }
}

void
FieldEncStrArr::addEncStr() {
    array.emplace_back();
    EncStr &s   = array.back();
    size_t  ind = array.size() - 1;

    s.container.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
    s.subbox.set_orientation(Gtk::ORIENTATION_VERTICAL);

    s.entry.set_has_frame(false);
    s.entry.set_width_chars(64);
    s.entry.signal_changed().connect([this, ind]() { this->validateSingleEncStr(ind); });
    s.subbox.add(s.entry);

    s.encoding.append("binary");
    s.encoding.append("bcdp");
    s.encoding.append("ascii6bit");
    s.encoding.append("unicode");
    s.subbox.add(s.encoding);

    s.subbox.add(s.label_error);
    s.container.add(s.subbox);

    s.btn_del.set_label("x");
    s.btn_del.set_vexpand(false);
    s.btn_del.set_valign(Gtk::ALIGN_START);
    s.btn_del_con =
        s.btn_del.signal_clicked().connect([this, ind]() { this->removeEncStr(ind); });

    s.container.pack_start(s.btn_del, Gtk::PACK_SHRINK);
    s.container.set_homogeneous(false);

    array_container.pack_start(s.container);
    array_container.show_all();
}

void
FieldEncStrArr::addEncStr(nlohmann::json &j) {
    if (!j.contains("data") || !j.contains("type")) {
        return;
    }

    // show
    addEncStr();

    // set values
    EncStr &s = array.back();

    s.encoding.set_active_text(j["type"].get<std::string>());
    s.entry.set_text(j["data"].get<std::string>());
}

void
FieldEncStrArr::removeEncStr(size_t ind) {
    if (array.size() == 0 || ind >= array.size()) {
        return;
    }

    array_container.remove(array[ind].container);
    array.erase(array.begin() + ind);
    reconnectSignals();
}

void
FieldEncStrArr::clear() {
    if (array.size() == 0) {
        return;
    }

    size_t size = array.size();
    for (size_t i = 0; i < size; i++) {
        removeEncStr(size - i - 1);
    }
}

void
FieldEncStrArr::get(nlohmann::json &j) {
    nlohmann::json jarray;

    // return empty json array if empty
    if (array.size() == 0) {
        j[tag] = nlohmann::json::array_t{};
        return;
    }

    for (auto &&i : array) {
        nlohmann::json jentry;
        jentry["type"] = i.encoding.get_active_text();
        jentry["data"] = i.entry.get_text();
        jarray.push_back(jentry);
    }

    j[tag] = jarray;
}

void
FieldEncStrArr::set(nlohmann::json &j) {
    if (!j.contains(tag)) {
        return;
    }

    nlohmann::json jval = j[tag];

    if (!jval.is_array()) {
        std::cout << tag << ": have invalid type: expected JSON array";
        return;
    }

    nlohmann::json jarray = jval.get<nlohmann::json::array_t>();
    for (size_t i = 0; i < jarray.size(); i++) {
        addEncStr(jarray[i]);
    }
}

bool
FieldEncStrArr::validate() {
    return true;
}

FieldEncStrArr::FieldEncStrArr(std::string tag, std::string label) : FieldBase(tag, label) {
    label_error.set_text("");
    label_error.set_width_chars(0);

    btn_add.set_label("      +      ");
    btn_add.set_hexpand(false);
    btn_add.set_halign(Gtk::ALIGN_END);
    btn_add.signal_clicked().connect([this]() { this->addEncStr(); });
    btn_container.pack_start(btn_add, Gtk::PACK_SHRINK);

    array_container.set_orientation(Gtk::ORIENTATION_VERTICAL);
    array_container.set_spacing(2);
    array_container.pack_end(btn_container);

    container.add(array_container);
    container.reorder_child(array_container, 1);
}

FieldEncStrArr::~FieldEncStrArr() {
}
