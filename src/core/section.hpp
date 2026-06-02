#pragma once

#include <string>
#include <tuple>
#include <unordered_map>
#include <variant>

#include "encoding.hpp"
#include "fru_errs.hpp"
#include "json.hpp"
#include "types.hpp"

using namespace nlohmann;

typedef std::vector<json>              jarray;
typedef std::variant<int, std::string> fieldVal;

struct encodedStr {
    std::string str;
    Encoding    enc;
};

class Section {
  protected:
    std::string tag;
    std::string label;
    bool        present;

    bool      tryEncodeStr(std::string ftag, encodedStr str, bytes &outb, FRU_errs &errs);
    std::byte calcZeroChecksum(bytes bs);

    bool tryParseFieldJSON_bool(json j, bool &val, std::string &err);
    bool tryParseFieldJSON_int(json j, int &val, std::string &err);
    bool tryParseFieldJSON_obj(json j, json &val, std::string &err);
    bool tryParseFieldJSON_arr(json j, jarray &val, std::string &err);
    bool tryParseFieldJSON_str(json j, std::string &val, std::string &err);
    bool tryParseFieldJSON_encStr(json j, encodedStr &val, std::string &err);

  public:
    virtual bool   validate()                                     = 0;
    virtual bool   tryParseJSON(nlohmann::json j, FRU_errs &errs) = 0;
    virtual bool   tryParseTOML()                                 = 0;
    virtual bool   emmitBinary(bytes &out_bin, FRU_errs &errs)    = 0;
    virtual bool   decodeBinary(bytes &out_bin)                   = 0;
    virtual size_t getByteLen()                                   = 0;

    Section(std::string tag, std::string label);
    ~Section();
};
