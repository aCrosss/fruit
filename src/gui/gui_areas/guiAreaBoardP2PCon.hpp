#pragma once

#include "fieldArr.hpp"
#include "fieldNum.hpp"
#include "fieldStr.hpp"
#include "guiAreaMRecBase.hpp"
#include "types.hpp"

class GUIAreaBoardP2PCon : public GUIAreaMRecBase {
  private:
    FStrType    guid_str_type = FSTR_TYPE_GUID;
    FArrayDescr guids_ad      = {
        {FARRAY_FIELD_TYPE_STR, "guid", "OEM GUID", &guid_str_type, nullptr}};

    FieldNumProps byte_cp_props{
        0,   // initial
        0,   // min
        255, // max
        1,   // step_increment
        5,   // page_increment
        0    // digits
    };

    FieldNumProps hbyte_cp_props{
        0,  // initial
        0,  // min
        63, // max
        1,  // step_increment
        5,  // page_increment
        0   // digits
    };

    enumVals link_types{
        {"PICMG 3.0 Base Interface 10/100/1000 BASE-T", 0x01},
        {"PICMG 3.1 Ethernet Fabric Interface", 0x02},
        {"PICMG 3.2 Infiniband Fabric Interface", 0x03},
        {"PICMG 3.3 StarFabric Fabric Interface", 0x04},
        {"PICMG 3.4 PCI Express Fabric Interface", 0x05},
        {"OEM-C0", 0xF0},
        {"OEM-C1", 0xF1},
        {"OEM-C2", 0xF2},
        {"OEM-C3", 0xF3},
        {"OEM-C4", 0xF4},
        {"OEM-C5", 0xF5},
        {"OEM-C6", 0xF6},
        {"OEM-C7", 0xF7},
        {"OEM-C8", 0xF8},
        {"OEM-C9", 0xF9},
        {"OEM-CA", 0xFA},
        {"OEM-CB", 0xFB},
        {"OEM-CC", 0xFC},
        {"OEM-CD", 0xFD},
        {"OEM-CE", 0xFE},
    };
    ValType link_types_enum_type = VAL_TYPE_INT;

    enumVals iface_types{
        {"Base Interface", "base"},
        {"Fabric Interface", "fabric"},
        {"Update Channel Interface", "update_channel"},
    };
    ValType iface_types_type = VAL_TYPE_TEXT;

    enumVals link_type_extensions{
        {"10/100/1000 BASE-T Link", 0x00},
        {"ShMC Cross-connect", 0x01},
    };
    ValType link_type_extensions_type = VAL_TYPE_INT;

    FArrayDescr link_descriptors_ad = {
        {FARRAY_FIELD_TYPE_INT,
         "link_grouping_id",
         "Link Grouping ID",
         &hbyte_cp_props,
         nullptr},
        // ---
        {FARRAY_FIELD_TYPE_ENUM,
         "link_type_extension",
         "Link Type Extension",
         &link_type_extensions,
         &link_type_extensions_type},
        // ---
        {FARRAY_FIELD_TYPE_ENUM, "link_type", "Link Type", &link_types, &link_types_enum_type},
        {FARRAY_FIELD_TYPE_CHECKBOX, "port_3", "Port 3 Inlcuded", nullptr, nullptr},
        {FARRAY_FIELD_TYPE_CHECKBOX, "port_2", "Port 2 Inlcuded", nullptr, nullptr},
        {FARRAY_FIELD_TYPE_CHECKBOX, "port_1", "Port 1 Inlcuded", nullptr, nullptr},
        {FARRAY_FIELD_TYPE_CHECKBOX, "port_0", "Port 0 Inlcuded", nullptr, nullptr},
        {FARRAY_FIELD_TYPE_ENUM, "interface", "Interface", &iface_types, &iface_types_type},
        {FARRAY_FIELD_TYPE_INT, "channel_number", "Channel Number", &hbyte_cp_props, nullptr}};

  public:
    GUIAreaBoardP2PCon();
};
