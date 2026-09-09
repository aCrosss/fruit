#include "fieldBase.hpp"

Gtk::Box *
FieldBase::getTopContainer() {
    return &top_container;
}

void
FieldBase::show(Gtk::Box *parent) {
    parent->add(top_container);
}

void
FieldBase::hide(Gtk::Box *parent) {
    parent->remove(top_container);
}

FieldBase::FieldBase(std::string tag, std::string label) : tag(tag), label(label) {
    top_container.set_orientation(Gtk::ORIENTATION_HORIZONTAL);
    container.set_orientation(Gtk::ORIENTATION_VERTICAL);

    label_tag.set_text(label);
    label_tag.set_size_request(label.empty() ? FLEN_LABEL_LEN_MIN : FLEN_LABEL_LEN);
    label_tag.set_margin_end(FLEN_LABEL_MARGIN);
    label_tag.set_xalign(0.0f);
    label_tag.set_yalign(0.0f);
    top_container.add(label_tag);

    label_error.set_text("");
    label_error.set_xalign(0.0f);
    label_error.set_margin_end(FLEN_LABEL_MARGIN);
    container.pack_end(label_error);

    top_container.add(container);
}
