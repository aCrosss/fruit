#pragma once

#include "mRecordBase.hpp"

class MRecordLEDDescriptor : public MRecordBase {
  private:
    void debug_printOutVals();
    void clear() override;

    struct LEDDescriptor {
        uchar      led_id;
        encodedStr led_legend;
        encodedStr led_symbol;
        encodedStr led_description;
    };

    std::vector<LEDDescriptor> entries;

  public:
    uchar getLength() override;

    template <typename T>
    bool tryParseImpl(T v, Errs &errs);
    bool tryParse(nlohmann::json j, Errs &errs) override;
    bool tryParse(toml::value &t, Errs &errs) override;
    bool tryParseBinary(ibytes begin, ibytes end, Errs &errs) override;

    void emitJSON(nlohmann::json &j) override;
    void emitTOML(toml::table &t) override;
    bool emitBinary(bytes &out_bin, bool eol, Errs &errs) override;

    MRecordLEDDescriptor();
};
