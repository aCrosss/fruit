#pragma once

#include "section.hpp"

class scnChassis : public Section {
  private:
    encodedStr              type;
    encodedStr              part_number;
    encodedStr              serial_number;
    std::vector<encodedStr> custom;

    // void initFields();
    void debug_printOutVals();

  public:
    bool   validate() override;
    bool   tryParseJSON(nlohmann::json j, FRU_errs &errs) override;
    bool   tryParseTOML() override;
    bool   emmitBinary(bytes &out_bin, FRU_errs &errs) override;
    bool   decodeBinary(bytes &out_bin) override;
    size_t getByteLen() override;

    scnChassis(/* args */);
    ~scnChassis();
};
