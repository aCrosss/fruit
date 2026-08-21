#include "guiAreaProduct.hpp"
#include "guiAreaBase.hpp"

void
GUIAreaProduct::show(Gtk::Box *parent) {
    header.set_title("Product Info Area");

    container.set_orientation(Gtk::ORIENTATION_VERTICAL);
    container.set_spacing(6);
    container.pack_start(header);

    for (auto &&i : fields) {
        i->show(&container);
    }

    parent->pack_start(container);
}

void
GUIAreaProduct::hide(Gtk::Box *parent) {
    for (auto &&i : fields) {
        i->hide(&container);
    }

    parent->remove(container);
}

void
GUIAreaProduct::get(nlohmann::json &j) {
    nlohmann::json jarea;

    for (auto &&i : fields) {
        i->get(jarea);
    }

    j[tag] = jarea;
}

void
GUIAreaProduct::set(nlohmann::json &j) {
    nlohmann::json area;
    if (!j.contains(tag)) {
        return;
    }

    area = j[tag];
    for (auto &&i : fields) {
        i->set(area);
    }
}

GUIAreaProduct::GUIAreaProduct() : GUIAreaBase("product_info") {
    fields.emplace_back(std::make_shared<FieldEnum>(
        "language_code", "Language Code", lang_codes_enum, VAL_TYPE_INT));
    fields.emplace_back(std::make_shared<FieldEncStr>("manufacturer", "Manufacturer Name"));
    fields.emplace_back(std::make_shared<FieldEncStr>("product_name", "Product Name"));
    fields.emplace_back(std::make_shared<FieldEncStr>("part", "Product Part/Model\nNumber"));
    fields.emplace_back(std::make_shared<FieldEncStr>("version", "Product Version"));
    fields.emplace_back(
        std::make_shared<FieldEncStr>("serial_number", "Product Serial\nNubmer"));
    fields.emplace_back(std::make_shared<FieldEncStr>("asset_tag", "Asset Tag"));
    fields.emplace_back(std::make_shared<FieldEncStr>("fru_file_id", "FRU File ID"));
    fields.emplace_back(std::make_shared<FieldEncStrArr>("custom", "Custom"));
}
