#include "fieldDateTime.hpp"
#include "common.hpp"

void
FieldDateTime::clear() {
	entry.set_text("");
}

void
FieldDateTime::show(Gtk::Box *parent) {
	parent->add(top_container);
}

void
FieldDateTime::hide(Gtk::Box *parent) {
	parent->remove(top_container);
}

void
FieldDateTime::get(nlohmann::json &j) {
	j[tag] = entry.get_text();
}

void
FieldDateTime::set(nlohmann::json &j) {
	if (!j.contains(tag)) {
        return;
    }

    entry.set_text(j[tag].get<std::string>());
    validate();
}

bool
FieldDateTime::validate() {
	label_error.set_text("");

	std::string dtime = entry.get_text();

	std::string err;
	if (parseDateTime(dtime, err) < 0) {
		label_error.set_text(err);
		return false;
	}

	return false;
}


FieldDateTime::FieldDateTime(std::string tag, std::string label): FieldBase(tag, label) {
    entry.set_has_frame(false);
    entry.set_width_chars(64);
    entry.signal_changed().connect([this]() { this->validate(); });
    entry.set_placeholder_text("YYYY-MM-DD HH:mm");

    container.pack_end(entry);
}