#include "mROutBuff.hpp"
#include "mRecordBase.hpp"
#include "section.hpp"
#include "types.hpp"

#define HEADER_OFFSET_START 4
#define HEADER_OFFSET_END   1

void
MROutBuff::buildMRecordHeader(bool eol, bytes &data) {
    uchar header_len = IPMI_HEADER_LEN;
    uchar total_len  = static_cast<uchar>(header_len + data.size());

    uchar eolb = eol ? MREC_DEF_FORMAT_VER | MRECORD_EOL_BYTE : MREC_DEF_FORMAT_VER;

    out.emplace_back(BYTE_CAST(record_id));
    out.emplace_back(BYTE_CAST(eolb));
    out.emplace_back(BYTE_CAST(total_len));
    out.emplace_back(BYTE_CAST(calcZeroChecksum(data)));

    ibytes hstart = out.end() - HEADER_OFFSET_START;
    ibytes hend   = out.end() - HEADER_OFFSET_END;
    out.emplace_back(BYTE_CAST(calcZeroChecksum(hstart, hend)));
}

bool
MROutBuff::shouldEmmit(size_t appended_len) {
    size_t current_len = IPMI_HEADER_LEN;

    current_len += part_const.size();
    if (should_overwrite) {
        current_len -= overwrite_offset;
        current_len += overwrite.size();
    }

    current_len += part_dynamic.size();

    return current_len + appended_len > MAX_AREA_LEN;
}

void
MROutBuff::emit(bool eol, bool last_part) {
    bytes srdata;

    APPEND_BYTES(srdata, part_const);

    if (should_overwrite) {
        if (srdata.size() < overwrite_offset) {
            throw "MROutBuff: trying to overwrite more of constant part bytes that there is "
                  "present";
        }

        srdata.erase(srdata.end() - overwrite_offset, srdata.end());
        APPEND_BYTES(srdata, overwrite);
        should_overwrite = false;
        overwrite.clear();
    }

    if (counting) {
        auto counter_byte = &part_dynamic.data()[counter_ind];
        *counter_byte     = std::byte{counter};
    }

    APPEND_BYTES(srdata, part_dynamic)
    buildMRecordHeader(eol, srdata);
    APPEND_BYTES(out, srdata)

    part_dynamic.clear();
    if (counting && !last_part) {
        counter_ind = reserveCounter();
    }
}

void
MROutBuff::appendConst(std::byte b) {
    part_const.emplace_back(b);
}

void
MROutBuff::appendConst(bytes bs) {
    if (bs.empty()) {
        throw "MROutBuff: trying to append empty byte vector at const part";
    }

    APPEND_BYTES(part_const, bs);
}

void
MROutBuff::overwriteConstOnce(bytes overwrite, uchar overwrite_offset) {
    this->overwrite        = overwrite;
    this->overwrite_offset = overwrite_offset;

    should_overwrite = true;
}

void
MROutBuff::append(std::byte b) {
    if (shouldEmmit(1)) {
        emit(false, false);
    }

    part_dynamic.emplace_back(b);
    if (counting) {
        counter++;
    }
}

void
MROutBuff::append(bytes bs) {
    if (bs.empty()) {
        throw "MROutBuff: trying to append empty byte vector at dynamic part";
    }

    if (shouldEmmit(bs.size())) {
        emit(false, false);
    }

    APPEND_BYTES(part_dynamic, bs);
    if (counting) {
        counter++;
    }
}

bytes &
MROutBuff::dump() {
    if (!part_dynamic.empty()) {
        emit(eol, true);
    }

    if (out.empty()) {
        throw "MROutBuff: trying to dump empty out byte vector";
    }

    return out;
}

size_t
MROutBuff::reserveCounter() {
    part_dynamic.emplace_back(std::byte{0});
    counter_ind = part_dynamic.size() - 1;
    counter     = 0;
    counting    = true;

    return counter_ind;
}

MROutBuff::MROutBuff(MRecID rec_id, bool eol) : record_id(rec_id), eol(eol) {
    //
}
