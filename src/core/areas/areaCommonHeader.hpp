#include "section.hpp"

class AreaCommonHeader : public Section {
  private:
    // 1 header header byte + 5 offset bytes + 1 pad byte + 1 checkusm byte
    const uchar const_len = 8;

    std::vector<uchar> offsets = {0};

    void debug_printOutVals();

  public:
    uchar getLength() override;
    void  clear() override;

    uchar getOffset(Areas offset_ind);
    uchar getNextOffset(Areas offset_ind);
    bool  setOffsets(std::vector<std::unique_ptr<Section>> &sections);

    // template <typename T>
    // bool tryParseImpl(T v, Errs &errs);
    bool tryParseJSON(nlohmann::json j, Errs &errs) override;
    bool tryParseTOML(toml::value &t, Errs &errs) override;
    bool tryParseBinary(biterator begin, biterator end, Errs &errs) override;

    void emitJSON(nlohmann::json &j) override;
    void emitTOML() override;
    bool emitBinary(bytes &out_bin, Errs &errs) override;

    AreaCommonHeader();
    ~AreaCommonHeader();
};
