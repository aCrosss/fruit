#include "section.hpp"

class AreaProductInfo : public Section {
  private:
    // 1 header byte + 1 length byte + 1 lang code byte
    // + 1 end of fields byte + 1 checkusm byte
    const uchar const_len = 5;

    int                     language_code;
    encodedStr              manufacturer;
    encodedStr              product_name;
    encodedStr              part;
    encodedStr              version;
    encodedStr              serial_number;
    encodedStr              asset_tag;
    encodedStr              fru_file_id;
    std::vector<encodedStr> custom;

    void debug_printOutVals();

  public:
    uchar getLength() override;
    void  clear() override;

    template <typename T>
    bool tryParseImpl(T v, Errs &errs);
    bool tryParseJSON(nlohmann::json j, Errs &errs) override;
    bool tryParseTOML(toml::value &t, Errs &errs) override;
    bool tryParseBinary(biterator begin, biterator end, Errs &errs) override;

    void emitJSON(nlohmann::json &j) override;
    void emitTOML(toml::table &t) override;
    bool emitBinary(bytes &out_bin, Errs &errs) override;

    AreaProductInfo(/* args */);
    ~AreaProductInfo();
};
