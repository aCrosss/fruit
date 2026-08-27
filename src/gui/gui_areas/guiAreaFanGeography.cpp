#include "guiAreaFanGeography.hpp"
#include "fieldArr.hpp"
#include "guiAreaBase.hpp"
#include "guiAreaMRecBase.hpp"
#include "types.hpp"

GUIAreaFanGeography::GUIAreaFanGeography()
    : GUIAreaMRecBase("Shelf Fan Geography", MRECORD_PICMG_RECORD, PICMGREC_FAN_GEOGRAPHY) {
    //
    INIT_FIELD(FieldArr, "entries", "Fan-to-FRU Entries", entries_descr);
}
