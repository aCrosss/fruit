#pragma once

#include "section.hpp"

class AreaBoard : public Section {
  private:
    // 1 header byte + 1 length byte + 1 lang code byte + 3 bytes of date/timee
    // + 1 end of fields byte + 1 checkusm byte
    const uchar const_len = 8;

    int                     language_code;
    long long               date_time; // minutes from 00:00 01-01-1996
    encodedStr              manufacturer;
    encodedStr              product_name;
    encodedStr              serial_number;
    encodedStr              file_id;
    std::vector<encodedStr> custom;

    void debug_printOutVals();
    void clear() override;

  public:
    uchar getLength() override;

    template <typename T>
    bool tryParseImpl(T v, Errs &errs);
    bool tryParse(nlohmann::json j, Errs &errs) override;
    bool tryParse(toml::value &t, Errs &errs) override;
    bool tryParseBinary(biterator begin, biterator end, Errs &errs) override;

    void emitJSON(nlohmann::json &j) override;
    void emitTOML(toml::table &t) override;
    bool emitBinary(bytes &out_bin, Errs &errs) override;

    AreaBoard();
    ~AreaBoard();
};
