#include "guiAreaInternalUse.hpp"
#include "fieldBytes.hpp"
#include "guiAreaBase.hpp"

GUIAreaInternalUse::GUIAreaInternalUse() : GUIAreaBase("internal_use", "Internal Use Area") {
    INIT_FIELD(FieldBytes, "data", "Internal use data");
}
