#pragma once

#include "fieldArr.hpp"
#include "fieldEnum.hpp"
#include "fieldNum.hpp"
#include "guiAreaMRecBase.hpp"
#include "types.hpp"

class GUIAreaAddressTable : public GUIAreaMRecBase {
  private:
    FieldNumProps cp_props{
        0,   // initial
        0,   // min
        255, // max
        1,   // step_increment
        5,   // page_increment
        0    // digits
    };

    enumVals site_types = {
        {"Front Board", 0x00},
        {"Power Entry", 0x01},
        {"Shelf FRU Information", 0x02},
        {"Dedicated ShMC", 0x03},
        {"Fan Tray", 0x04},
        {"Fan Filter Tray", 0x05},
        {"Alarm", 0x06},
        {"AdvancedMC Module", 0x07},
        {"PMC", 0x08},
        {"Rear Transition Module", 0x09},
        {"OEM-C0", 0xC0},
        {"OEM-C1", 0xC1},
        {"OEM-C2", 0xC2},
        {"OEM-C3", 0xC3},
        {"OEM-C4", 0xC4},
        {"OEM-C5", 0xC5},
        {"OEM-C6", 0xC6},
        {"OEM-C7", 0xC7},
        {"OEM-C8", 0xC8},
        {"OEM-C9", 0xC9},
        {"OEM-CA", 0xCA},
        {"OEM-CB", 0xCB},
        {"OEM-CC", 0xCC},
        {"OEM-CD", 0xCD},
        {"OEM-CE", 0xCE},
        {"OEM-CF", 0xCF},
        {"Unknown", 0xFF},
    };
    ValType site_type_valtype = VAL_TYPE_INT;

    FArrayDescr arr_descr = {
        {FARRAY_FIELD_TYPE_INT, "hardware_address", "Hardware Address", &cp_props, nullptr},
        {FARRAY_FIELD_TYPE_INT, "site_number", "Site Number", &cp_props, nullptr},
        {FARRAY_FIELD_TYPE_ENUM, "site_type", "Site Type", &site_types, &site_type_valtype}};

  public:
    GUIAreaAddressTable();
};
