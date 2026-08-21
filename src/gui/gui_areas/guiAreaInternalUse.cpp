#include "guiAreaInternalUse.hpp"
#include "fieldBytes.hpp"
#include "guiAreaBase.hpp"

void
GUIAreaInternalUse::show(Gtk::Box *parent) {
    header.set_title("Internal Use Area");

    container.set_orientation(Gtk::ORIENTATION_VERTICAL);
    container.set_spacing(6);
    container.pack_start(header);

    for (auto &&i : fields) {
        i->show(&container);
    }

    parent->pack_start(container);
}

void
GUIAreaInternalUse::hide(Gtk::Box *parent) {
    for (auto &&i : fields) {
        i->hide(&container);
    }

    parent->remove(container);
}

void
GUIAreaInternalUse::get(nlohmann::json &j) {
    nlohmann::json jarea;

    for (auto &&i : fields) {
        i->get(jarea);
    }

    j[tag] = jarea;
}

void
GUIAreaInternalUse::set(nlohmann::json &j) {
    nlohmann::json area;
    if (!j.contains(tag)) {
        return;
    }

    area = j[tag];
    for (auto &&i : fields) {
        i->set(area);
    }
}

GUIAreaInternalUse::GUIAreaInternalUse() : GUIAreaBase("internal_use") {
    fields.emplace_back(std::make_shared<FieldBytes>("data", "Internal use data"));
}
