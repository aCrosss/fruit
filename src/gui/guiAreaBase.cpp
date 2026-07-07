#include "guiAreaBase.hpp"

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

GUIAreaBase::~GUIAreaBase() {
}
