#pragma once

#include "encoding.hpp"
#include "fieldBase.hpp"

class FieldEncStrArr : public FieldBase {
  private:
    struct EncStr {
        Gtk::Box          container;
        Gtk::Box          subbox;
        Gtk::Entry        entry;
        Gtk::ComboBoxText encoding;
        Gtk::Label        label_error;
        Gtk::Button       btn_del;
        sigc::connection  btn_del_con;
    };
    std::vector<EncStr> array;

    Gtk::Box array_container;

    Gtk::Box    btn_container;
    Gtk::Button btn_add;

    void validateSingleEncStr(size_t ind);
    void reconnectSignals();
    void addEncStr();
    void addEncStr(nlohmann::json &j);
    void removeEncStr(size_t ind);

  public:
    void clear() override;

    void show(Gtk::Box *parent) override;
    void hide(Gtk::Box *parent) override;

    void get(nlohmann::json &j) override;
    void set(nlohmann::json &j) override;

    bool validate() override;

    FieldEncStrArr(std::string tag, std::string label);
    ~FieldEncStrArr();
};
