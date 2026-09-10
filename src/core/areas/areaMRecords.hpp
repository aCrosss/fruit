#pragma once

#include "mRecordBase.hpp"
#include "section.hpp"

typedef std::shared_ptr<MRecordBase> MRecord;

class AreaMRecords : public Section {
  private:
    std::vector<MRecord> mrecords;

    bool validateMRecordHeader(ibytes begin, Errs &errs);
    bool tryAppendMRecord(std::byte type, std::byte byte8, MRecord &mrecord, Errs &errs);

    void debug_printOutVals();
    void clear() override;

  public:
    uchar getLength() override;

    template <typename T>
    bool tryParseImpl(T v, Errs &errs);
    bool tryParse(nlohmann::json j, Errs &errs) override;
    bool tryParse(toml::value &t, Errs &errs) override;
    bool tryParseBinary(ibytes begin, ibytes end, Errs &errs) override;

    void emitJSON(nlohmann::json &j) override;
    void emitTOML(toml::table &t) override;
    bool emitBinary(bytes &out_bin, Errs &errs) override;

    AreaMRecords();
};
