#include "guiAreaChassis.hpp"
#include "fieldArr.hpp"
#include "fieldEncStr.hpp"
#include "guiAreaBase.hpp"

GUIAreaChassis::GUIAreaChassis() : GUIAreaBase("chassis", "Chassis Info Area") {
    INIT_FIELD(FieldEnum, "type", "Type", chassis_types_enum, VAL_TYPE_INT);
    INIT_FIELD(FieldEncStr, "part_number", "Part Number");
    INIT_FIELD(FieldEncStr, "serial_number", "Serial Number");
    INIT_FIELD(FieldArr, "custom", "Custom", custom, true);
}
