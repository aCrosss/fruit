#include "guiAreaAddressTable.hpp"
#include "fieldArr.hpp"
#include "fieldEncStr.hpp"
#include "guiAreaMRecBase.hpp"
#include "types.hpp"

GUIAreaAddressTable::GUIAreaAddressTable()
    : GUIAreaMRecBase("Address Table", MRECORD_PICMG_RECORD, PICMGREC_ADDRESS_TABLE) {
    //
    INIT_FIELD(FieldEncStr, "shelf_address", "Shelf Address");
    INIT_FIELD(FieldArr, "entries", "Address Table\nEntries", arr_descr);
}
