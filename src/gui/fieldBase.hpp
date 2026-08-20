#pragma once

#include <gtkmm.h>
#include <iostream>

#include "json.hpp"
#include "types.hpp"

class FieldBase {
  protected:
    std::string tag;
    std::string label;

    Gtk::Box   container;
    Gtk::Box   top_container;
    Gtk::Label label_tag;
    Gtk::Label label_error;

    void drawDefaultLayout(Gtk::Box *parent);

  public:
    virtual void clear() = 0;

    virtual void show(Gtk::Box *parent) = 0;
    virtual void hide(Gtk::Box *parent) = 0;

    virtual void get(nlohmann::json &j) = 0;
    virtual void set(nlohmann::json &j) = 0;

    virtual bool validate() = 0;

    FieldBase(std::string tag, std::string label);
    virtual ~FieldBase() = default;
};
