#pragma once

#include "mRecordBase.hpp"

class MRecordPowerDistribuiton : public MRecordBase {
  private:
    // 2 bye external curren + 2 byte internal current + 1 bytes expected voltage + 1 byte count
    const size_t POWER_FEED_BASE_BYTE_LEN = 6;
    const size_t MAP_ENTRY_BYTE_LEN       = 2;

    const int MIN_VOLTAGE_HEX = 0x48;
    const int MAX_VOLTAGE_HEX = 0x90;

    void debug_printOutVals();
    void clear() override;

    void bytesToFloat(ibytes b, float &out);
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
    bool tryParsePowerFeeds(ibytes &begin, ibytes end, Map &m, Errs &errs);

    void emitPowerFeed(bytes &out_bin, Map &power_feed);

    std::vector<Map> power_feeds;

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

    MRecordPowerDistribuiton();
};
