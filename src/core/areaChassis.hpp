#pragma once

#include "section.hpp"

class AreaChassis : public Section {
  private:
    encodedStr              type;
    encodedStr              part_number;
    encodedStr              serial_number;
    std::vector<encodedStr> custom;

    // void initFields();
    void debug_printOutVals();

  public:
    bool validate() override;

    bool tryParseJSON(nlohmann::json j, Errs &errs) override;
    bool tryParseTOML(toml::value &t, Errs &errs) override;
    bool tryParseBinary(bytes::iterator in_bin, Errs &errs) override;

    void emitJSON(nlohmann::json &j) override;
    void emitTOML() override;
    bool emitBinary(bytes &out_bin, Errs &errs) override;

    size_t getByteLen() override;
    void   clear() override;

    AreaChassis(/* args */);
    ~AreaChassis();
};
