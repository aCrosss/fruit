#pragma once

#include "section.hpp"

#define IPMI_HEADER_LEN  5
#define PICMG_HEADER_LEN IPMI_HEADER_LEN + 5

#define MREC_DEF_FORMAT_VER (0x02)

#define DR_BYTE(v) static_cast<uchar>(*(v))
#define DR_INT(v)  static_cast<int>(*(v))
#define SC_I(v)    static_cast<int>(v)
#define CHECK_BOUNDS(v, min, max, errs)                                                    \
    if (v < min || v > max) {                                                              \
        std::stringstream s;                                                               \
        s << "must be in range [" << static_cast<int>(min) << ":" << static_cast<int>(max) \
          << "]";                                                                          \
        errs.append(tag, #v, s.str());                                                     \
        valid = false;                                                                     \
    }

enum MRType {
    MR_IMPI,
    MR_PICMG,
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

    void buildMRecordHeader(bytes &out, bytes &data);
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
};
