#pragma once

#include <memory>
#include <vector>

#include "fieldBase.hpp"

#define BLANK_TAG "tag"

enum FArrayType {
    FARRAY_FIELD_TYPE_ENUM,
    FARRAY_FIELD_TYPE_INT,
    FARRAY_FIELD_TYPE_CHECKBOX,
    FARRAY_FIELD_TYPE_STR,
    FARRAY_FIELD_TYPE_ENC_STR,
    FARRAY_FIELD_TYPE_ARRAY,
};

struct ArrayDescriptor {
    FArrayType  type;
    std::string tag;
    std::string label;
    void       *arg1;
    void       *arg2;
};

typedef std::vector<std::shared_ptr<FieldBase>> FieldsRef;
typedef std::vector<ArrayDescriptor>            FArrayDescr;

class FieldArr : public FieldBase {
  private:
    // plain means field array contains only one type of entries, each entry will have
    // only one field and field's will have same tag "tag" which won't be present in exported
    // json and not needed when importing
    // good example is guid string array:
    // "guids": [
    //     "3c83af87-7595-4711-abff-692bc2b58758",
    //     "81c36d83-7280-45d8-b529-9cc3fe2680c8"
    //  ],
    //
    // Note! If you use plain you *shall*:
    // 1. send FArrayDescr with only one entry
    // 2. make sure entering json is an array with no nested objects
    // 3. probably shouldn't use plain array of arrays. who knows...
    // in case of plane array FArrayDescr's field's tag doesn't matter
    bool flat;

    FArrayDescr array_description;

    struct FArrayEntry {
        FieldsRef        fields;
        // contains collection of full array entries
        Gtk::Box         entry_container;
        // single array entry, contains fields widgets and del button
        Gtk::Box         subentry_container;
        // contains all fields' widgets
        Gtk::Box         fields_container;
        Gtk::Button      btn_del;
        sigc::connection btn_del_con;
    };
    std::vector<FArrayEntry> entries;

    // contains all entries and add button
    Gtk::Box array_container;

    Gtk::Box    btn_container;
    Gtk::Button btn_add;

    void reconnectSignals();
    void removeEntry(size_t ind);

  public:
    void clear() override;

    void get(nlohmann::json &j) override;
    void set(nlohmann::json &j) override;

    bool validate() override;

    void appendEntry();
    void appendEntry(nlohmann::json j);

    FieldArr(std::string  tag,
             std::string  label,
             FArrayDescr &array_description,
             bool         flat = false);
    ~FieldArr();
};
