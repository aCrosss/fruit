#include "guiAreaBoard.hpp"
#include "common.hpp"
#include "fieldDateTime.hpp"
#include "fieldEnum.hpp"
#include "types.hpp"

GUIAreaBoard::GUIAreaBoard() : GUIAreaBase("board", "Board Info Area") {
    INIT_FIELD(FieldEnum, "language_code", "Language Code", lang_codes_enum, VAL_TYPE_INT);
    INIT_FIELD(FieldDateTime, "date_time", "Mfg. Date/Time");
    INIT_FIELD(FieldEncStr, "manufacturer", "Manufacturer");
    INIT_FIELD(FieldEncStr, "product_name", "Product Name");
    INIT_FIELD(FieldEncStr, "serial_number", "Serial Number");
    INIT_FIELD(FieldEncStr, "file_id", "FRU File ID");
    INIT_FIELD(FieldArr, "custom", "Custom", custom, true);
}
