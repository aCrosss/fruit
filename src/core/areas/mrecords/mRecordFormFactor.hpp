#pragma once

#include "mRecordBase.hpp"

class MRecordFormFactor : public MRecordBase {
  protected:
    void debug_printOutVals();
    void clear() override;

    enum FormFactor {
        FFACTOR_ATCA,
        FFACTOR_ATCA300,
        FFACTOR_MicroTCA,
        FFACTOR_UNDEFINED,
    };

    bool isFormFactorValid(uchar ff);

    std::string FormFactorToStr(FormFactor ff);
    FormFactor  StrToFormFactor(std::string s);

    FormFactor form_factor;

  public:
    uchar getLength() override;

    template <typename T>
    bool tryParseImpl(T v, Errs &errs);
    bool tryParse(nlohmann::json j, Errs &errs) override;
    bool tryParse(toml::value &t, Errs &errs) override;
    bool tryParseBinary(biterator begin, biterator end, Errs &errs) override;

    void emitJSON(nlohmann::json &j) override;
    void emitTOML(toml::table &t) override;
    bool emitBinary(bytes &out_bin, bool eol, Errs &errs) override;

    MRecordFormFactor();
};
