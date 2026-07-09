#pragma once

#include "fieldEncStr.hpp"
#include "fieldEncStrArr.hpp"
#include "fieldEnum.hpp"

typedef std::shared_ptr<FieldBase> Field;

enum FieldType {
    FIELD_TYPE_BOOL,
    FIELD_TYPE_ENC_STR,
    FIELD_TYPE_INT,   //
    FIELD_TYPE_CHAR,  // single numerical field
    FIELD_TYPE_FLOAT, //
    // FIELD_TYPE_ENUM_ENC,
};

class GUIAreaBase {
  protected:
    std::string tag;

    std::vector<Field> fields;

  public:
    void clear();

    virtual void show(Gtk::Box *parent) = 0;
    virtual void hide(Gtk::Box *parent) = 0;

    virtual void get(nlohmann::json &j) = 0;
    virtual void set(nlohmann::json &j) = 0;

    bool validate();

    GUIAreaBase(std::string tag);
    virtual ~GUIAreaBase() = default;
};
