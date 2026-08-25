#pragma once

#include "fieldArr.hpp"
#include "fieldNum.hpp"
#include "guiAreaMRecBase.hpp"

class GUIAreaPowerDistribution : public GUIAreaMRecBase {
  private:
    FieldNumProps current_cp_props{
        0,    // initial
        0,    // min
        25.6, // 1/10th of 2 bytes value
        0.1,  // step_increment
        1,    // page_increment
        1     // digits
    };

    FieldNumProps voltage_cp_props{
        0,    // initial
        36.0, // min
        72.0, // 1/10th of 2 bytes value
        0.5,  // step_increment
        5,    // page_increment
        1     // digits
    };

    /* map entries */

    FieldNumProps mapping_cp_props{
        0,   // initial
        0,   // min
        255, // max
        1,   // step_increment
        5,   // page_increment
        0    // digits
    };

    FArrayDescr mapping_entries_descr = {
        {FARRAY_FIELD_TYPE_INT,
         "hardware_address",
         "Hardware Address",
         &mapping_cp_props,
         nullptr},
        {FARRAY_FIELD_TYPE_INT, "fru_device_id", "FRU Device ID", &mapping_cp_props, nullptr},
    };

    /* mappings */

    FArrayDescr mapping_descr = {{FARRAY_FIELD_TYPE_INT,
                                  "max_external_current",
                                  "Maximum External\nAvailable Current",
                                  &current_cp_props,
                                  nullptr},
                                 // ---
                                 {FARRAY_FIELD_TYPE_INT,
                                  "max_internal_current",
                                  "Maximum Internal\nCurrent",
                                  &current_cp_props,
                                  nullptr},
                                 // ---
                                 {FARRAY_FIELD_TYPE_INT,
                                  "min_expected_voltage",
                                  "Minimum Expected\nOperating Voltage",
                                  &voltage_cp_props,
                                  nullptr},
                                 // ---
                                 {FARRAY_FIELD_TYPE_ARRAY,
                                  "entries",
                                  "Mapping Entries",
                                  &mapping_entries_descr,
                                  nullptr}};

  public:
    GUIAreaPowerDistribution();
};
