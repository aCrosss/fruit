#pragma once

#include "guiAreaBase.hpp"

class GUIAreaInternalUse : public GUIAreaBase {
  private:
    Gtk::Box       container;
    Gtk::HeaderBar header;

  public:
    void show(Gtk::Box *parent) override;
    void hide(Gtk::Box *parent) override;

    void get(nlohmann::json &j) override;
    void set(nlohmann::json &j) override;

    GUIAreaInternalUse();
};
