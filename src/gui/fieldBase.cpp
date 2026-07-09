#include "fieldBase.hpp"

void
FieldBase::drawDefaultLayout(Gtk::Box *parent) {
    if (parent == nullptr) {
        std::cout << tag << " parent is null" << std::endl;
        return;
    }

    top_container.show_all();
    // parent->add(top_container);
}

FieldBase::FieldBase(std::string tag) {
    this->tag = tag;

    top_container.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
    container.set_orientation(Gtk::ORIENTATION_VERTICAL);

    label_tag.set_text(tag);
    label_tag.set_width_chars(16);
    label_tag.set_padding(4, 0);
    label_tag.set_xalign(0.0f);
    label_tag.set_yalign(0.0f);
    top_container.add(label_tag);

    label_error.set_width_chars(64);
    label_error.set_text("Test error message");
    label_error.set_xalign(0.0f);
    label_error.set_padding(4, 0);
    container.pack_end(label_error);

    top_container.add(container);
}
