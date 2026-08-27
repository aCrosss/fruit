#pragma once

#include "fieldArr.hpp"
#include "fieldNum.hpp"
#include "guiAreaMRecBase.hpp"

class GUIAreaFanGeography : public GUIAreaMRecBase {
  private:
    FieldNumProps byte_cpp{
        0,   // initial
        0,   // min
        255, // max
        1,   // step_increment
        5,   // page_increment
        0    // digits
    };

    FArrayDescr entries_descr = {
        {FARRAY_FIELD_TYPE_INT, "hardware_address", "Hardware Address", &byte_cpp, nullptr},
        {FARRAY_FIELD_TYPE_INT, "fru_device_id", "FRU Device ID", &byte_cpp, nullptr},
        {FARRAY_FIELD_TYPE_INT, "site_number", "Site Number", &byte_cpp, nullptr},
        {FARRAY_FIELD_TYPE_INT, "site_type", "Site Type", &byte_cpp, nullptr},
    };

  public:
    GUIAreaFanGeography();
};
