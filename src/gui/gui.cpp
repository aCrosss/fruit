#include <gtkmm.h>
#include <iostream>

#include "gui_areas/guiAreaChassis.hpp"
#include "manager.hpp"

#ifdef DGUI_EXTERNAL
#    define UI_FILE "iface.ui"
#endif

#define RES_PATH "/org/akber-soft/fruit"
#define UI_PATH  RES_PATH "/iface.ui"

typedef Glib::RefPtr<Gtk::FileFilter> FileFilter;

using namespace Glib;

Manager manager{};

Gtk::Window *win;

RefPtr<Gtk::Application>        app;
Gtk::TreeView                  *nav_tree;
RefPtr<Gtk::ListStore>          areas_list;
RefPtr<Gtk::CellRendererToggle> area_toggle;
Gtk::Viewport                  *area_viewport;
Gtk::Box                       *main_container;

std::unique_ptr<GUIAreaChassis> area;

void
cleanup() {
    area.reset();

    areas_list.reset();
    area_toggle.reset();

    nav_tree       = nullptr;
    area_viewport  = nullptr;
    main_container = nullptr;
    win            = nullptr;

    if (app) {
        app.reset();
    }
}

void
on_app_shutdown() {
    cleanup();
    exit(0);
}

bool
show_open_file_dialog(ustring title, FileFilter filter, std::string &path) {
    ustring ok   = "Выбрать";
    ustring cncl = "Отмена";

    auto dialog =
        Gtk::FileChooserNative::create(title, Gtk::FILE_CHOOSER_ACTION_OPEN, ok, cncl);

    dialog->add_filter(filter);

    const int result = dialog->run();

    switch (result) {
    case Gtk::RESPONSE_ACCEPT: {
        path = dialog->get_filename();
        return true;
    }
    case Gtk::RESPONSE_CANCEL:
    case Gtk::RESPONSE_DELETE_EVENT: {
        return false;
    }
    default: return false;
    }

    return false;
}

void
on_load_json() {
    ustring title = "Выбрать JSON файл для импорта";

    auto filter_text = Gtk::FileFilter::create();
    filter_text->set_name("JSON");
    filter_text->add_pattern("*.json");

    std::string path;
    if (!show_open_file_dialog(title, filter_text, path)) {
        return;
    }

    std::string    err;
    nlohmann::json j;
    if (!manager.loadJSON(path, j, err)) {
        std::cout << "failed to load JSON: " << err << std::endl;
        return;
    }

    area->set(j);
}

void
on_tree_selection_changed() {
    Glib::RefPtr<Gtk::TreeSelection> selection = nav_tree->get_selection();
    Gtk::TreeModel::iterator         iter      = selection->get_selected();
    Gtk::TreeModel::Path             path(iter);
    int                              row_number = path[0];

    // std::cout << "selection changed to " << row_number << std::endl;
}

void
on_area_toggle_toggled(const ustring &path) {
    auto iter = areas_list->get_iter(Gtk::TreeModel::Path(path));

    if (iter) {
        bool current_value;
        iter->get_value(1, current_value);
        iter->set_value(1, !current_value);
    }
}

void
on_app_activate() {
    auto refBuilder = Gtk::Builder::create();
    try {
#ifdef DGUI_EXTERNAL
        refBuilder->add_from_file(UI_FILE);
#else
        refBuilder->add_from_resource(UI_PATH);
#endif
    } catch (...) { return; }

    // load make window and make it visible
    Gtk::Window *win;
    refBuilder->get_widget<Gtk::Window>("main_window", win);
    app->add_window(*win);
    win->set_visible(true);

    // menu_import_json
    Gtk::MenuItem *menu_import_json;
    refBuilder->get_widget<Gtk::MenuItem>("menu_import_json", menu_import_json);
    menu_import_json->signal_activate().connect(sigc::ptr_fun(&on_load_json));

    // load left side 'navigation menu' TreeView widget
    refBuilder->get_widget<Gtk::TreeView>("nav_menu", nav_tree);
    auto select = nav_tree->get_selection();
    // connect signal on row selection
    select->signal_changed().connect(sigc::ptr_fun(&on_tree_selection_changed));

    // load ListStore object - data object of navigation menu
    auto al_obj = refBuilder->get_object("areas_list");
    areas_list  = RefPtr<Gtk::ListStore>::cast_dynamic(al_obj);

    // load CellRendererToggle
    auto at_obj = refBuilder->get_object("area_toggle");
    area_toggle = RefPtr<Gtk::CellRendererToggle>::cast_dynamic(at_obj);

    // connect CellRendererToggle's signal
    area_toggle->signal_toggled().connect(sigc::ptr_fun(&on_area_toggle_toggled));

    // area_viewport
    refBuilder->get_widget<Gtk::Viewport>("area_viewport", area_viewport);

    // main_container
    refBuilder->get_widget<Gtk::Box>("main_container", main_container);

    area = std::make_unique<GUIAreaChassis>();
    area->draw(main_container);

    std::string    err;
    nlohmann::json j;
    manager.loadJSON("test.json", j, err);

    area->set(j);

    main_container->show_all();

    win->show_all();
}

int
main(int argc, char **argv) {
    app = Gtk::Application::create(argc, argv);

    app->signal_shutdown().connect(sigc::ptr_fun(&on_app_shutdown));

    app->signal_activate().connect([]() { on_app_activate(); });
    app->run();

    cleanup();
    return 0;
}
