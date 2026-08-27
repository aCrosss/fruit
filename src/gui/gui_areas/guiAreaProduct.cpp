#include "guiAreaProduct.hpp"
#include "guiAreaBase.hpp"

GUIAreaProduct::GUIAreaProduct() : GUIAreaBase("product", "Product Info Area") {
    INIT_FIELD(FieldEnum, "language_code", "Language Code", lang_codes_enum, VAL_TYPE_INT);
    INIT_FIELD(FieldEncStr, "manufacturer", "Manufacturer Name");
    INIT_FIELD(FieldEncStr, "product_name", "Product Name");
    INIT_FIELD(FieldEncStr, "part", "Product Part/Model\nNumber");
    INIT_FIELD(FieldEncStr, "version", "Product Version");
    INIT_FIELD(FieldEncStr, "serial_number", "Product Serial\nNubmer");
    INIT_FIELD(FieldEncStr, "asset_tag", "Asset Tag");
    INIT_FIELD(FieldEncStr, "fru_file_id", "FRU File ID");
    INIT_FIELD(FieldArr, "custom", "Custom", custom, true);
}
