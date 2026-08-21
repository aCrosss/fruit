#include "guiAreaBoard.hpp"
#include "common.hpp"
#include "fieldDateTime.hpp"
#include "fieldEnum.hpp"
#include "types.hpp"

GUIAreaBoard::GUIAreaBoard() : GUIAreaBase("board", "Board Info Area") {
    fields.emplace_back(std::make_shared<FieldEnum>(
        "language_code", "Language Code", lang_codes_enum, VAL_TYPE_INT));
    fields.emplace_back(std::make_shared<FieldDateTime>("date_time", "Mfg. Date/Time"));
    fields.emplace_back(std::make_shared<FieldEncStr>("manufacturer", "Manufacturer"));
    fields.emplace_back(std::make_shared<FieldEncStr>("product_name", "Product Name"));
    fields.emplace_back(std::make_shared<FieldEncStr>("serial_number", "Serial Number"));
    fields.emplace_back(std::make_shared<FieldEncStr>("file_id", "FRU File ID"));
    fields.emplace_back(std::make_shared<FieldEncStrArr>("custom", "Custom"));
}
