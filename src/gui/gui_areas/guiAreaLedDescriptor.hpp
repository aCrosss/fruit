#pragma once

#include "fieldArr.hpp"
#include "fieldNum.hpp"
#include "guiAreaMRecBase.hpp"

class GUIAreaLEDDescriptor : public GUIAreaMRecBase {
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
        {FARRAY_FIELD_TYPE_INT, "led_id", "LED ID", &byte_cpp, nullptr},
        {FARRAY_FIELD_TYPE_ENC_STR, "led_legend", "LED Legend", &byte_cpp, nullptr},
        {FARRAY_FIELD_TYPE_ENC_STR, "led_symbol", "LED Symbol", &byte_cpp, nullptr},
        {FARRAY_FIELD_TYPE_ENC_STR, "led_description", "LED Description", &byte_cpp, nullptr},
    };

  public:
    GUIAreaLEDDescriptor();
};
