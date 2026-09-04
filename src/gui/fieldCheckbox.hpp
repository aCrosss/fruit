#pragma once

#include "fieldBase.hpp"

class FieldCheckbox : public FieldBase {
  private:
    Gtk::CheckButton entry;

  public:
    void clear() override;

    void setWidthLevel(uchar level) override;

    void get(nlohmann::json &j) override;
    void set(nlohmann::json &j) override;

    bool validate() override;

    FieldCheckbox(std::string tag, std::string label);
};
