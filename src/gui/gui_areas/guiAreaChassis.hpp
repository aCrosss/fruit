#pragma once

#include "guiAreaBase.hpp"

class GUIAreaChassis : public GUIAreaBase {
  private:
    enumVals chassis_types_enum{
        {"Other", 0x01},
        {"Unknown", 0x02},
        {"Desktop", 0x03},
        {"Low Profile Desktop", 0x04},
        {"Pizza Box", 0x05},
        {"Mini Tower", 0x06},
        {"Tower", 0x07},
        {"Portable", 0x08},
        {"LapTop", 0x09},
        {"Notebook", 0x0A},
        {"Hand Held", 0x0B},
        {"Docking Station", 0x0C},
        {"All in One", 0x0D},
        {"Sub Notebook", 0x0E},
        {"Space-saving", 0x0F},
        {"Lunch Box", 0x10},
        {"Main Server Chassis", 0x11},
        {"Expansion Chassis", 0x12},
        {"SubChassis", 0x13},
        {"Bus Expansion Chassis", 0x14},
        {"Peripheral Chassis", 0x15},
        {"RAID Chassis", 0x16},
        {"Rack Mount Chassis", 0x17},
        {"Sealed-case PC", 0x18},
        {"Multi-system chassis", 0x19},
        {"CompactPCI", 0x1A},
        {"AdvancedTCA", 0x1B},
        {"Blade", 0x1C},
        {"Blade Enclosure", 0x1D},
    };

    Gtk::Box       container;
    Gtk::HeaderBar header;
    Gtk::Button    btn_hide;

  public:
    void show(Gtk::Box *parent) override;
    void hide(Gtk::Box *parent) override;

    void get(nlohmann::json &j) override;
    void set(nlohmann::json &j) override;

    GUIAreaChassis(/* args */);
    ~GUIAreaChassis();
};
