#pragma once

#include "mRecordBase.hpp"

class MRecordBackplaneP2PCon : public MRecordBase {
  private:
    const size_t SLOT_BASE_BYTE_LEN = 3; // type + address + channels count
    const size_t CHANNEL_BYTE_LEN   = 3;

    /* CHANNEL DESCRIPTOR */
    struct ChannelDescr {
        uchar local_channel;
        uchar remote_channel;
        uchar remote_slot;
    };

    template <typename T>
    bool tryParseChannelDescrImpl(T v, ChannelDescr &cd, Errs &errs);
    bool tryParseChannelDescr(biterator &begin, biterator end, ChannelDescr &cd, Errs &errs);

    void emitChannelDescr(nlohmann::json &j, ChannelDescr &cd);
    void emitChannelDescr(toml::table &t, ChannelDescr &cd);
    void emitChannelDescr(bytes &out_bin, ChannelDescr &cd);

    /* SLOT DESCRIPTOR */
    struct SlotDescriptor {
        uchar                     type;
        uchar                     address;
        std::vector<ChannelDescr> channels;
    };

    template <typename T>
    bool tryParseSlotDescrImpl(T v, SlotDescriptor &sd, Errs &errs);
    bool tryParseSlotDescr(biterator &begin, biterator end, SlotDescriptor &sd, Errs &errs);

    void emitSlotDescr(nlohmann::json &j, SlotDescriptor &sd);
    void emitSlotDescr(toml::table &t, SlotDescriptor &sd);
    void emitSlotDescr(bytes &out_bin, SlotDescriptor &sd);

    /* BASE */
    std::vector<SlotDescriptor> slots;

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

    MRecordBackplaneP2PCon();
};
