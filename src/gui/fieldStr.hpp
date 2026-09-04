#pragma once

#include "fieldBase.hpp"

enum FStrType {
    FSTR_TYPE_IP,
    FSTR_TYPE_GUID,
    FSTR_TYPE_HEX,
};

class FieldStr : public FieldBase {
  private:
    Gtk::Entry entry;
    FStrType   type;

    size_t expected_len;

  public:
    void clear() override;

    void setWidthLevel(uchar level) override;

    void get(nlohmann::json &j) override;
    void set(nlohmann::json &j) override;

    bool validate() override;

    FieldStr(std::string tag, std::string label, FStrType type, int add_prop = 0);
};
