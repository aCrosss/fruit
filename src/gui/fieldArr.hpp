#pragma once

#include <memory>
#include <vector>

#include "fieldBase.hpp"

enum FArrayType {
    FARRAY_FIELD_TYPE_ENUM,
    FARRAY_FIELD_TYPE_INT,
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

    FieldArr(std::string tag, std::string label, FArrayDescr &array_description);
    ~FieldArr();
};
