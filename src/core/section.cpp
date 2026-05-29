#include "section.hpp"

Section::Section(std::string tag, std::string label) {
    this->tag   = tag;
    this->label = label;

    present = false;
}

Section::~Section() {
}
