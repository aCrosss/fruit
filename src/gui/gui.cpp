#include <gtkmm.h>
#include <iostream>

#ifdef DGUI_EXTERNAL
#    define UI_FILE "iface.ui"
#endif

#define RES_PATH "/org/akber-soft/fruit"
#define UI_PATH  RES_PATH "/iface.ui"

using namespace Glib;

RefPtr<Gtk::Application>        app;
Gtk::TreeView                  *nav_tree;
RefPtr<Gtk::ListStore>          areas_list;
RefPtr<Gtk::CellRendererToggle> area_toggle;

Gtk::Viewport *area_viewport;

void
on_tree_selection_changed() {
    Glib::RefPtr<Gtk::TreeSelection> selection = nav_tree->get_selection();
    Gtk::TreeModel::iterator         iter      = selection->get_selected();
    Gtk::TreeModel::Path             path(iter);
    int                              row_number = path[0];

    std::cout << "selection changed to " << row_number << std::endl;
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
}

int
main(int argc, char **argv) {
    app = Gtk::Application::create(argc, argv);

    app->signal_activate().connect([]() { on_app_activate(); });
    app->run();

    app.reset();
    return 0;
}
