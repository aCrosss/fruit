#pragma once

#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <variant>

#include "encoding.hpp"
#include "errs.hpp"
#include "json.hpp"
#include "toml.hpp"
#include "types.hpp"

using namespace nlohmann;

#define FILED_MISSING_GUARD(v)                      \
    if (!v.contains(ftag)) {                        \
        errs.append(tag, ftag, "field is missing"); \
        return false;                               \
    }

#define ROUND_LEN_TO_8_BYTES_MULTPL(len) ((len + 7) / 8)
#define IPMI_TO_REAL_LEN(len)            (len * 8)

typedef std::vector<json>              jarray;
typedef std::variant<int, std::string> fieldVal;

enum Areas {
    AREA_COMMON_HEADER,
    AREA_INTERNAL_USE,
    AREA_CHASSIS_INFO,
    AREA_BOARD_INFO,
    AREA_PRODUCT_INFO,
    AREA_MULTIRECORD,
};

struct encodedStr {
    std::string str;
    Encoding    enc;
};

class Section {
  protected:
    std::string tag;
    std::string label;
    bool        present;

    virtual void clear() = 0;

    bool tryEncodeStr(std::string ftag, encodedStr str, bytes &outb, Errs &errs);
    bool tryDecodeStr(biterator &inb, std::string ftag, encodedStr &str, Errs &errs);

    std::byte calcZeroChecksum(bytes bs);
    std::byte calcZeroChecksum(biterator begin, biterator end);
    bool      checkChecksums(biterator cs1p, biterator cs2beg, biterator cs2end, Errs &errs);

    bool tryParseField_bool(json j, std::string ftag, bool &val, Errs &errs);
    bool tryParseField_bool(toml::value t, std::string ftag, bool &val, Errs &errs);

    bool tryParseField_int(json j, std::string ftag, int &val, Errs &errs);
    bool tryParseField_int(toml::value t, std::string ftag, int &val, Errs &errs);

    bool tryParseField_obj(json j, std::string ftag, json &val, Errs &errs);
    bool tryParseField_obj(toml::value t, std::string ftag, toml::value &val, Errs &errs);

    bool tryParseField_arr(json j, std::string ftag, json &val, Errs &errs);
    bool tryParseField_arr(toml::value t, std::string ftag, toml::value &val, Errs &errs);

    bool tryParseField_str(json j, std::string ftag, std::string &val, Errs &errs);
    bool tryParseField_str(toml::value t, std::string ftag, std::string &val, Errs &errs);

    template <typename T>
    bool tryParseEncStrImpl(T v, std::string ftag, encodedStr &val, Errs &errs);
    bool tryParseField_encStr(json j, std::string ftag, encodedStr &val, Errs &errs);
    bool tryParseField_encStr(toml::value t, std::string ftag, encodedStr &val, Errs &errs);
    bool
    tryParseField_encStrArr(json j, std::string ftag, std::vector<encodedStr> &val, Errs &errs);
    bool tryParseField_encStrArr(toml::value              t,
                                 std::string              ftag,
                                 std::vector<encodedStr> &val,
                                 Errs                    &errs);

  public:
    std::string getTag();

    virtual uchar getLength() = 0;

    virtual bool tryParseJSON(nlohmann::json j, Errs &errs)                 = 0;
    virtual bool tryParseTOML(toml::value &t, Errs &errs)                   = 0;
    virtual bool tryParseBinary(biterator begin, biterator end, Errs &errs) = 0;

    virtual void emitJSON(nlohmann::json &j)            = 0;
    virtual void emitTOML()                             = 0;
    virtual bool emitBinary(bytes &out_bin, Errs &errs) = 0;

    Section(std::string tag, std::string label);
    virtual ~Section() = default;
};
