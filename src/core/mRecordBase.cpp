#include "mRecordBase.hpp"
#include "types.hpp"
#include <cstdlib>

bool
MRecordBase::emitBinary(bytes &out_bin, Errs &errs) {
    UNUSED(out_bin);
    UNUSED(errs);

    std::cerr << "classes derived from MRecordBase should use "
              << "emitBinary(bytes &out_bin, bool eol, Errs &errs) instead of "
              << "emitBinary(bytes &out_bin, Errs &errs)" << std::endl;

    exit(1);
}

bool
isMRecordIDValid(int id) {
    switch (id) {
    case MRECORD_POWER_SUPPLY_INFO:
    case MRECORD_DC_OUTPUT:
    case MRECORD_DC_LOAD:
    case MRECORD_MANAGEMENT_ACCESS:
    case MRECORD_BASE_COMPATIBILITY:
    case MRECORD_EXTEND_COMPATIBILITY:
    case MRECORD_PICMG_RECORD        : return true;
    }

    return false;
}

bool
isPICMGMRecordIDValid(int id) {
    switch (id) {
    case PICMGREC_BACKPLANE_P2PCON:
    case PICMGREC_ADDRESS_TABLE:
    case PICMGREC_POWER_DISTRIBUTION:
    case PICMGREC_ACTIVATION_AND_POWER_MNG:
    case PICMGREC_IP_CONNECTION:
    case PICMGREC_BOARD_P2PCON:
    case PICMGREC_RADIAL_IPMB0_LINK_MAP:
    case PICMGREC_FAN_GEOGRAPHY:
    case PICMGREC_FORM_FACTOR:
    case PICMGREC_LED_DESCRIPTOR          : return true;
    }

    return false;
}

void
MRecordBase::buildMRecordHeader(bytes &bs, bool eol, bytes &data) {
    uchar header_len = IPMI_HEADER_LEN;
    uchar total_len  = static_cast<uchar>(header_len + data.size());
    bs.resize(header_len);

    uchar eolb = eol ? MREC_DEF_FORMAT_VER | MRECORD_EOL_BYTE : MREC_DEF_FORMAT_VER;

    bs[0] = std::byte{static_cast<uchar>(record_id)};
    bs[1] = std::byte{eolb};
    bs[2] = std::byte{total_len};
    bs[3] = calcZeroChecksum(data);
    bs[4] = calcZeroChecksum(bs);
}

void
MRecordBase::prependPICMGHeader(bytes &out) {
    // Note: maybe should be changed to custom manufacturer id, this value taken from PICMG
    // specification
    out.emplace_back(std::byte{0x5A});
    out.emplace_back(std::byte{0x31});
    out.emplace_back(std::byte{0x00});

    out.emplace_back(std::byte{static_cast<uchar>(picmg_record_id)});
    // record format version, 0x00 taken from PICMG specification
    out.emplace_back(std::byte{record_ver});
}

MRecordBase::MRecordBase(std::string tag, std::string label) : Section(tag, label) {
    //
}
