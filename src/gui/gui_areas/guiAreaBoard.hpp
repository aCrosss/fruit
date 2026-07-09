#pragma once

#include "guiAreaBase.hpp"

class GUIAreaBoard : public GUIAreaBase {
  private:
    Gtk::Box       container;
    Gtk::HeaderBar header;
    // Gtk::Button    btn_hide;

  public:
    void show(Gtk::Box *parent);
    void hide(Gtk::Box *parent);

    void get(nlohmann::json &j);
    void set(nlohmann::json &j);

    GUIAreaBoard(/* args */);
    ~GUIAreaBoard();
};
