#include "fieldBase.hpp"

void
FieldBase::drawDefaultLayout(Gtk::Box *parent) {
    if (parent == nullptr) {
        std::cout << tag << " parent is null" << std::endl;
        return;
    }

    label_tag.set_text(tag);
    label_tag.set_width_chars(16);
    label_tag.set_padding(4, 0);
    label_tag.set_xalign(0);
    container.add(label_tag);

    label_error.set_width_chars(64);
    label_error.set_text("Test error message");
    label_error.set_padding(4, 0);
    container.add(label_error);

    container.set_visible(true);
    parent->add(container);
}

FieldBase::FieldBase(std::string tag) {
    this->tag = tag;
}
