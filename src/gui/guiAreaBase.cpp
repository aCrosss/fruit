#include "guiAreaBase.hpp"

void
GUIAreaBase::clear() {
    for (auto &&i : fields) {
        i->clear();
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

GUIAreaBase::GUIAreaBase(std::string tag) {
    this->tag = tag;
}
