#pragma once

#include <gtkmm.h>
#include <iostream>

#include "json.hpp"
#include "types.hpp"

// that's garbage, but it works and i don't want to deal with it anymore
#ifdef _WIN32
#    define FLEN_BASE_LEN 880
#else
#    define FLEN_BASE_LEN 800
#endif

#define FLEN_LABEL_LEN     160
#define FLEN_LABEL_LEN_MIN 20
#define FLEN_LABEL_MARGIN  4

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

    Gtk::Box *getTopContainer();

    void show(Gtk::Box *parent);
    void hide(Gtk::Box *parent);

    virtual void get(nlohmann::json &j) = 0;
    virtual void set(nlohmann::json &j) = 0;

    virtual bool validate() = 0;

    FieldBase(std::string tag, std::string label);
};
