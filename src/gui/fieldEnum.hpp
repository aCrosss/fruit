#pragma once

#include <variant>

#include "fieldBase.hpp"

typedef std::variant<std::string, int>        enumValTyped;
typedef std::tuple<std::string, enumValTyped> enumVal;
typedef std::vector<enumVal>                  enumVals;

class FieldEnum : public FieldBase {
  private:
    Gtk::ComboBoxText         combo_box;
    std::vector<enumValTyped> out_vals;
    ValType                   type;

  public:
    void clear() override;

    void get(nlohmann::json &j) override;
    void set(nlohmann::json &j) override;

    bool validate() override;

    FieldEnum(std::string tag, std::string label, enumVals vars, ValType type);
};
