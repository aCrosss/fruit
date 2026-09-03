#pragma once

#include "mRecordBase.hpp"

class MRecordRadialIPMB0Topology : public MRecordBase {
  private:
    /* CONSTS */

    enum BusCoverage {
        BUS_COVERAGE_IPMB_MIN  = 0b00,
        BUS_COVERAGE_IPMB_A    = 0b01,
        BUS_COVERAGE_IPMB_B    = 0b10,
        BUS_COVERAGE_IPMB_BOTH = 0b11,
        BUS_COVERAGE_IPMB_MAX  = 0b100,
    };

    const std::map<BusCoverage, std::string> bus_coverage_to_str = {
        {BUS_COVERAGE_IPMB_A, "ipmb_a"},
        {BUS_COVERAGE_IPMB_B, "ipmb_b"},
        {BUS_COVERAGE_IPMB_BOTH, "ipmb_both"}};

    const uchar connector_definer_len = 3;
    bytes       connector_definer;

    const uchar connector_version_id_len = 2;
    bytes       connector_version_id;

    const uchar ipmb0_link_entry_min = 1;
    const uchar ipmb0_link_entry_max = 95;

    /* LINK MAPPINS */

    struct MappingEntry {
        uchar hardware_address;
        uchar ipmb0_link_entry;
    };

    template <typename T>
    bool tryParseLinkMappingImpl(T v, MappingEntry &hd, Errs &errs);
    bool tryParseLinkMapping(biterator &begin, biterator end, MappingEntry &hd);

    void emitLinkMapping(nlohmann::json &j, MappingEntry &hd);
    void emitLinkMapping(toml::table &t, MappingEntry &hd);
    void emitLinkMapping(bytes &out_bin, MappingEntry &hd);

    /* HUB DESCRIPTORS */

    struct HubDescriptor {
        uchar       hardware_address;
        BusCoverage bus_coverage;

        std::vector<MappingEntry> link_mappings;
    };
    std::vector<HubDescriptor> hub_descriptors;

    template <typename T>
    bool tryParseHubDescriptorImpl(T v, HubDescriptor &hd, Errs &errs);
    bool tryParseHubDescriptor(biterator &begin, biterator end, HubDescriptor &hd);

    void emitHubDescriptor(nlohmann::json &j, HubDescriptor &hd);
    void emitHubDescriptor(toml::table &t, HubDescriptor &hd);
    void emitHubDescriptor(bytes &out_bin, HubDescriptor &hd);

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

    MRecordRadialIPMB0Topology();
};
