#include <gtkmm.h>
#include <iostream>
#include <memory>
#include <sstream>

// clang-format off
#include "gui_areas/guiAreaInternalUse.hpp"
#include "gui_areas/guiAreaChassis.hpp"
#include "gui_areas/guiAreaBoard.hpp"
#include "gui_areas/guiAreaProduct.hpp"
#include "gui_areas/guiAreaBackplaneP2PCon.hpp"
#include "gui_areas/guiAreaAddressTable.hpp"
#include "gui_areas/guiAreaPowerDistribution.hpp"
#include "gui_areas/guiAreaActivationAndPowerMng.hpp"
#include "gui_areas/guiAreaIPConnection.hpp"
#include "gui_areas/guiAreaBoardP2PCon.hpp"
#include "gui_areas/guiAreaRadialIPMB0Topology.hpp"
#include "gui_areas/guiAreaFanGeography.hpp"
#include "gui_areas/guiAreaFormFactor.hpp"
#include "gui_areas/guiAreaLedDescriptor.hpp"
// clang-format on

#include "manager.hpp"
#include "types.hpp"

#ifdef DGUI_EXTERNAL
#    define UI_FILE "iface.ui"
#endif

#define RES_PATH "/org/akber-soft/fruit"
#define UI_PATH  RES_PATH "/iface.ui"

#define GUI_MRECORDS_START_INDEX 4

#define SHOW_ERRORW(msg)                       \
    {                                          \
        std::stringstream ss;                  \
        ss << msg;                             \
        show_alert_dialog("Ошибка", ss.str()); \
    }

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

std::vector<std::shared_ptr<GUIAreaBase>> gui_areas;
size_t                                    curent_area;

void
cleanup() {
    for (auto &&i : gui_areas) {
        i.reset();
    }

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

static inline Gtk::TreeModel::Path
get_area_path(size_t ind) {
    std::stringstream path;
    path << ind;
    return Gtk::TreeModel::Path(path.str());
}

static inline bool
get_area_enabled(size_t ind) {
    bool area_enabled = false;

    std::stringstream path;
    path << ind;
    auto iter = areas_list->get_iter(Gtk::TreeModel::Path(path.str()));
    if (iter) {
        iter->get_value(1, area_enabled);
    } else {
        return false;
    }

    return area_enabled;
}

static inline void
set_area_enabled(size_t ind, bool area_enabled) {
    std::stringstream path;
    path << ind;
    auto iter = areas_list->get_iter(Gtk::TreeModel::Path(path.str()));
    if (!iter) {
        return;
    }

    iter->set_value(1, area_enabled);
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

bool
show_confirmation_dialog(std::string title, std::string msg) {
    Gtk::MessageDialog dialog(
        title, false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL, true);
    if (!msg.empty()) {
        dialog.set_secondary_text(msg);
    }

    int result = dialog.run();
    return result == Gtk::ResponseType::RESPONSE_OK;
}

void
show_alert_dialog(std::string title, std::string msg) {
    Gtk::MessageDialog dialog(title, false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_CLOSE, true);
    if (!msg.empty()) {
        dialog.set_secondary_text(msg);
    }

    dialog.run();
}

static void
on_new() {
    std::string title = "Внимание!";
    std::string msg =
        "Это действие очистит все введенные данные, Вы уверены, что хотите продолжить?";

    if (!show_confirmation_dialog(title, msg)) {
        return;
    }

    for (size_t i = 0; i < gui_areas.size(); i++) {
        auto &&a = gui_areas[i];
        a->clear();
        set_area_enabled(i, false);
    }
}

static void
load_json(nlohmann::json j) {
    for (int i = 0; i < GUI_MRECORDS_START_INDEX; ++i) {
        auto &&a = gui_areas[i];
        a->clear();
        bool enabled = a->set(j);
        set_area_enabled(i, enabled);
    }

    if (!j.contains(AREA_TAG_MRECORDS)) {
        return;
    }
    nlohmann::json mrecs = j[AREA_TAG_MRECORDS];

    for (size_t i = GUI_MRECORDS_START_INDEX; i < gui_areas.size(); ++i) {
        auto a = std::dynamic_pointer_cast<GUIAreaMRecBase>(gui_areas[i]);
        a->clear();
        bool enabled = a->set(mrecs);
        set_area_enabled(i, enabled);
    }

    main_container->show_all();
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
        SHOW_ERRORW("failed to load JSON: " << err)
        return;
    }

    Errs errs;
    if (!manager.parseJSON(j, errs)) {
        SHOW_ERRORW("failed to parse JSON: " << std::endl << errs.getPlainText())
        return;
    }

    load_json(j);
}

void
on_load_toml() {
    ustring title = "Выбрать TOML файл для импорта";

    auto filter_text = Gtk::FileFilter::create();
    filter_text->set_name("TOML");
    filter_text->add_pattern("*.toml");

    std::string path;
    if (!show_open_file_dialog(title, filter_text, path)) {
        return;
    }

    std::string err;
    toml::value t;
    if (!manager.loadTOML(path, t, err)) {
        SHOW_ERRORW("failed to load TOML: " << err)
        return;
    }

    Errs errs;
    if (!manager.parseTOML(t, errs)) {
        SHOW_ERRORW("failed to parse TOML: " << std::endl << errs.getPlainText())
        return;
    }

    nlohmann::json j;
    manager.emitJSON(j);

    load_json(j);
}

void
on_load_bin() {
    ustring title = "Выбрать FRU Image файл для импорта";

    auto filter_text = Gtk::FileFilter::create();
    filter_text->set_name("FRU Image");
    filter_text->add_pattern("*.bin");

    std::string path;
    if (!show_open_file_dialog(title, filter_text, path)) {
        return;
    }

    std::string err;
    bytes       bs;
    if (!manager.loadBinary(path, bs, err)) {
        SHOW_ERRORW("failed to load FRU Image: " << err)
        return;
    }

    Errs errs;
    if (!manager.parseBinary(bs, errs)) {
        SHOW_ERRORW("failed to parse FRU Image: " << std::endl << errs.getPlainText())
        return;
    }

    nlohmann::json j;
    manager.emitJSON(j);

    load_json(j);
}

bool
show_save_file_dialog(ustring title, ustring file, FileFilter filter, std::string &path) {
    ustring ok   = "Выбрать";
    ustring cncl = "Отмена";

    auto dialog =
        Gtk::FileChooserNative::create(title, Gtk::FILE_CHOOSER_ACTION_SAVE, ok, cncl);

    dialog->add_filter(filter);
    dialog->set_current_name(file);
    dialog->set_do_overwrite_confirmation(true);

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

bool
getAreaIfValid(size_t ind, nlohmann::json &j) {
    auto path = get_area_path(ind);

    if (!gui_areas[ind]->validate()) {
        Glib::RefPtr<Gtk::TreeSelection> selection = nav_tree->get_selection();
        selection->select(get_area_path(ind));

        gui_areas[curent_area]->hide(main_container);

        gui_areas[ind]->show(main_container);
        main_container->show_all();

        show_alert_dialog("Ошибка!", "Некорректные введенные данные");
        return false;
    }

    if (ind < GUI_MRECORDS_START_INDEX) {
        gui_areas[ind]->get(j);
        return true;
    } else {
        auto a = std::dynamic_pointer_cast<GUIAreaMRecBase>(gui_areas[ind]);
        a->get(j);

        return true;
    }
}

bool
manager_parse_ui() {
    nlohmann::json j;

    for (size_t i = 0; i < GUI_MRECORDS_START_INDEX; i++) {
        if (!get_area_enabled(i)) {
            continue;
        }

        if (!getAreaIfValid(i, j)) {
            return false;
        }
    }

    nlohmann::json jarray;
    for (size_t i = GUI_MRECORDS_START_INDEX; i < gui_areas.size(); ++i) {
        if (!get_area_enabled(i)) {
            continue;
        }

        if (!getAreaIfValid(i, jarray)) {
            return false;
        }
    }

    if (!jarray.empty()) {
        j[AREA_TAG_MRECORDS] = jarray;
    }

    Errs errs;
    if (!manager.parseJSON(j, errs)) {
        SHOW_ERRORW("failed to parse JSON: " << std::endl << errs.getPlainText())
        return false;
    }

    return true;
}

void
on_save_json() {
    if (!manager_parse_ui()) {
        return;
    }

    ustring title = "Выбрать JSON файл для экспорта";
    ustring file  = "out.json";

    auto filter_text = Gtk::FileFilter::create();
    filter_text->set_name("JSON");
    filter_text->add_pattern("*.json");

    std::string path;
    if (!show_save_file_dialog(title, file, filter_text, path)) {
        return;
    }

    Errs errs;
    if (!manager.saveJSON(path, errs)) {
        SHOW_ERRORW("failed to save JSON: " << std::endl << errs.getPlainText())
        return;
    }
}

void
on_save_toml() {
    if (!manager_parse_ui()) {
        return;
    }

    ustring title = "Выбрать TOML файл для экспорта";
    ustring file  = "out.toml";

    auto filter_text = Gtk::FileFilter::create();
    filter_text->set_name("TOML");
    filter_text->add_pattern("*.toml");

    std::string path;
    if (!show_save_file_dialog(title, file, filter_text, path)) {
        return;
    }

    Errs errs;
    if (!manager.saveTOML(path, errs)) {
        SHOW_ERRORW("failed to save TOML: " << std::endl << errs.getPlainText())
        return;
    }
}

void
on_save_bin() {
    if (!manager_parse_ui()) {
        return;
    }

    ustring title = "Выбрать FRU Image файл для экспорта";
    ustring file  = "out.bin";

    auto filter_text = Gtk::FileFilter::create();
    filter_text->set_name("FRU Image");
    filter_text->add_pattern("*.bin");

    std::string path;
    if (!show_save_file_dialog(title, file, filter_text, path)) {
        return;
    }

    Errs errs;
    if (!manager.saveBinary(path, errs)) {
        SHOW_ERRORW("failed to save FRU Image: " << std::endl << errs.getPlainText())
        return;
    }
}

void
on_tree_selection_changed() {
    Glib::RefPtr<Gtk::TreeSelection> selection = nav_tree->get_selection();
    Gtk::TreeModel::iterator         iter      = selection->get_selected();
    Gtk::TreeModel::Path             path(iter);
    size_t                           row_number = static_cast<size_t>(path[0]);

    if (row_number == curent_area) {
        return;
    }

    if (curent_area < gui_areas.size()) {
        gui_areas[curent_area]->hide(main_container);
    }
    gui_areas[row_number]->show(main_container);

    curent_area = row_number;
    main_container->show_all();
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

inline void
connect_menu_bar(Glib::RefPtr<Gtk::Builder> refBuilder) {
    // menu_new
    Gtk::MenuItem *menu_new;
    refBuilder->get_widget<Gtk::MenuItem>("menu_new", menu_new);
    menu_new->signal_activate().connect(sigc::ptr_fun(&on_new));

    // menu_import_json
    Gtk::MenuItem *menu_import_json;
    refBuilder->get_widget<Gtk::MenuItem>("menu_import_json", menu_import_json);
    menu_import_json->signal_activate().connect(sigc::ptr_fun(&on_load_json));

    // menu_import_toml
    Gtk::MenuItem *menu_import_toml;
    refBuilder->get_widget<Gtk::MenuItem>("menu_import_toml", menu_import_toml);
    menu_import_toml->signal_activate().connect(sigc::ptr_fun(&on_load_toml));

    // menu_import_bin
    Gtk::MenuItem *menu_import_bin;
    refBuilder->get_widget<Gtk::MenuItem>("menu_import_bin", menu_import_bin);
    menu_import_bin->signal_activate().connect(sigc::ptr_fun(&on_load_bin));

    // menu_export_json
    Gtk::MenuItem *menu_export_json;
    refBuilder->get_widget<Gtk::MenuItem>("menu_export_json", menu_export_json);
    menu_export_json->signal_activate().connect(sigc::ptr_fun(&on_save_json));

    // menu_export_toml
    Gtk::MenuItem *menu_export_toml;
    refBuilder->get_widget<Gtk::MenuItem>("menu_export_toml", menu_export_toml);
    menu_export_toml->signal_activate().connect(sigc::ptr_fun(&on_save_toml));

    // menu_export_bin
    Gtk::MenuItem *menu_export_bin;
    refBuilder->get_widget<Gtk::MenuItem>("menu_export_bin", menu_export_bin);
    menu_export_bin->signal_activate().connect(sigc::ptr_fun(&on_save_bin));
}

inline void
init_gui_areas() {
    gui_areas.emplace_back(std::make_shared<GUIAreaInternalUse>());
    gui_areas.emplace_back(std::make_shared<GUIAreaChassis>());
    gui_areas.emplace_back(std::make_shared<GUIAreaBoard>());
    gui_areas.emplace_back(std::make_shared<GUIAreaProduct>());
    gui_areas.emplace_back(std::make_shared<GUIAreaBackplaneP2PCon>());
    gui_areas.emplace_back(std::make_shared<GUIAreaAddressTable>());
    gui_areas.emplace_back(std::make_shared<GUIAreaPowerDistribution>());
    gui_areas.emplace_back(std::make_shared<GUIAreaActivationAndPowerMng>());
    gui_areas.emplace_back(std::make_shared<GUIAreaIPConnection>());
    gui_areas.emplace_back(std::make_shared<GUIAreaBoardP2PCon>());
    gui_areas.emplace_back(std::make_shared<GUIAreaRadialIPMB0Topology>());
    gui_areas.emplace_back(std::make_shared<GUIAreaFanGeography>());
    gui_areas.emplace_back(std::make_shared<GUIAreaFormFactor>());
    gui_areas.emplace_back(std::make_shared<GUIAreaLEDDescriptor>());
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

    auto csspr = Gtk::CssProvider::create();
    csspr->load_from_data("* {font-size:12px;} .array-button {font-size:10px;} ");

    auto style_context = win->get_style_context();
    auto screen        = Gdk::Screen::get_default();
    style_context->add_provider_for_screen(
        screen, csspr, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

    app->add_window(*win);
    win->set_visible(true);

    connect_menu_bar(refBuilder);

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

    init_gui_areas();

    gui_areas[0]->show(main_container);
    curent_area = 0;
    main_container->show_all();

    win->show_all();
}

int
main(int argc, char **argv) {
    curent_area = 999999;

    app = Gtk::Application::create(argc, argv);

    app->signal_shutdown().connect(sigc::ptr_fun(&on_app_shutdown));

    app->signal_activate().connect([]() { on_app_activate(); });
    app->run();

    cleanup();
    return 0;
}
