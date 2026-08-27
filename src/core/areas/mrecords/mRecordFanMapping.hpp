#pragma once

#include "mRecordBase.hpp"

class MRecordFanMapping : public MRecordBase {
  private:
    void debug_printOutVals();
    void clear() override;

    struct TableEntry {
        uchar hardware_address;
        uchar fru_device_id;
        uchar site_number;
        uchar site_type;
    };

    std::vector<TableEntry> entries;

    template <typename T>
    bool tryParseEntry(T v, TableEntry &e, Errs &errs);
    bool tryParseEntry(biterator begin, TableEntry &e, Errs &errs);

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

    MRecordFanMapping();
};
