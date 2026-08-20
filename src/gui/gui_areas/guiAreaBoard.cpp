#include "guiAreaBoard.hpp"

void
GUIAreaBoard::show(Gtk::Box *parent) {
    header.set_title("Board Info Area");

    container.set_orientation(Gtk::ORIENTATION_VERTICAL);
    container.set_spacing(6);
    container.pack_start(header);

    for (auto &&i : fields) {
        i->show(&container);
    }

    parent->pack_start(container);
}

void
GUIAreaBoard::hide(Gtk::Box *parent) {
    for (auto &&i : fields) {
        i->hide(&container);
    }

    parent->remove(container);
}

void
GUIAreaBoard::get(nlohmann::json &j) {
    nlohmann::json jarea;

    for (auto &&i : fields) {
        i->get(jarea);
    }

    j[tag] = jarea;
}

void
GUIAreaBoard::set(nlohmann::json &j) {
    nlohmann::json area;
    if (!j.contains(tag)) {
        return;
    }

    area = j[tag];
    for (auto &&i : fields) {
        i->set(area);
    }
}

GUIAreaBoard::GUIAreaBoard(/* args */) : GUIAreaBase("board") {
    fields.emplace_back(std::make_shared<FieldEncStr>("manufacturer", "Manufacturer"));
    fields.emplace_back(std::make_shared<FieldEncStr>("product_name", "Product Name"));
    fields.emplace_back(std::make_shared<FieldEncStr>("serial_number", "Serial Number"));
    fields.emplace_back(std::make_shared<FieldEncStr>("file_id", "FRU File ID"));
    fields.emplace_back(std::make_shared<FieldEncStrArr>("custom", "Custom"));
}

GUIAreaBoard::~GUIAreaBoard() {
    //
}
