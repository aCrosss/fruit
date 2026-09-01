#pragma once

#include "mRecordBase.hpp"

class MRecordActivationAndPowerMng : public MRecordBase {
  private:
    uchar allowance_for_activation;

    struct ActivationAndPwrDescr {
        uchar          hardware_address;
        uchar          fru_device_id;
        unsigned short max_fru_power_cap;
        // so, 1 is disabled and 0 is enabled... I just hate them
        bool           controlled_deactivation;
        bool           controlled_activation;
        uchar          next_power_on_delay;
    };

    template <typename T>
    bool tryParseDescrImpl(T v, ActivationAndPwrDescr &d, Errs &errs);
    bool tryParseDescr(biterator &begin, ActivationAndPwrDescr &d, Errs &errs);

    std::vector<ActivationAndPwrDescr> entries;

    void emitEntry(bytes &out_bin, ActivationAndPwrDescr &entry);

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
    bool emitBinary(bytes &out_bin, bool eol, Errs &errs) override;

    MRecordActivationAndPowerMng();
};
