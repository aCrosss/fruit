#include "areaMRecords.hpp"

// clang-format off
#include "mrecords/mRecordBackplaneP2PCon.hpp"
#include "mrecords/mRecordAddressTable.hpp"
#include "mrecords/mRecordPowerDistribuiton.hpp"
#include "mrecords/mRecordActivationAndPowerMng.hpp"
#include "mrecords/mRecordIPConnection.hpp"
#include "mrecords/mRecordBoardP2PCon.hpp"
#include "mrecords/mRecordFanMapping.hpp"
#include "mrecords/mRecordFormFactor.hpp"
#include "mrecords/mRecordLEDDescriptor.hpp"
// clang-format on

#define HDR_OFFSET_TYPE   0 // offset to record type byte
#define HDR_OFFSET_EOL    1 // offset to end of list byte
#define HDR_OFFSET_LEN    2 // offset to record length byte
#define HDR_OFFSET_PLD_CS 3 // offset to record checkusm byte
#define HDR_OFFSET_HDR_CS 4 // offset to header checksum byte

#define APPEND_MRECORD(mrecord)                      \
    mrecords.push_back(std::make_shared<mrecord>()); \
    break;

//    ##        #######   ######     ###    ##
//    ##       ##     ## ##    ##   ## ##   ##
//    ##       ##     ## ##        ##   ##  ##
//    ##       ##     ## ##       ##     ## ##
//    ##       ##     ## ##       ######### ##
//    ##       ##     ## ##    ## ##     ## ##
//    ########  #######   ######  ##     ## ########

void
AreaMRecords::debug_printOutVals() {
}

void
AreaMRecords::clear() {
    for (auto &&i : mrecords) {
        i.reset();
    }

    mrecords.clear();
}

uchar
AreaMRecords::getLength() {
    return uchar();
}

//    ########  ########  ######   #######  ########  ########   ######
//    ##     ## ##       ##    ## ##     ## ##     ## ##     ## ##    ##
//    ##     ## ##       ##       ##     ## ##     ## ##     ## ##
//    ########  ######   ##       ##     ## ########  ##     ##  ######
//    ##   ##   ##       ##       ##     ## ##   ##   ##     ##       ##
//    ##    ##  ##       ##    ## ##     ## ##    ##  ##     ## ##    ##
//    ##     ## ########  ######   #######  ##     ## ########   ######

bool
AreaMRecords::validateMRecordHeader(biterator begin, Errs &errs) {
    int  record_id       = static_cast<int>(*begin);
    int  byte9           = static_cast<int>(*(begin + 8));
    bool is_PICMG_record = record_id == MRECORD_PICMG_RECORD;
    int  record_len      = static_cast<int>(*(begin + HDR_OFFSET_LEN));

    if (!isMRecordIDValid(record_id)) {
        std::stringstream s;
        s << "invalid id=" << record_id;
        errs.append(tag, "multirecord", s.str());
        return false;
    }

    if (is_PICMG_record && !isPICMGMRecordIDValid(byte9)) {
        std::stringstream s;
        s << "invalid PICMG id=" << byte9;
        errs.append(tag, "multirecord", s.str());
        return false;
    }

    std::stringstream t;
    if (is_PICMG_record) {
        t << "multirecord(id=" << record_id << ":" << byte9 << ")";
    } else {
        t << "multirecord(id=" << record_id << ")";
    }

    biterator header_cs  = begin + HDR_OFFSET_HDR_CS;
    biterator payload_cs = begin + HDR_OFFSET_PLD_CS;

    if (!checkChecksums(header_cs, begin, begin + HDR_OFFSET_PLD_CS, errs)) {
        std::stringstream s;
        s << "header checksum is invalid to header itself";
        errs.append(tag, t.str(), s.str());
    }

    biterator data_begin = begin + IPMI_HEADER_LEN;
    biterator data_end   = begin + record_len - 1;

    if (!checkChecksums(payload_cs, data_begin, data_end, errs)) {
        std::stringstream s;
        s << "header data checksum is invalid to multirecord data";
        errs.append(tag, t.str(), s.str());
        return false;
    }

    return true;
}

bool
AreaMRecords::tryAppendMRecord(std::byte type, std::byte byte9, MRecord &mrecord, Errs &errs) {
    MRecID record_type = MRecID(type);

    switch (record_type) {
    case MRECORD_POWER_SUPPLY_INFO   : break;
    case MRECORD_DC_OUTPUT           : break;
    case MRECORD_DC_LOAD             : break;
    case MRECORD_MANAGEMENT_ACCESS   : break;
    case MRECORD_BASE_COMPATIBILITY  : break;
    case MRECORD_EXTEND_COMPATIBILITY: break;
    case MRECORD_PICMG_RECORD        : {
        PICMGMRecdID picmg_record_id = PICMGMRecdID(byte9);

        switch (picmg_record_id) {
        case PICMGREC_BACKPLANE_P2PCON        : APPEND_MRECORD(MRecordBackplaneP2PCon)
        case PICMGREC_ADDRESS_TABLE           : APPEND_MRECORD(MRecordAddressTable)
        case PICMGREC_POWER_DISTRIBUTION      : APPEND_MRECORD(MRecordPowerDistribuiton)
        case PICMGREC_ACTIVATION_AND_POWER_MNG: APPEND_MRECORD(MRecordActivationAndPowerMng)
        case PICMGREC_IP_CONNECTION           : APPEND_MRECORD(MRecordIPConnection)
        case PICMGREC_BOARD_P2PCON            : APPEND_MRECORD(MRecordBoardP2PCon)
        case PICMGREC_RADIAL_IPMB0_LINK_MAP   : break;
        case PICMGREC_FAN_GEOGRAPHY           : APPEND_MRECORD(MRecordFanMapping)
        case PICMGREC_FORM_FACTOR             : APPEND_MRECORD(MRecordFormFactor)
        case PICMGREC_LED_DESCRIPTOR          : APPEND_MRECORD(MRecordLEDDescriptor)

        default: {
            std::stringstream s;
            s << "invalid PICMG record id: " << static_cast<int>(picmg_record_id);
            errs.append(tag, "common", s.str());
            return false;
        }
        }
        break;
    }

    default: {
        std::stringstream s;
        s << "invalid IPMI record id: " << static_cast<int>(record_type);
        errs.append(tag, "common", s.str());
        return false;
    }
    }

    // lastly created mrecord
    mrecord = mrecords.back();
    return true;
}

//    ########     ###    ########   ######  #### ##    ##  ######
//    ##     ##   ## ##   ##     ## ##    ##  ##  ###   ## ##    ##
//    ##     ##  ##   ##  ##     ## ##        ##  ####  ## ##
//    ########  ##     ## ########   ######   ##  ## ## ## ##   ####
//    ##        ######### ##   ##         ##  ##  ##  #### ##    ##
//    ##        ##     ## ##    ##  ##    ##  ##  ##   ### ##    ##
//    ##        ##     ## ##     ##  ######  #### ##    ##  ######

template <typename T>
inline bool
AreaMRecords::tryParseImpl(T v, Errs &errs) {
    std::string err;
    bool        valid = true;

    for (size_t i = 0; i < v.size(); i++) {
        T entry = v[i];

        int record_id;
        if (!tryParseField_int(entry, "record_id", record_id, errs)) {
            valid = false;
            continue;
        }

        if (!isMRecordIDValid(record_id)) {
            std::stringstream s;
            s << "invalid id=" << record_id;
            errs.append(tag, "multirecord", s.str());
        }

        int  picmg_record_id = 0;
        bool is_PICMG_record = record_id == MRECORD_PICMG_RECORD;
        if (is_PICMG_record) {
            if (!tryParseField_int(entry, "picmg_record_id", picmg_record_id, errs)) {
                valid = false;
                continue;
            }

            if (!isPICMGMRecordIDValid(picmg_record_id)) {
                std::stringstream s;
                s << "invalid PICMG id=" << record_id;
                errs.append(tag, "multirecord", s.str());
            }
        }

        MRecord   record;
        std::byte type{static_cast<uchar>(record_id)};
        std::byte subtype{static_cast<uchar>(picmg_record_id)};
        if (!tryAppendMRecord(type, subtype, record, errs)) {
            valid = false;
            continue;
        }

        if (!record->tryParse(entry, errs)) {
            valid = false;
            continue;
        }
    }

    present = true;
    return valid;
}

bool
AreaMRecords::tryParse(nlohmann::json j, Errs &errs) {
    clear();

    return tryParseImpl(j, errs);
}

bool
AreaMRecords::tryParse(toml::value &t, Errs &errs) {
    clear();

    return tryParseImpl(t, errs);
}

bool
AreaMRecords::tryParseBinary(biterator begin, biterator end, Errs &errs) {
    while (true) {
        if (!validateMRecordHeader(begin, errs)) {
            return false;
        }

        MRecord record;
        if (!tryAppendMRecord(*begin, *(begin + 8), record, errs)) {
            return false;
        }

        biterator record_end = begin + static_cast<int>(*(begin + 2));
        if (!record->tryParseBinary(begin, record_end, errs)) {
            return false;
        }

        // end of fields
        if (static_cast<int>(*(begin + 1)) & 128) {
            present = true;
            return true;
        }

        begin += record->getLength();
        if (begin >= end) {
            errs.append(tag, "common", "parsing went out of bounds");
            return false;
        }
    }

    return false;
}

//    ######## ##     ## #### ######## #### ##    ##  ######
//    ##       ###   ###  ##     ##     ##  ###   ## ##    ##
//    ##       #### ####  ##     ##     ##  ####  ## ##
//    ######   ## ### ##  ##     ##     ##  ## ## ## ##   ####
//    ##       ##     ##  ##     ##     ##  ##  #### ##    ##
//    ##       ##     ##  ##     ##     ##  ##   ### ##    ##
//    ######## ##     ## ####    ##    #### ##    ##  ######

void
AreaMRecords::emitJSON(nlohmann::json &j) {
    json jarray;

    for (size_t i = 0; i < mrecords.size(); i++) {
        MRecord mr = mrecords[i];
        json    jentry;

        mr->emitJSON(jentry);
        jarray[i] = jentry;
    }

    j = jarray;
}

void
AreaMRecords::emitTOML(toml::table &t) {
    if (!present) {
        return;
    }

    toml::array tarr;

    for (auto &&mr : mrecords) {
        toml::table tentry;
        mr->emitTOML(tentry);
        tarr.push_back(toml::value(std::move(tentry)));
    }

    t[getTag()] = toml::value(std::move(tarr));
}

bool
AreaMRecords::emitBinary(bytes &out_bin, Errs &errs) {
    bool  valid = true;
    bytes bs;

    for (size_t i = 0; i < mrecords.size(); i++) {
        MRecord mr = mrecords[i];
        bytes   bs;

        if (!mr->emitBinary(bs, errs)) {
            valid = false;
        }

        // last one - make eol
        if (i == mrecords.size() - 1) {
            bs[HDR_OFFSET_EOL]    |= std::byte{128};
            bs[HDR_OFFSET_HDR_CS]  = calcZeroChecksum(bs.begin(), bs.begin() + 3);
        }

        APPEND_BYTES(out_bin, bs);
    }

    return valid;
}

//    #### ##    ## #### ########
//     ##  ###   ##  ##     ##
//     ##  ####  ##  ##     ##
//     ##  ## ## ##  ##     ##
//     ##  ##  ####  ##     ##
//     ##  ##   ###  ##     ##
//    #### ##    ## ####    ##

AreaMRecords::AreaMRecords(/* args */) : Section("multi_records", "MultiRecords") {
    //
}
