#include "guiAreaActivationAndPowerMng.hpp"
#include "fieldArr.hpp"
#include "fieldNum.hpp"
#include "guiAreaBase.hpp"
#include "guiAreaMRecBase.hpp"
#include "types.hpp"

GUIAreaActivationAndPowerMng::GUIAreaActivationAndPowerMng()
    : GUIAreaMRecBase("Shelf Activation and Power Management",
                      MRECORD_PICMG_RECORD,
                      PICMGREC_ACTIVATION_AND_POWER_MNG) {
    //
    INIT_FIELD(FieldNum,
               "allowance_for_activation",
               "Allowance for FRU\nAcrivation Readiness",
               byte_cp_props);
    INIT_FIELD(FieldArr, "entries", "Descriptors", descriptor);
}
