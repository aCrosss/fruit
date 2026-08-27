#include "guiAreaLedDescriptor.hpp"
#include "fieldArr.hpp"
#include "guiAreaBase.hpp"
#include "types.hpp"

GUIAreaLEDDescriptor::GUIAreaLEDDescriptor()
    : GUIAreaMRecBase("LED Description", MRECORD_PICMG_RECORD, PICMGREC_LED_DESCRIPTOR) {
    //
    INIT_FIELD(FieldArr, "entries", "LED Descriptors", entries_descr);
}
