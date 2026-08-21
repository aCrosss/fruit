#include "guiAreaChassis.hpp"

void
GUIAreaChassis::show(Gtk::Box *parent) {
    // btn_hide.set_label("v");

    header.set_title("Chassis Info Area");
    // header.pack_end(btn_hide);

    container.set_orientation(Gtk::ORIENTATION_VERTICAL);
    container.set_spacing(6);
    container.pack_start(header);

    for (auto &&i : fields) {
        i->show(&container);
    }

    parent->pack_start(container);
}

void
GUIAreaChassis::hide(Gtk::Box *parent) {
    for (auto &&i : fields) {
        i->hide(&container);
    }

    parent->remove(container);
}

void
GUIAreaChassis::get(nlohmann::json &j) {
    nlohmann::json jarea;

    for (auto &&i : fields) {
        i->get(jarea);
    }

    j[tag] = jarea;
}

void
GUIAreaChassis::set(nlohmann::json &j) {
    nlohmann::json area;
    if (!j.contains(tag)) {
        return;
    }

    area = j[tag];
    for (auto &&i : fields) {
        i->set(area);
    }
}

GUIAreaChassis::GUIAreaChassis(/* args */) : GUIAreaBase("chassis") {
    fields.emplace_back(
        std::make_shared<FieldEnum>("type", "Type", chassis_types_enum, VAL_TYPE_INT));
    fields.emplace_back(std::make_shared<FieldEncStr>("part_number", "Part Number"));
    fields.emplace_back(std::make_shared<FieldEncStr>("serial_number", "Serial Number"));
    fields.emplace_back(std::make_shared<FieldEncStrArr>("custom", "Custom"));
}

GUIAreaChassis::~GUIAreaChassis() {
    //
}
