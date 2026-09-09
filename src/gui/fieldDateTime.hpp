#pragma once

#include "fieldBase.hpp"

class FieldDateTime : public FieldBase {
  private:
    Gtk::Entry entry;

  public:
    void clear() override;

    void get(nlohmann::json &j) override;
    void set(nlohmann::json &j) override;

    bool validate() override;

    FieldDateTime(std::string tag, std::string label);
};
