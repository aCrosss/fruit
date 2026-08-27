#pragma once

#include "mRecordBase.hpp"

class MRecordBoardP2PCon : public MRecordBase {
  private:
    void debug_printOutVals();
    void clear() override;

    enum Interface {
        IFACE_BASE     = 0b00,
        IFACE_FABRIC   = 0b01,
        IFACE_UCHANNEL = 0b10,
        IFACE_RESERVED = 0b11,
    };

    std::string IFaceToStr(Interface ifac);

    std::vector<bytes> guids;

    bool strToGUID(std::string s, bytes &guid, std::string err);
    void GUIDToStr(bytes &guid, std::string &out);

    struct LinkDescriptor {
        uchar link_grouping_id;
        uchar link_type_extension;
        uchar link_type;

        // link_designator;
        bool      port_3;
        bool      port_2;
        bool      port_1;
        bool      port_0;
        Interface iface;
        uchar     ch_number;
    };

    template <typename T>
    bool tryParseLDescriptor(T v, LinkDescriptor &ld, Errs &errs);
    bool tryParseLDescriptor(biterator begin, LinkDescriptor &ld, Errs &errs);
    void emitLDescriptor(nlohmann::json &j, LinkDescriptor &ld);
    void emitLDescriptor(toml::table &t, LinkDescriptor &ld);
    void emitLDescriptor(bytes &out_bin, LinkDescriptor &ld);

    std::vector<LinkDescriptor> link_descriptors;

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

    MRecordBoardP2PCon();
};
