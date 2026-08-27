#pragma once

#include "mRecordBase.hpp"

class MRecordAddressTable : public MRecordBase {
  private:
    struct TableEntry {
        uchar hardware_address;
        uchar site_number;
        uchar site_type;
    };

    encodedStr              shelf_address;
    std::vector<TableEntry> entries;

    void debug_printOutVals();
    void clear() override;

    template <typename T>
    bool tryParseTableEntryImpl(T v, TableEntry &sd, Errs &errs);
    bool tryParseTableEntry(biterator &begin, TableEntry &sd, Errs &errs);

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

    MRecordAddressTable();
};
