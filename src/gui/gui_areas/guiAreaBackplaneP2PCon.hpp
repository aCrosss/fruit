#pragma once

#include "fieldArr.hpp"
#include "fieldEnum.hpp"
#include "fieldNum.hpp"
#include "guiAreaMRecBase.hpp"
#include "types.hpp"

class GUIAreaBackplaneP2PCon : public GUIAreaMRecBase {
  private:
    /* channel descriptor */

    FieldNumProps chanels_cp_props{
        0,  // initial
        0,  // min
        31, // max - bits 17:13
        1,  // step_increment
        5,  // page_increment
        0   // digits
    };

    FieldNumProps slot_cp_props{
        0,   // initial
        0,   // min
        255, // max
        1,   // step_increment
        5,   // page_increment
        0    // digits
    };

    FArrayDescr p2p_channel_descr = {
        {FARRAY_FIELD_TYPE_INT, "local_channel", "Local Channel", &chanels_cp_props, nullptr},
        {FARRAY_FIELD_TYPE_INT, "remote_channel", "Remote Channel", &chanels_cp_props, nullptr},
        {FARRAY_FIELD_TYPE_INT, "remote_slot", "Remote Slot", &slot_cp_props, nullptr}};

    /* slot descriptor */

    enumVals channel_types{
        {"Single Port Fabric Interface", 0x08},
        {"Double Port Fabric Interface", 0x09},
        {"Full Channel Fabric Interface", 0x0A},
        {"Base Interface", 0x0B},
        {"Update Channel Interface", 0x0C},
        {"Base Interface ShMC Cross-connect", 0x0D},
    };
    ValType chan_type_enum_type = VAL_TYPE_INT;

    FArrayDescr p2p_slot_descr = {
        // ---
        {FARRAY_FIELD_TYPE_ENUM,
         "type",
         "Point-To-Point\nChannel Type",
         &channel_types,
         &chan_type_enum_type},
        // ---
        {FARRAY_FIELD_TYPE_INT, "address", "Slot Address", &slot_cp_props, nullptr},
        // ---
        {FARRAY_FIELD_TYPE_ARRAY,
         "channels",
         "Channel Descriptors",
         &p2p_channel_descr,
         nullptr}};

  public:
    GUIAreaBackplaneP2PCon();
};
