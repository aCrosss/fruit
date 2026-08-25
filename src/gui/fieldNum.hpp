#pragma once

#include "fieldBase.hpp"

struct FieldNumProps {
    double initial;
    double min;
    double max;
    double step_increment;
    double page_increment;
    uint   digits; // digits after coma
};

class FieldNum : public FieldBase {
  private:
    Gtk::SpinButton entry;

    bool is_integer;
    bool is_signed;

  public:
    void clear() override;

    void get(nlohmann::json &j) override;
    void set(nlohmann::json &j) override;

    bool validate() override;

    FieldNum(std::string tag, std::string label, FieldNumProps props);
};
