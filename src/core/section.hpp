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
    bool tryDecodeStr(nlohmann::json &j, std::string ftag, encodedStr &str, Errs &errs);
    bool tryDecodeStr(toml::value &t, std::string ftag, encodedStr &str, Errs &errs);

    std::byte calcZeroChecksum(bytes bs);
    std::byte calcZeroChecksum(biterator begin, biterator end);
    bool      checkChecksums(biterator cs1p, biterator cs2beg, biterator cs2end, Errs &errs);

    bool tryParseField_bool(json j, bool &val, std::string &err);
    bool tryParseField_int(json j, int &val, std::string &err);
    bool tryParseField_obj(json j, json &val, std::string &err);
    bool tryParseField_arr(json j, json &val, std::string &err);
    bool tryParseField_str(json j, std::string &val, std::string &err);
    bool tryParseField_encStr(json j, encodedStr &val, std::string &err);

    bool tryParseField_bool(toml::value t, bool &val, std::string &err);
    bool tryParseField_int(toml::value t, int &val, std::string &err);
    bool tryParseField_obj(toml::value t, toml::value &val, std::string &err);
    bool tryParseField_arr(toml::value t, toml::value &val, std::string &err);
    bool tryParseField_str(toml::value t, std::string &val, std::string &err);
    bool tryParseField_encStr(toml::value t, encodedStr &val, std::string &err);

  public:
    std::string getTag();

    virtual bool tryParseJSON(nlohmann::json j, Errs &errs)   = 0;
    virtual bool tryParseTOML(toml::value &t, Errs &errs)     = 0;
    virtual bool tryParseBinary(biterator in_bin, Errs &errs) = 0;

    virtual void emitJSON(nlohmann::json &j)            = 0;
    virtual void emitTOML()                             = 0;
    virtual bool emitBinary(bytes &out_bin, Errs &errs) = 0;

    Section(std::string tag, std::string label);
    ~Section();
};
