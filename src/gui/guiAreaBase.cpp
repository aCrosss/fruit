#include "guiAreaBase.hpp"

void
GUIAreaBase::clear() {
    for (auto &&i : fields) {
        i->clear();
    }
}

void
GUIAreaBase::show(Gtk::Box *parent) {
    header.set_title(label);

    container.set_orientation(Gtk::ORIENTATION_VERTICAL);
    container.set_spacing(6);
    container.pack_start(header);

    for (auto &&i : fields) {
        i->show(&container);
    }

    parent->pack_start(container);
}

void
GUIAreaBase::hide(Gtk::Box *parent) {
    for (auto &&i : fields) {
        i->hide(&container);
    }

    parent->remove(header);
    parent->remove(container);
}

void
GUIAreaBase::get(nlohmann::json &j) {
    nlohmann::json jarea;

    for (auto &&i : fields) {
        i->get(jarea);
    }

    j[tag] = jarea;
}

void
GUIAreaBase::set(nlohmann::json &j) {
    nlohmann::json area;
    if (!j.contains(tag)) {
        return;
    }

    area = j[tag];
    for (auto &&i : fields) {
        i->set(area);
    }
}

bool
GUIAreaBase::validate() {
    bool valid = true;

    for (auto &&f : fields) {
        if (!f->validate()) {
            valid = false;
        }
    }

    return valid;
}

GUIAreaBase::GUIAreaBase(std::string tag, std::string label) : tag(tag), label(label) {
    //
}
