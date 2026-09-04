#pragma once

#include <gtkmm.h>
#include <iostream>

#include "json.hpp"
#include "types.hpp"

// that's garbage, but it works and i don't want to deal with it anymore
#define BUTTON_LEN         26
#define BUTTON_LEN_TRIPPLE BUTTON_LEN * 3

#define BLEN_CALC(level) (BUTTON_LEN_TRIPPLE * level)

#define FLEN_BASE_LEN          600
#define FLEN_BASE_LEN_CHARS    FLEN_BASE_LEN / 8
#define FLEN_LEVEL_MULTIPLIER  160 // it's also label_tag length in chars * 8
#define FLEN_CALC(level)       (FLEN_BASE_LEN - level * FLEN_LEVEL_MULTIPLIER - BLEN_CALC(level))
#define FLEN_CALC_CHARS(level) (FLEN_CALC(level) / 10)

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

    virtual void setWidthLevel(uchar level) = 0;

    virtual void get(nlohmann::json &j) = 0;
    virtual void set(nlohmann::json &j) = 0;

    virtual bool validate() = 0;

    FieldBase(std::string tag, std::string label);
};
