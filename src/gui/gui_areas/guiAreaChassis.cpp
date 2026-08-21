#include "guiAreaChassis.hpp"

GUIAreaChassis::GUIAreaChassis() : GUIAreaBase("chassis", "Chassis Info Area") {
    fields.emplace_back(
        std::make_shared<FieldEnum>("type", "Type", chassis_types_enum, VAL_TYPE_INT));
    fields.emplace_back(std::make_shared<FieldEncStr>("part_number", "Part Number"));
    fields.emplace_back(std::make_shared<FieldEncStr>("serial_number", "Serial Number"));
    fields.emplace_back(std::make_shared<FieldEncStrArr>("custom", "Custom"));
}
