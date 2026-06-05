#pragma once

#include <string>
#include <tuple>
#include <unordered_map>
#include <variant>

#include "encoding.hpp"
#include "errs.hpp"
#include "json.hpp"
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

    bool      tryEncodeStr(std::string ftag, encodedStr str, bytes &outb, Errs &errs);
    bool      tryDecodeStr(bytes::iterator &inb, std::string ftag, encodedStr &str, Errs &errs);
    bool      tryDecodeStr(nlohmann::json j, std::string ftag, encodedStr &str, Errs &errs);
    std::byte calcZeroChecksum(bytes bs);
    std::byte calcZeroChecksum(bytes::iterator begin, bytes::iterator end);

    bool tryParseFieldJSON_bool(json j, bool &val, std::string &err);
    bool tryParseFieldJSON_int(json j, int &val, std::string &err);
    bool tryParseFieldJSON_obj(json j, json &val, std::string &err);
    bool tryParseFieldJSON_arr(json j, jarray &val, std::string &err);
    bool tryParseFieldJSON_str(json j, std::string &val, std::string &err);
    bool tryParseFieldJSON_encStr(json j, encodedStr &val, std::string &err);

  public:
    std::string getTag();

    virtual bool validate() = 0; // MAYBE UNUSED

    virtual bool tryParseJSON(nlohmann::json j, Errs &errs)         = 0;
    virtual bool tryParseTOML()                                     = 0;
    virtual bool tryParseBinary(bytes::iterator in_bin, Errs &errs) = 0;

    virtual void emitJSON(nlohmann::json &j)            = 0;
    virtual void emitTOML()                             = 0;
    virtual bool emitBinary(bytes &out_bin, Errs &errs) = 0;

    virtual size_t getByteLen() = 0; // MAYBE UNUSED
    virtual void   clear()      = 0;

    Section(std::string tag, std::string label);
    ~Section();
};
