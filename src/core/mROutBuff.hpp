#pragma once

#include "types.hpp"

class MROutBuff {
  private:
    MRecID record_id;
    bool   eol;

    bool   counting    = false;
    size_t counter_ind = 0;
    uchar  counter     = 0;

    bytes part_const;
    bool  should_overwrite = false;
    bytes overwrite;
    uchar overwrite_offset;

    bytes part_dynamic;
    bytes out;

    void buildMRecordHeader(bool eol, bytes &data);
    bool shouldEmmit(size_t appended_len);
    void emit(bool eol, bool last_part);

  public:
    void appendConst(std::byte b);
    void appendConst(bytes bs);
    void overwriteConstOnce(bytes overwrite, uchar overwrite_offset);

    void append(std::byte b);
    void append(bytes bs);

    size_t reserveCounter();

    bytes &dump();

    MROutBuff(MRecID rec_id, bool eol);
};
