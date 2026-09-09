#pragma once

#include "fieldArr.hpp"
#include "fieldNum.hpp"
#include "guiAreaMRecBase.hpp"

class GUIAreaActivationAndPowerMng : public GUIAreaMRecBase {
  private:
    FieldNumProps byte_cp_props{
        0,   // initial
        0,   // min
        255, // max
        1,   // step_increment
        5,   // page_increment
        0    // digits
    };

    FieldNumProps two_bytes_cp_props{
        0,     // initial
        0,     // min
        65535, // max
        1,     // step_increment
        5,     // page_increment
        0      // digits
    };

    FieldNumProps delay_cp_props{
        0,  // initial
        0,  // min
        63, // max - 5:0 bits
        1,  // step_increment
        5,  // page_increment
        0   // digits
    };

    FArrayDescr descriptor{
        {FARRAY_FIELD_TYPE_INT,
         "hardware_address",
         "Hardware Address",
         &byte_cp_props,
         nullptr},
        // ---
        {FARRAY_FIELD_TYPE_INT, "fru_device_id", "FRU Device ID", &byte_cp_props, nullptr},
        // ---
        {FARRAY_FIELD_TYPE_INT,
         "max_fru_power_cap",
         "Maximum FRU\nPower Capability",
         &two_bytes_cp_props,
         nullptr},
        // ---
        {FARRAY_FIELD_TYPE_CHECKBOX,
         "controlled_deactivation",
         "Shelf Manager\nControlled Deactivation",
         nullptr,
         nullptr},
        // ---
        {FARRAY_FIELD_TYPE_CHECKBOX,
         "controlled_activation",
         "Shelf Manager\nControlled Activation",
         nullptr,
         nullptr},
        // ---
        {FARRAY_FIELD_TYPE_INT,
         "next_power_on_delay",
         "Delay Before Next\nPower On",
         &delay_cp_props,
         nullptr},
    };

  public:
    GUIAreaActivationAndPowerMng();
};
