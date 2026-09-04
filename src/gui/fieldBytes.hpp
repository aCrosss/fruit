#pragma once

#include <gtkmm.h>
#include <string>

#include "encoding.hpp"
#include "fieldBase.hpp"

class FieldBytes : public FieldBase {
  private:
    Gtk::TextView                 entry;
    Glib::RefPtr<Gtk::TextBuffer> text_buffer;

  public:
    void clear() override;

    void setWidthLevel(uchar level) override;

    void get(nlohmann::json &j) override;
    void set(nlohmann::json &j) override;

    bool validate() override;

    FieldBytes(std::string tag, std::string label);
};
