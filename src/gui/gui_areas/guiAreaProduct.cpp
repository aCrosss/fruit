#include "guiAreaProduct.hpp"
#include "guiAreaBase.hpp"

GUIAreaProduct::GUIAreaProduct() : GUIAreaBase("product_info", "Product Info Area") {
    fields.emplace_back(std::make_shared<FieldEnum>(
        "language_code", "Language Code", lang_codes_enum, VAL_TYPE_INT));
    fields.emplace_back(std::make_shared<FieldEncStr>("manufacturer", "Manufacturer Name"));
    fields.emplace_back(std::make_shared<FieldEncStr>("product_name", "Product Name"));
    fields.emplace_back(std::make_shared<FieldEncStr>("part", "Product Part/Model\nNumber"));
    fields.emplace_back(std::make_shared<FieldEncStr>("version", "Product Version"));
    fields.emplace_back(
        std::make_shared<FieldEncStr>("serial_number", "Product Serial\nNubmer"));
    fields.emplace_back(std::make_shared<FieldEncStr>("asset_tag", "Asset Tag"));
    fields.emplace_back(std::make_shared<FieldEncStr>("fru_file_id", "FRU File ID"));
    fields.emplace_back(std::make_shared<FieldEncStrArr>("custom", "Custom"));
}
