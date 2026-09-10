#include <cstddef>
#include <cstring>
#include <sstream>
#include <string>

#include "encoding.hpp"
#include "mRecordBase.hpp"
#include "mRecordRadialIPMB0Topology.hpp"
#include "types.hpp"

//    ##        #######   ######     ###    ##
//    ##       ##     ## ##    ##   ## ##   ##
//    ##       ##     ## ##        ##   ##  ##
//    ##       ##     ## ##       ##     ## ##
//    ##       ##     ## ##       ######### ##
//    ##       ##     ## ##    ## ##     ## ##
//    ########  #######   ######  ##     ## ########

#define PARSE_STR_TO_BYTES(bytes, expected_len, errs)                                   \
    {                                                                                   \
        std::string buff;                                                               \
        if (tryParseField_str(v, #bytes, buff, errs)) {                                 \
            std::string err;                                                            \
            if (!hexStrToBytes(buff, bytes, err)) {                                     \
                errs.append(tag, #bytes, err);                                          \
                valid = false;                                                          \
            } else {                                                                    \
                if (bytes.size() != expected_len) {                                     \
                    std::stringstream ss;                                               \
                    ss << "expected " << expected_len << "bytes, got " << bytes.size(); \
                    errs.append(tag, #bytes, ss.str());                                 \
                    valid = false;                                                      \
                }                                                                       \
            }                                                                           \
        } else {                                                                        \
            valid = false;                                                              \
        }                                                                               \
    }

void
MRecordRadialIPMB0Topology::debug_printOutVals() {
    std::cout << "=== " << label << " ===" << std::endl;
    std::cout << "record_id:       " << record_id << std::endl;
    std::cout << "picmg_record_id: " << picmg_record_id << std::endl;

    for (size_t i = 0; i < hub_descriptors.size(); ++i) {
        auto &hd = hub_descriptors[i];

        std::cout << "hub_descriptor[" << i << "]:" << std::endl;
        std::cout << "  hardware_address   : " << SC_I(hd.hardware_address) << std::endl;
        std::string bus_coverage = bus_coverage_to_str.at(hd.bus_coverage);
        std::cout << "  bus_coverage       : " << bus_coverage << std::endl;

        for (size_t j = 0; j < hd.link_mappings.size(); ++j) {
            auto &l = hd.link_mappings[j];

            std::cout << "  link_mappings[" << j << "]:" << std::endl;
            std::cout << "    hardware_address: " << SC_I(l.hardware_address) << std::endl;
            std::cout << "    ipmb0_link_entry: " << SC_I(l.ipmb0_link_entry) << std::endl;
        }
    }
}

void
MRecordRadialIPMB0Topology::clear() {
    connector_definer.clear();
    connector_version_id.clear();
    hub_descriptors.clear();
}

uchar
MRecordRadialIPMB0Topology::getLength() {
    uchar len  = PICMG_HEADER_LEN;
    len       += 3; // connector definer bytes
    len       += 2; // connector version id bytes
    len       += 1; // hub descriptors count

    for (size_t i = 0; i < hub_descriptors.size(); ++i) {
        len += 1; // hardware address
        len += 1; // hub info byte
        len += 1; // address entry count
        len += hub_descriptors[i].link_mappings.size() * 2;
    }

    return len;
}

// #      # #    # #    #    #    #   ##   #####  #####  # #    #  ####   ####
// #      # ##   # #   #     ##  ##  #  #  #    # #    # # ##   # #    # #
// #      # # #  # ####      # ## # #    # #    # #    # # # #  # #       ####
// #      # #  # # #  #      #    # ###### #####  #####  # #  # # #  ###      #
// #      # #   ## #   #     #    # #    # #      #      # #   ## #    # #    #
// ###### # #    # #    #    #    # #    # #      #      # #    #  ####   ####

template <typename T>
bool
MRecordRadialIPMB0Topology::tryParseLinkMappingImpl(T v, MappingEntry &me, Errs &errs) {
    bool valid = true;

    uchar hardware_address;
    uchar ipmb0_link_entry;

    if (!tryParseField_uchar(v, "hardware_address", hardware_address, errs)) {
        valid = false;
    }

    if (!tryParseField_uchar(v, "ipmb0_link_entry", ipmb0_link_entry, errs)) {
        valid = false;
    }

    if (!valid) {
        return false;
    }

    me.hardware_address = hardware_address;
    me.ipmb0_link_entry = ipmb0_link_entry;

    return true;
}

bool
MRecordRadialIPMB0Topology::tryParseLinkMapping(ibytes       &begin,
                                                ibytes        end,
                                                MappingEntry &me,
                                                Errs         &errs) {
    OUT_OF_BOUNDS_GUARD_OFFSET("link_mappings", LINK_MAPPING_BYTE_LEN)

    me.hardware_address = DR_BYTE(begin + 0);
    me.ipmb0_link_entry = DR_BYTE(begin + 1);

    begin += LINK_MAPPING_BYTE_LEN;
    return true;
}

void
MRecordRadialIPMB0Topology::emitLinkMapping(nlohmann::json &j, MappingEntry &me) {
    j["local_channel"]  = me.hardware_address;
    j["remote_channel"] = me.ipmb0_link_entry;
}

void
MRecordRadialIPMB0Topology::emitLinkMapping(toml::table &t, MappingEntry &me) {
    t["local_channel"]  = toml::value(me.hardware_address);
    t["remote_channel"] = toml::value(me.ipmb0_link_entry);
}

void
MRecordRadialIPMB0Topology::emitLinkMapping(bytes &out_bin, MappingEntry &me) {
    out_bin.emplace_back(std::byte{me.hardware_address});
    out_bin.emplace_back(std::byte{me.ipmb0_link_entry});
}

//   #    # #    # #####     #####  ######  ####   ####  #####  # #####  #####  ####  #####
//   #    # #    # #    #    #    # #      #      #    # #    # # #    #   #   #    # #    #
//   ###### #    # #####     #    # #####   ####  #      #    # # #    #   #   #    # #    #
//   #    # #    # #    #    #    # #           # #      #####  # #####    #   #    # #####
//   #    # #    # #    #    #    # #      #    # #    # #   #  # #        #   #    # #   #
//   #    #  ####  #####     #####  ######  ####   ####  #    # # #        #    ####  #    #

template <typename T>
bool
MRecordRadialIPMB0Topology::tryParseHubDescriptorImpl(T v, HubDescriptor &hd, Errs &errs) {
    bool valid = true;

    uchar hardware_address;

    if (!tryParseField_uchar(v, "hardware_address", hardware_address, errs)) {
        valid = false;
    }

    std::string bus_coverage_str;
    if (tryParseField_str(v, "bus_coverage", bus_coverage_str, errs)) {
        if (strcmp(bus_coverage_str.c_str(), "ipmb_a") == 0) {
            hd.bus_coverage = BUS_COVERAGE_IPMB_A;
        } else if (strcmp(bus_coverage_str.c_str(), "ipmb_b") == 0) {
            hd.bus_coverage = BUS_COVERAGE_IPMB_B;
        } else if (strcmp(bus_coverage_str.c_str(), "ipmb_both") == 0) {
            hd.bus_coverage = BUS_COVERAGE_IPMB_BOTH;
        } else {
            std::stringstream ss;
            ss << "unknown bus coverage type: " << bus_coverage_str;
            errs.append(tag, "bus_coverage", ss.str());
            valid = false;
        }
    } else {
        valid = false;
    }

    hd.hardware_address = hardware_address;

    T array;
    if (!tryParseField_arr(v, "link_mappings", array, errs)) {
        return false;
    }

    for (size_t i = 0; i < array.size(); ++i) {
        T            entry = array[i];
        MappingEntry me;

        if (!tryParseLinkMappingImpl(entry, me, errs)) {
            valid = false;
            continue;
        }

        hd.link_mappings.push_back(me);
    }

    return valid;
}

bool
MRecordRadialIPMB0Topology::tryParseHubDescriptor(ibytes        &begin,
                                                  ibytes         end,
                                                  HubDescriptor &hd,
                                                  Errs          &errs) {
    OUT_OF_BOUNDS_GUARD_OFFSET("hub_descriptors", HUB_DESCRIPTOR_BASE_BYTE_LEN)

    hd.hardware_address = DR_BYTE(begin + 0);

    uchar bus_coverage = DR_BYTE(begin + 1) & MASK_2b;
    if (bus_coverage == BUS_COVERAGE_IPMB_MIN || bus_coverage >= BUS_COVERAGE_IPMB_MAX) {
        return false;
    }
    hd.bus_coverage = BusCoverage(bus_coverage);

    uchar count  = DR_BYTE(begin + 2);
    begin       += HUB_DESCRIPTOR_BASE_BYTE_LEN;
    for (uchar i = 0; i < count; ++i) {
        MappingEntry me;
        if (!tryParseLinkMapping(begin, end, me, errs)) {
            return false;
        }

        hd.link_mappings.push_back(me);
    }

    return true;
}

void
MRecordRadialIPMB0Topology::emitHubDescriptor(nlohmann::json &j, HubDescriptor &hd) {
    j["hardware_address"] = hd.hardware_address;
    j["bus_coverage"]     = bus_coverage_to_str.at(hd.bus_coverage);

    if (hd.link_mappings.size() == 0) {
        return;
    }

    nlohmann::json jarray;
    for (size_t i = 0; i < hd.link_mappings.size(); ++i) {
        nlohmann::json jentry;
        emitLinkMapping(jentry, hd.link_mappings[i]);
        jarray[i] = jentry;
    }

    j["link_mappings"] = jarray;
}

void
MRecordRadialIPMB0Topology::emitHubDescriptor(toml::table &t, HubDescriptor &hd) {
    t["hardware_address"] = toml::value(hd.hardware_address);
    t["bus_coverage"]     = toml::value(bus_coverage_to_str.at(hd.bus_coverage));

    if (hd.link_mappings.size() == 0) {
        return;
    }

    toml::array tarray;
    for (size_t i = 0; i < hd.link_mappings.size(); ++i) {
        toml::table tentry;
        emitLinkMapping(tentry, hd.link_mappings[i]);
        tarray.push_back(toml::value(std::move(tentry)));
    }

    t["link_mappings"] = toml::value(std::move(tarray));
}

void
MRecordRadialIPMB0Topology::emitHubDescriptor(bytes &out_bin, HubDescriptor &hd) {
    out_bin.emplace_back(std::byte{hd.hardware_address});

    uchar bus_coverage = static_cast<uchar>(hd.bus_coverage & MASK_2b);
    out_bin.emplace_back(std::byte{bus_coverage});

    uchar count = static_cast<uchar>(hd.link_mappings.size());
    out_bin.emplace_back(std::byte{count});
    for (auto &&i : hd.link_mappings) {
        emitLinkMapping(out_bin, i);
    }
}

//    ########     ###    ########   ######  #### ##    ##  ######
//    ##     ##   ## ##   ##     ## ##    ##  ##  ###   ## ##    ##
//    ##     ##  ##   ##  ##     ## ##        ##  ####  ## ##
//    ########  ##     ## ########   ######   ##  ## ## ## ##   ####
//    ##        ######### ##   ##         ##  ##  ##  #### ##    ##
//    ##        ##     ## ##    ##  ##    ##  ##  ##   ### ##    ##
//    ##        ##     ## ##     ##  ######  #### ##    ##  ######

template <typename T>
bool
MRecordRadialIPMB0Topology::tryParseImpl(T v, Errs &errs) {
    bool valid = true;

    PARSE_STR_TO_BYTES(connector_definer, connector_definer_len, errs);
    PARSE_STR_TO_BYTES(connector_version_id, connector_version_id_len, errs);

    T array;
    if (!tryParseField_arr(v, "hub_descriptors", array, errs)) {
        return false;
    }

    for (size_t i = 0; i < array.size(); ++i) {
        T             entry = array[i];
        HubDescriptor hd;

        if (!tryParseHubDescriptorImpl(entry, hd, errs)) {
            valid = false;
            continue;
        }

        hub_descriptors.push_back(hd);
    }

    debug_printOutVals();
    return valid;
}

bool
MRecordRadialIPMB0Topology::tryParse(nlohmann::json j, Errs &errs) {
    return tryParseImpl(j, errs);
}

bool
MRecordRadialIPMB0Topology::tryParse(toml::value &t, Errs &errs) {
    return tryParseImpl(t, errs);
}

bool
MRecordRadialIPMB0Topology::tryParseBinary(ibytes begin, ibytes end, Errs &errs) {
    OUT_OF_BOUNDS_GUARD_OFFSET("common", CONST_BYTE_LEN)

    begin += PICMG_HEADER_LEN;

    connector_definer.push_back(std::byte{DR_BYTE(begin + 0)});
    connector_definer.push_back(std::byte{DR_BYTE(begin + 1)});
    connector_definer.push_back(std::byte{DR_BYTE(begin + 2)});

    connector_version_id.push_back(std::byte{DR_BYTE(begin + 3)});
    connector_version_id.push_back(std::byte{DR_BYTE(begin + 4)});

    uchar count  = DR_BYTE(begin + CONNECTOR_INFO_BYE_LEN);
    begin       += CONNECTOR_INFO_BYE_LEN + 1; /*+1 count byte*/
    for (uchar i = 0; i < count; i++) {
        HubDescriptor hd;
        if (!tryParseHubDescriptor(begin, end, hd, errs)) {
            return false;
        }

        hub_descriptors.push_back(hd);
    }

    debug_printOutVals();
    return true;
}

//    ######## ##     ## #### ######## #### ##    ##  ######
//    ##       ###   ###  ##     ##     ##  ###   ## ##    ##
//    ##       #### ####  ##     ##     ##  ####  ## ##
//    ######   ## ### ##  ##     ##     ##  ## ## ## ##   ####
//    ##       ##     ##  ##     ##     ##  ##  #### ##    ##
//    ##       ##     ##  ##     ##     ##  ##   ### ##    ##
//    ######## ##     ## ####    ##    #### ##    ##  ######

void
MRecordRadialIPMB0Topology::emitJSON(nlohmann::json &j) {
    j["record_id"]       = record_id;
    j["picmg_record_id"] = picmg_record_id;

    std::string connector_definer_hex;
    bytesToHexStr(connector_definer, connector_definer_hex);
    j["connector_definer"] = connector_definer_hex;

    std::string connector_version_id_hex;
    bytesToHexStr(connector_version_id, connector_version_id_hex);
    j["connector_version_id"] = connector_version_id_hex;

    nlohmann::json jarray;
    for (auto &&i : hub_descriptors) {
        nlohmann::json jentry;
        emitHubDescriptor(jentry, i);
        jarray.push_back(jentry);
    }

    j["hub_descriptors"] = jarray;
}

void
MRecordRadialIPMB0Topology::emitTOML(toml::table &t) {
    t["record_id"]       = toml::value(SC_I(record_id));
    t["picmg_record_id"] = toml::value(SC_I(picmg_record_id));

    std::string connector_definer_hex;
    bytesToHexStr(connector_definer, connector_definer_hex);
    t["connector_definer"] = toml::value(connector_definer_hex);

    std::string connector_version_id_hex;
    bytesToHexStr(connector_version_id, connector_version_id_hex);
    t["connector_version_id"] = toml::value(connector_version_id_hex);

    toml::array tarray;
    for (auto &&i : hub_descriptors) {
        toml::table tentry;
        emitHubDescriptor(tentry, i);
        tarray.push_back(toml::value(std::move(tentry)));
    }

    t["hub_descriptors"] = toml::value(std::move(tarray));
}

bool
MRecordRadialIPMB0Topology::emitBinary(bytes &out_bin, bool eol, Errs &errs) {
    UNUSED(errs);

    if (hub_descriptors.size() == 0) {
        return true;
    }

    bytes header;
    bytes payload;
    bytes tmp;
    bytes tmppl;

    size_t i         = 0;
    uchar  out_count = 0; // count of power feed in current record

    while (i < hub_descriptors.size()) {
        // +5: 3 bytes of connector definer + 2 bytes of connector version id
        size_t len = payload.size() + tmp.size() + MRECORD_HEADER_LEN_IPMI + 5;
        if (len >= MAX_AREA_LEN) {
            prependPICMGHeader(payload);
            APPEND_BYTES(payload, connector_definer);
            APPEND_BYTES(payload, connector_version_id);
            payload.emplace_back(std::byte{static_cast<uchar>(out_count - 1)});
            APPEND_BYTES(payload, tmppl);

            buildMRecordHeader(header, false, payload);
            APPEND_BYTES(out_bin, header);
            APPEND_BYTES(out_bin, payload);

            header.clear();
            payload.clear();
            tmppl.clear();
            out_count = 1; // we still have one buffered
        }

        APPEND_BYTES(tmppl, tmp);
        tmp.clear();

        emitHubDescriptor(tmp, hub_descriptors[i]);
        out_count++;
        i++;
    }

    APPEND_BYTES(tmppl, tmp);
    tmp.clear();

    prependPICMGHeader(payload);
    APPEND_BYTES(payload, connector_definer);
    APPEND_BYTES(payload, connector_version_id);
    payload.emplace_back(std::byte{out_count});
    APPEND_BYTES(payload, tmppl);

    buildMRecordHeader(header, eol, payload);
    APPEND_BYTES(out_bin, header);
    APPEND_BYTES(out_bin, payload);

    return true;
}

//    #### ##    ## #### ########
//     ##  ###   ##  ##     ##
//     ##  ####  ##  ##     ##
//     ##  ## ## ##  ##     ##
//     ##  ##  ####  ##     ##
//     ##  ##   ###  ##     ##
//    #### ##    ## ####    ##

MRecordRadialIPMB0Topology::MRecordRadialIPMB0Topology()
    : MRecordBase("radial_ipmb0_topology", "Radial IPMB-0 Link Mapping") {
    record_id       = MRECORD_PICMG_RECORD;
    picmg_record_id = PICMGREC_RADIAL_IPMB0_LINK_MAP;
}
