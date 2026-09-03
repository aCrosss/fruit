#pragma once

#include "fieldArr.hpp"
#include "fieldNum.hpp"
#include "guiAreaMRecBase.hpp"
#include "types.hpp"

class GUIAreaRadialIPMB0Topology : public GUIAreaMRecBase {
  private:
    FieldNumProps byte_cp_props{
        0,   // initial
        0,   // min
        255, // max
        1,   // step_increment
        5,   // page_increment
        0    // digits
    };

    FieldNumProps link_cp_props{
        0,  // initial
        1,  // min
        95, // max - look picmg table 3-66
        1,  // step_increment
        5,  // page_increment
        0   // digits
    };

    FArrayDescr link_entry = {{FARRAY_FIELD_TYPE_INT,
                               "hardware_address",
                               "Hardware Address",
                               &byte_cp_props,
                               nullptr},
                              {FARRAY_FIELD_TYPE_INT,
                               "ipmb0_link_entry",
                               "IPMB-0 Link Entry",
                               &link_cp_props,
                               nullptr}};

    enumVals bus_coverage{
        {"IPMB-A only", "ipmb_a"},
        {"IPMB-B only", "ipmb_b"},
        {"IPMB-A and IPMB-B", "ipmb_both"},
    };
    ValType bus_coverage_enum_type = VAL_TYPE_TEXT;

    FArrayDescr hub_descriptor = {
        {FARRAY_FIELD_TYPE_INT,
         "hardware_address",
         "Hardware Address",
         &byte_cp_props,
         nullptr},
        // ---
        {FARRAY_FIELD_TYPE_ENUM,
         "bus_coverage",
         "Bus Coverage",
         &bus_coverage,
         &bus_coverage_enum_type},
        // ---
        {FARRAY_FIELD_TYPE_ARRAY,
         "link_mappings",
         "IPMB-0 Link\nMappings",
         &link_entry,
         nullptr},
    };

  public:
    GUIAreaRadialIPMB0Topology();
};
