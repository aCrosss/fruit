#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#define UNUSED(x) (void)(x)

// 7:4 - reserved, written as 0000b
// 0:3 - format version number = 1h
#define DEFAULT_SECTION_HEADER_BYTE std::byte{0x01}

#define END_OF_FIELDS_BYTE std::byte{0xC1}

#define LANG_CODE_CAP 136 // Zulu!

#define AREA_TAG_MRECORDS "multi_records"

#define FIELD_TAG_MRECORD_ID       "record_id"
#define FIELD_TAG_PICMG_MRECORD_ID "picmg_record_id"

typedef unsigned char            uchar;
typedef std::vector<std::byte>   bytes;
typedef bytes::iterator          biterator;
typedef std::vector<std::string> svector;

enum ValType {
    VAL_TYPE_NONE,
    VAL_TYPE_INT,
    VAL_TYPE_TEXT,
};

enum Encoding {
    ENCODING_BINARY_UNSPEC = 0, // 00b
    ENCODING_BCDp          = 1, // 01b
    ENCODING_ASCII_6b      = 2, // 10b
    ENCODING_UNOCODE       = 3, // 11b
};

//||ascii6bit|langcode
extern std::unordered_map<std::string, Encoding> encoding_map;

std::string
encodingToString(Encoding enc);

// inner use
enum AreaID {
    //
};

extern std::unordered_map<std::string, int> area_tags;

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
