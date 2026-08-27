#pragma once

#include "fieldEnum.hpp"
#include "guiAreaMRecBase.hpp"
#include "types.hpp"

class GUIAreaFormFactor : public GUIAreaMRecBase {
  private:
    enumVals ffs = {
        {"ATCA", "ATCA"},
        {"ATCA300", "ATCA300"},
        {"MicroTCA", "MicroTCA"},
    };
    ValType ffs_type = VAL_TYPE_TEXT;

  public:
    GUIAreaFormFactor();
};
