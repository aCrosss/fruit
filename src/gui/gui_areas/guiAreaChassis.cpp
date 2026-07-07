#include "guiAreaChassis.hpp"

void
GUIAreaChassis::draw(Gtk::Box *parent) {
    btn_hide.set_label("v");

    header.set_title("Chassis Info Area");
    header.pack_end(btn_hide);

    container.set_orientation(Gtk::ORIENTATION_VERTICAL);
    container.set_spacing(6);
    container.pack_start(header);

    for (auto &&i : fields) {
        i->draw(&container);
    }

    parent->pack_start(container);
}

void
GUIAreaChassis::clear(Gtk::Box *parent) {
    //
}

void
GUIAreaChassis::get(nlohmann::json &j) {
    //
}

void
GUIAreaChassis::set(nlohmann::json &j) {
    nlohmann::json area;
    std::cout << "tag=" << tag << std::endl;
    if (!j.contains(tag)) {
        return;
    }

    area = j[tag];
    for (auto &&i : fields) {
        i->set(area);
    }
}

GUIAreaChassis::GUIAreaChassis(/* args */) : GUIAreaBase("chassis") {
    fields.emplace_back(std::make_shared<FieldEnum>("type", chassis_types_enum, VAL_TYPE_INT));
    fields.emplace_back(std::make_shared<FieldEncStr>("part_number"));
    fields.emplace_back(std::make_shared<FieldEncStr>("serial_number"));
    // fields.emplace_back(std::make_shared<FieldEncStr>("test 3"));
}

GUIAreaChassis::~GUIAreaChassis() {
    //
}
