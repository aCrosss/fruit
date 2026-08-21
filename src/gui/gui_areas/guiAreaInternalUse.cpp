#include "guiAreaInternalUse.hpp"
#include "fieldBytes.hpp"
#include "guiAreaBase.hpp"

GUIAreaInternalUse::GUIAreaInternalUse() : GUIAreaBase("internal_use", "Internal Use Area") {
    fields.emplace_back(std::make_shared<FieldBytes>("data", "Internal use data"));
}
