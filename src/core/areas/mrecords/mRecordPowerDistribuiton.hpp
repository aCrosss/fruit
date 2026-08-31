#pragma once

#include "mRecordBase.hpp"

class MRecordPowerDistribuiton : public MRecordBase {
    const int MIN_VOLTAGE_HEX = 0x48;
    const int MAX_VOLTAGE_HEX = 0x90;

    void debug_printOutVals();
    void clear() override;

    void bytesToFloat(biterator b, float &out);
    void floatToBytes(float f, bytes &out);

    struct MapEntry {
        uchar hardware_address;
        uchar fru_device_id;
    };

    struct Map {
        float max_external_current;
        float max_internal_current;
        float min_expected_voltage;

        std::vector<MapEntry> entries;
    };

    template <typename T>
    bool tryParsePowerFeedsImpl(T v, Map &m, Errs &errs);
    bool tryParsePowerFeeds(biterator &begin, Map &m, Errs &errs);

    std::vector<Map> power_feeds;

  public:
    uchar getLength() override;

    template <typename T>
    bool tryParseImpl(T v, Errs &errs);
    bool tryParse(nlohmann::json j, Errs &errs) override;
    bool tryParse(toml::value &t, Errs &errs) override;
    bool tryParseBinary(biterator begin, biterator end, Errs &errs) override;

    void emitJSON(nlohmann::json &j) override;
    void emitTOML(toml::table &t) override;
    bool emitBinary(bytes &out_bin, bool eol, Errs &errs) override;

    MRecordPowerDistribuiton();
};
