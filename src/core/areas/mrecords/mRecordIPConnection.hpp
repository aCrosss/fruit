#pragma once

#include "mRecordBase.hpp"

class MRecordIPConnection : public MRecordBase {
  private:
    union IP {
        std::byte bytes[4];
        uchar     raw[4];
    };

    void bytesToIP(biterator begin, IP &ip);
    bool strToIP(std::string s, IP &ip, std::string &err);
    void IPtoStr(IP ip, std::string &out);

    IP ip_address;
    IP gateway_address;
    IP subnet_mask;

    void debug_printOutVals();
    void clear() override;

  public:
    uchar getLength() override;

    template <typename T>
    bool tryParseImpl(T v, Errs &errs);
    bool tryParse(nlohmann::json j, Errs &errs) override;
    bool tryParse(toml::value &t, Errs &errs) override;
    bool tryParseBinary(biterator begin, biterator end, Errs &errs) override;

    void emitJSON(nlohmann::json &j) override;
    void emitTOML(toml::table &t) override;
    bool emitBinary(bytes &out_bin, Errs &errs) override;

    MRecordIPConnection();
};
