#include "guiAreaBackplaneP2PCon.hpp"
#include "fieldArr.hpp"
#include "guiAreaBase.hpp"
#include "types.hpp"

GUIAreaBackplaneP2PCon::GUIAreaBackplaneP2PCon()
    : GUIAreaMRecBase("Backplane Point-to-Point Connectivity",
                      MRECORD_PICMG_RECORD,
                      PICMGREC_BACKPLANE_P2PCON) {
    //
    INIT_FIELD(FieldArr, "slots", "Slot Description\nList", p2p_slot_descr);
}
