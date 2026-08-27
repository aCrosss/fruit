#pragma once

#include "guiAreaBase.hpp"

class GUIAreaMRecBase : public GUIAreaBase {
  protected:
    uchar record_id       = 0;
    uchar picmg_record_id = 0;

  public:
    void get(nlohmann::json &j); // overwritten
    bool set(nlohmann::json &j); // overwritten

    GUIAreaMRecBase(std::string label, uchar record_id, uchar picmg_record_id);
};
