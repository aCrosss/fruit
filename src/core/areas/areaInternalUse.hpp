#include "section.hpp"

class AreaInternalUse : public Section {
  private:
    // 1 header header byte
    const uchar const_len = 1;

    bytes internal_data; // "data" taqg used

    void debug_printOutVals();

  public:
    uchar getLength() override;
    void  clear() override;

    template <typename T>
    bool tryParseImpl(T v, Errs &errs);
    bool tryParse(nlohmann::json j, Errs &errs) override;
    bool tryParse(toml::value &t, Errs &errs) override;
    bool tryParseBinary(biterator begin, biterator end, Errs &errs) override;

    void emitJSON(nlohmann::json &j) override;
    void emitTOML(toml::table &t) override;
    bool emitBinary(bytes &out_bin, Errs &errs) override;

    AreaInternalUse();
    ~AreaInternalUse();
};
