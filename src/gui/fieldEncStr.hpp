#pragma once

#include "fieldBase.hpp"

#include "encoding.hpp"

class FieldEncStr : public FieldBase {
  private:
    Gtk::Box          subbox;
    Gtk::Entry        entry;
    Gtk::ComboBoxText encoding;

  public:
    void clear() override;

    void show(Gtk::Box *parent) override;
    void hide(Gtk::Box *parent) override;

    void get(nlohmann::json &j) override;
    void set(nlohmann::json &j) override;

    bool validate() override;

    FieldEncStr(std::string tag);
    ~FieldEncStr();
};
