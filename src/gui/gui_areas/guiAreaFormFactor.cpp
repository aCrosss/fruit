#include "guiAreaFormFactor.hpp"
#include "fieldEnum.hpp"
#include "guiAreaBase.hpp"
#include "types.hpp"

GUIAreaFormFactor::GUIAreaFormFactor()
    : GUIAreaMRecBase("PICMG Form Factor", MRECORD_PICMG_RECORD, PICMGREC_FORM_FACTOR) {
    //
    INIT_FIELD(FieldEnum, "form_factor", "Form Factor", ffs, ffs_type);
}
