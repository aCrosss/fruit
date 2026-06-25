#pragma once

#include "section.hpp"

#define IPMI_HEADER_LEN  5
#define PICMG_HEADER_LEN IPMI_HEADER_LEN + 5

enum MRType {
    MR_IMPI,
    MR_PICMG,
};

enum MRecID {
    MRECORD_POWER_SUPPLY_INFO    = 0x00,
    MRECORD_DC_OUTPUT            = 0x01,
    MRECORD_DC_LOAD              = 0x02,
    MRECORD_MANAGEMENT_ACCESS    = 0x03,
    MRECORD_BASE_COMPATIBILITY   = 0x04,
    MRECORD_EXTEND_COMPATIBILITY = 0x05,
    MRECORD_PICMG_RECORD         = 0xC0,
};

enum PICMGMRecdID {
    PICMGREC_BACKPLANE_P2PCON         = 0x04,
    PICMGREC_ADDRESS_TABLE            = 0x10,
    PICMGREC_POWER_DISTRIBUTION       = 0x11,
    PICMGREC_ACTIVATION_AND_POWER_MNG = 0x12,
    PICMGREC_IP_CONNECTION            = 0x13,
    PICMGREC_BOARD_P2PCON             = 0x14,
    PICMGREC_RADIAL_IPMB0_LINK_MAP    = 0x15,
    PICMGREC_FAN_GEOGRAPHY            = 0x1B,
    PICMGREC_FORM_FACTOR              = 0x2E,
    PICMGREC_LED_DESCRIPTOR           = 0x2F,
};

bool
isMRecordIDValid(int id);
bool
isPICMGMRecordIDValid(int id);

class MRecordBase : public Section {
  protected:
    MRecID       record_id;
    PICMGMRecdID picmg_record_id;
    uchar        record_ver = 0;

    void clear() override = 0;

    void buildMRecordHeader(bytes &out, bool eol, bytes &data);
    void prependPICMGHeader(bytes &out);

  public:
    uchar getLength() override = 0;

    bool tryParse(nlohmann::json j, Errs &errs) override                     = 0;
    bool tryParse(toml::value &t, Errs &errs) override                       = 0;
    bool tryParseBinary(biterator begin, biterator end, Errs &errs) override = 0;

    void emitJSON(nlohmann::json &j) override            = 0;
    void emitTOML(toml::table &t) override               = 0;
    bool emitBinary(bytes &out_bin, Errs &errs) override = 0;

    MRecordBase(std::string tag, std::string label);
    virtual ~MRecordBase() = default;
};
