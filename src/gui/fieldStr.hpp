#pragma once

#include "fieldBase.hpp"

enum FStrType {
    FSTR_TYPE_IP,
    FSTR_TYPE_GUID,
};

class FieldStr : public FieldBase {
  private:
    Gtk::Entry entry;
    FStrType   type;

  public:
    void clear() override;

    void get(nlohmann::json &j) override;
    void set(nlohmann::json &j) override;

    bool validate() override;

    FieldStr(std::string tag, std::string label, FStrType type);
};
