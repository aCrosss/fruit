#pragma once

#include "mRecordBase.hpp"

class MRecordAddressTable : public MRecordBase {
  private:
    const size_t ENTRY_BYTE_LEN                = 3;
    const size_t SHELF_ADDR_STR_FIXED_LEN      = 20;
    const size_t SHELF_ADDR_STR_FIXED_LEN_WTLB = 21; // with type/length byte

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
    bool tryParseTableEntry(biterator &begin, biterator end, TableEntry &sd, Errs &errs);

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

    MRecordAddressTable();
};
