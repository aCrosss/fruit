#pragma once

#include "fieldEncStr.hpp"
#include "fieldEncStrArr.hpp"
#include "fieldEnum.hpp"

#define INIT_FIELD(type, ...) (fields.emplace_back(std::make_shared<type>(__VA_ARGS__)))

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
    std::string label;

    Gtk::Box       container;
    Gtk::HeaderBar header;

    std::vector<Field> fields;

  public:
    void clear();

    void show(Gtk::Box *parent);
    void hide(Gtk::Box *parent);

    void get(nlohmann::json &j);
    void set(nlohmann::json &j);

    bool validate();

    GUIAreaBase(std::string tag, std::string label);
    virtual ~GUIAreaBase() = default;
};
