#pragma once

#include <string>
#include <tuple>
#include <unordered_map>
#include <variant>

#include "types.hpp"

typedef std::variant<int, std::string> fieldVal;

struct field {
    bool     optional;
    valType  type;
    fieldVal val;
};

class Section {
  private:
    std::string tag;
    std::string label;
    bool        present;

    std::unordered_map<std::string, field> fields;

  public:
    virtual bool   validate() const                  = 0;
    virtual bool   tryParseJSON() const              = 0;
    virtual bool   tryParseTOML() const              = 0;
    virtual void   emmitBinary(bytes &out_bin) const = 0;
    virtual size_t getByteLen() const                = 0;

    template <typename T>
    T getVal(std::string field) const {
        T f = fields.find(field);
        if (!std::holds_alternative<T>(f.val)) {
            throw std::bad_variant_access();
        }
        return std::get<T>(f.val);
    }

    Section(std::string tag, std::string label);
    ~Section();
};
