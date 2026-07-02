#pragma once

#include <gtkmm.h>

#include "json.hpp"

class FieldBase {
  protected:
    std::string tag;

    /* data */
  public:
    virtual void draw(Gtk::Box &parent)  = 0;
    virtual void clear(Gtk::Box &parent) = 0;

    virtual void get(nlohmann::json &j) = 0;
    virtual void set(nlohmann::json &j) = 0;

    virtual bool validate() = 0;

    FieldBase(/* args */);
    ~FieldBase();
};

FieldBase::FieldBase(/* args */) {
}

FieldBase::~FieldBase() {
}
