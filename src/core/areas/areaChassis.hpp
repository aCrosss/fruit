#pragma once

#include "section.hpp"

class AreaChassis : public Section {
  private:
    // 1 header byte + 1 length byte + 1 end of fields byte + 1 checkusm byte
    uchar const_len = 4;

    encodedStr              type;
    encodedStr              part_number;
    encodedStr              serial_number;
    std::vector<encodedStr> custom;

    // void initFields();
    void debug_printOutVals();

  public:
    void clear() override;

    template <typename T>
    bool tryParseImpl(T v, Errs &errs);
    bool tryParseJSON(nlohmann::json j, Errs &errs) override;
    bool tryParseTOML(toml::value &t, Errs &errs) override;
    bool tryParseBinary(biterator in_bin, Errs &errs) override;

    void emitJSON(nlohmann::json &j) override;
    void emitTOML() override;
    bool emitBinary(bytes &out_bin, Errs &errs) override;

    AreaChassis(/* args */);
    ~AreaChassis();
};
