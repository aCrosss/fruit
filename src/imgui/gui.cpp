#include <algorithm>
#include <array>
#include <cfloat>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <GLFW/glfw3.h>

#include "ImGuiFileDialog.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "common.hpp"
#include "encoding.hpp"
#include "manager.hpp"
#include "types.hpp"

using json = nlohmann::json;

constexpr int AREA_COUNT         = 14;
constexpr int FIRST_MRECORD_AREA = 4;

enum AreaIndex {
    UI_AREA_INTERNAL_USE,
    UI_AREA_CHASSIS,
    UI_AREA_BOARD,
    UI_AREA_PRODUCT,
    UI_AREA_BACKPLANE_P2P,
    UI_AREA_ADDRESS_TABLE,
    UI_AREA_POWER_DISTRIBUTION,
    UI_AREA_ACTIVATION_AND_POWER,
    UI_AREA_IP_CONNECTION,
    UI_AREA_BOARD_P2P,
    UI_AREA_RADIAL_IPMB0,
    UI_AREA_FAN_GEOGRAPHY,
    UI_AREA_FORM_FACTOR,
    UI_AREA_LED_DESCRIPTOR,
};

struct AreaInfo {
    const char *name;
    const char *tag;
    int         picmg_record_id;
};

static constexpr std::array<AreaInfo, AREA_COUNT> AREA_INFOS{{
    {"Internal Use Area",                     "internal_use", 0                                },
    {"Chassis Info Area",                     "chassis",      0                                },
    {"Board Info Area",                       "board",        0                                },
    {"Product Info Area",                     "product",      0                                },
    {"Backplane Point-to-Point Connectivity", nullptr,        PICMGREC_BACKPLANE_P2PCON        },
    {"Address Table",                         nullptr,        PICMGREC_ADDRESS_TABLE           },
    {"Power Distribution",                    nullptr,        PICMGREC_POWER_DISTRIBUTION      },
    {"Shelf Activation and Power Management", nullptr,        PICMGREC_ACTIVATION_AND_POWER_MNG},
    {"IP Connection",                         nullptr,        PICMGREC_IP_CONNECTION           },
    {"Board Point-to-Point Connectivity",     nullptr,        PICMGREC_BOARD_P2PCON            },
    {"Radial IPMB-0 Link Mapping",            nullptr,        PICMGREC_RADIAL_IPMB0_LINK_MAP   },
    {"Shelf Fan Geography",                   nullptr,        PICMGREC_FAN_GEOGRAPHY           },
    {"PICMG Form Factor",                     nullptr,        PICMGREC_FORM_FACTOR             },
    {"LED Description",                       nullptr,        PICMGREC_LED_DESCRIPTOR          },
}};

enum class DialogAction {
    None,
    ImportJson,
    ImportToml,
    ImportBinary,
    ExportJson,
    ExportToml,
    ExportBinary,
};

struct App {
    Manager manager;
    json    document = json::object();

    int     current_area = 0;
    bool    area_enabled[AREA_COUNT]{};
    bool    dark_theme = true;

    std::string error_message;
    bool        show_error = false;
    bool        request_new = false;

    DialogAction dialog_action = DialogAction::None;
};

static void
apply_theme(bool dark_theme) {
    if (dark_theme) {
        ImGui::StyleColorsDark();
    } else {
        ImGui::StyleColorsLight();
    }
}

static bool
load_application_font(ImGuiIO &io) {
    std::vector<std::string> candidates;
#ifdef _WIN32
    if (const char *windows_dir = std::getenv("WINDIR")) {
        candidates.emplace_back(std::string(windows_dir) + "\\Fonts\\segoeui.ttf");
        candidates.emplace_back(std::string(windows_dir) + "\\Fonts\\arial.ttf");
    }
#else
    candidates.emplace_back("/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf");
    candidates.emplace_back("/usr/share/fonts/noto/NotoSans-Regular.ttf");
    candidates.emplace_back("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");
    candidates.emplace_back("/usr/share/fonts/TTF/DejaVuSans.ttf");
    candidates.emplace_back("/usr/share/fonts/dejavu/DejaVuSans.ttf");
    candidates.emplace_back("/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf");
#endif

    for (const std::string &path : candidates) {
        std::FILE *font_file = std::fopen(path.c_str(), "rb");
        if (font_file == nullptr) {
            continue;
        }
        std::fclose(font_file);

        if (io.Fonts->AddFontFromFileTTF(path.c_str(), 18.0F, nullptr, io.Fonts->GetGlyphRangesCyrillic()) != nullptr) {
            return true;
        }
    }

    io.Fonts->AddFontDefault();
    return false;
}

struct EnumIntOption {
    const char *label;
    int         value;
};

struct EnumTextOption {
    const char *label;
    const char *value;
};

static const char *const LANGUAGE_NAMES[] = {
    "English",        "Afar",        "Abkhazian",      "Afrikaans",   "Amharic",
    "Arabic",         "Assamese",    "Aymara",         "Azerbaijani", "Bashkir",
    "Byelorussian",   "Bulgarian",   "Bihari",         "Bislama",     "Bengali",
    "Tibetan",        "Breton",      "Catalan",        "Corsican ",   "Czech",
    "Welsh",          "danish",      "german",         "Bhutani",     "Greek",
    "English",        "Esperanto",   "Spanish",        "Estonian",    "Basque",
    "Persian",        "Finnish",     "Fiji",           "Faeroese",    "French",
    "Frisian",        "Irish ",      "Scots Gaelic",   "Galician",    "Guarani",
    "Gujarati",       "Hausa",       "Hindi",          "Croatian",    "Hungarian",
    "Armenian",       "Interlingua", "Interlingue",    "Inupiak",     "Indonesian",
    "Icelandic",      "Italian",     "Hebrew",         "Japanese",    "Yiddish",
    "Javanese",       "Georgian",    "Kazakh",         "Greenlandic", "Cambodian",
    "Kannada",        "Korean",      "Kashmiri",       "Kurdish",     "Kirghiz",
    "Latin",          "Lingala",     "Laothian",       "Lithuanian",  "Latvian,",
    "Malagasy",       "Maori",       "Macedonian",     "Malayalam",   "Mongolian",
    "Moldavian",      "Marathi",     "Malay",          "Maltese",     "Burmese",
    "Nauru ",         "Nepali",      "Dutch",          "Norwegian",   "Occitan",
    "(Afan) Oromo",   "Oriya",       "Punjabi",        "Polish",      "Pashto, Pushto",
    "Portuguese",     "Quechua",     "Rhaeto-Romance", "Kirundi",     "Romanian",
    "Russian",        "Kinyarwanda", "Sanskrit",       "Sindhi",      "Sangro",
    "Serbo-Croatian", "Singhalese",  "Slovak",         "Slovenian",   "Samoan",
    "Shona",          "Somali",      "Albanian",       "Serbian",     "Siswati",
    "Sesotho",        "Sudanese",    "Swedish",        "Swahili",     "Tamil",
    "Tegulu",         "Tajik",       "Thai",           "Tigrinya",    "Turkmen",
    "Tagalog",        "Setswana",    "Tonga ",         "Turkish",     "Tsonga",
    "Tatar",          "Twi",         "Ukrainian",      "Urdu",        "Uzbek",
    "Vietnamese",     "Volapuk",     "Wolof",          "Xhosa",       "Yoruba",
    "Chinese",        "Zulu ",
};

static constexpr EnumIntOption CHASSIS_TYPES[] = {
    {"Other",                0x01}, {"Unknown",               0x02},
    {"Desktop",              0x03}, {"Low Profile Desktop",   0x04},
    {"Pizza Box",            0x05}, {"Mini Tower",            0x06},
    {"Tower",                0x07}, {"Portable",              0x08},
    {"LapTop",               0x09}, {"Notebook",              0x0A},
    {"Hand Held",            0x0B}, {"Docking Station",       0x0C},
    {"All in One",           0x0D}, {"Sub Notebook",          0x0E},
    {"Space-saving",         0x0F}, {"Lunch Box",             0x10},
    {"Main Server Chassis",  0x11}, {"Expansion Chassis",     0x12},
    {"SubChassis",           0x13}, {"Bus Expansion Chassis", 0x14},
    {"Peripheral Chassis",   0x15}, {"RAID Chassis",          0x16},
    {"Rack Mount Chassis",   0x17}, {"Sealed-case PC",        0x18},
    {"Multi-system chassis", 0x19}, {"CompactPCI",            0x1A},
    {"AdvancedTCA",          0x1B}, {"Blade",                 0x1C},
    {"Blade Enclosure",      0x1D},
};

static constexpr EnumIntOption SITE_TYPES[] = {
    {"Front Board",            0x00},
    {"Power Entry",            0x01},
    {"Shelf FRU Information",  0x02},
    {"Dedicated ShMC",         0x03},
    {"Fan Tray",               0x04},
    {"Fan Filter Tray",        0x05},
    {"Alarm",                  0x06},
    {"AdvancedMC Module",      0x07},
    {"PMC",                    0x08},
    {"Rear Transition Module", 0x09},
    {"OEM-C0",                 0xC0},
    {"OEM-C1",                 0xC1},
    {"OEM-C2",                 0xC2},
    {"OEM-C3",                 0xC3},
    {"OEM-C4",                 0xC4},
    {"OEM-C5",                 0xC5},
    {"OEM-C6",                 0xC6},
    {"OEM-C7",                 0xC7},
    {"OEM-C8",                 0xC8},
    {"OEM-C9",                 0xC9},
    {"OEM-CA",                 0xCA},
    {"OEM-CB",                 0xCB},
    {"OEM-CC",                 0xCC},
    {"OEM-CD",                 0xCD},
    {"OEM-CE",                 0xCE},
    {"OEM-CF",                 0xCF},
    {"Unknown",                0xFF},
};

static constexpr EnumIntOption CHANNEL_TYPES[] = {
    {"Single Port Fabric Interface",      0x08},
    {"Double Port Fabric Interface",      0x09},
    {"Full Channel Fabric Interface",     0x0A},
    {"Base Interface",                    0x0B},
    {"Update Channel Interface",          0x0C},
    {"Base Interface ShMC Cross-connect", 0x0D},
};

static constexpr EnumIntOption LINK_TYPES[] = {
    {"PICMG 3.0 Base Interface 10/100/1000 BASE-T", 0x01},
    {"PICMG 3.1 Ethernet Fabric Interface",         0x02},
    {"PICMG 3.2 Infiniband Fabric Interface",       0x03},
    {"PICMG 3.3 StarFabric Fabric Interface",       0x04},
    {"PICMG 3.4 PCI Express Fabric Interface",      0x05},
    {"OEM-C0",                                      0xF0},
    {"OEM-C1",                                      0xF1},
    {"OEM-C2",                                      0xF2},
    {"OEM-C3",                                      0xF3},
    {"OEM-C4",                                      0xF4},
    {"OEM-C5",                                      0xF5},
    {"OEM-C6",                                      0xF6},
    {"OEM-C7",                                      0xF7},
    {"OEM-C8",                                      0xF8},
    {"OEM-C9",                                      0xF9},
    {"OEM-CA",                                      0xFA},
    {"OEM-CB",                                      0xFB},
    {"OEM-CC",                                      0xFC},
    {"OEM-CD",                                      0xFD},
    {"OEM-CE",                                      0xFE},
};

static constexpr EnumIntOption LINK_TYPE_EXTENSIONS[] = {
    {"10/100/1000 BASE-T Link", 0x00},
    {"ShMC Cross-connect",      0x01},
};

static constexpr EnumTextOption INTERFACE_TYPES[] = {
    {"Base Interface",           "base"          },
    {"Fabric Interface",         "fabric"        },
    {"Update Channel Interface", "update_channel"},
};

static constexpr EnumTextOption BUS_COVERAGE[] = {
    {"IPMB-A only",       "ipmb_a"   },
    {"IPMB-B only",       "ipmb_b"   },
    {"IPMB-A and IPMB-B", "ipmb_both"},
};

static constexpr EnumTextOption FORM_FACTORS[] = {
    {"ATCA",     "ATCA"    },
    {"ATCA300",  "ATCA300" },
    {"MicroTCA", "MicroTCA"},
};

static json
encoded_string_default() {
    return {{"type", "unicode"}, {"data", ""}};
}

static json::number_unsigned_t
json_unsigned(int value) {
    return static_cast<json::number_unsigned_t>(value);
}

static json
mrecord_default(int picmg_record_id) {
    json record = {
        {FIELD_TAG_MRECORD_ID,       MRECORD_PICMG_RECORD},
        {FIELD_TAG_PICMG_MRECORD_ID, picmg_record_id     },
    };

    switch (picmg_record_id) {
        case PICMGREC_BACKPLANE_P2PCON: {
            record["slots"] = json::array();
            break;
        }
        case PICMGREC_ADDRESS_TABLE: {
            record["shelf_address"] = encoded_string_default();
            record["entries"]       = json::array();
            break;
        }
        case PICMGREC_POWER_DISTRIBUTION: {
            record["power_feeds"] = json::array();
            break;
        }
        case PICMGREC_ACTIVATION_AND_POWER_MNG: {
            record["allowance_for_activation"] = json_unsigned(0);
            record["entries"]                  = json::array();
            break;
        }
        case PICMGREC_IP_CONNECTION: {
            record["ip_address"]      = "";
            record["gateway_address"] = "";
            record["subnet_mask"]     = "";
            break;
        }
        case PICMGREC_BOARD_P2PCON: {
            record["guids"]            = json::array();
            record["link_descriptors"] = json::array();
            break;
        }
        case PICMGREC_RADIAL_IPMB0_LINK_MAP: {
            record["connector_definer"]    = "";
            record["connector_version_id"] = "";
            record["hub_descriptors"]      = json::array();
            break;
        }
        case PICMGREC_FAN_GEOGRAPHY: {
            record["entries"] = json::array();
            break;
        }
        case PICMGREC_FORM_FACTOR: {
            record["form_factor"] = "ATCA";
            break;
        }
        case PICMGREC_LED_DESCRIPTOR: {
            record["entries"] = json::array();
            break;
        }
        default:
            break;
    }

    return record;
}

static json
regular_area_default(int index) {
    switch (index) {
        case UI_AREA_INTERNAL_USE: {
            return {{"data", ""}};
        }
        case UI_AREA_CHASSIS:
            return {
                {"type",          json_unsigned(CHASSIS_TYPES[0].value)},
                {"part_number",   encoded_string_default()},
                {"serial_number", encoded_string_default()},
                {"custom",        json::array()           },
            };
        case UI_AREA_BOARD:
            return {
                {"language_code", json_unsigned(0)        },
                {"date_time",     ""                      },
                {"manufacturer",  encoded_string_default()},
                {"product_name",  encoded_string_default()},
                {"serial_number", encoded_string_default()},
                {"file_id",       encoded_string_default()},
                {"custom",        json::array()           },
            };
        case UI_AREA_PRODUCT:
            return {
                {"language_code", json_unsigned(0)        },
                {"manufacturer",  encoded_string_default()},
                {"product_name",  encoded_string_default()},
                {"part",          encoded_string_default()},
                {"version",       encoded_string_default()},
                {"serial_number", encoded_string_default()},
                {"asset_tag",     encoded_string_default()},
                {"fru_file_id",   encoded_string_default()},
                {"custom",        json::array()           },
            };
        default: return json::object();
    }
}

static bool
is_matching_mrecord(const json &record, int picmg_record_id) {
    return record.is_object() &&
        record.contains(FIELD_TAG_MRECORD_ID) &&
        record.contains(FIELD_TAG_PICMG_MRECORD_ID) &&
        record[FIELD_TAG_MRECORD_ID].is_number_integer() &&
        record[FIELD_TAG_PICMG_MRECORD_ID].is_number_integer() &&
        record[FIELD_TAG_MRECORD_ID].get<int>() == MRECORD_PICMG_RECORD &&
        record[FIELD_TAG_PICMG_MRECORD_ID].get<int>() == picmg_record_id;
}

static json *
find_mrecord(json &document, int picmg_record_id) {
    if (!document.is_object() || !document.contains(AREA_TAG_MRECORDS) ||
        !document[AREA_TAG_MRECORDS].is_array()) {
        return nullptr;
    }

    for (auto &record : document[AREA_TAG_MRECORDS]) {
        if (is_matching_mrecord(record, picmg_record_id)) {
            return &record;
        }
    }
    return nullptr;
}

static json &
ensure_area_document(App *app, int index) {
    if (!app->document.is_object()) {
        app->document = json::object();
    }

    const AreaInfo &info = AREA_INFOS[static_cast<size_t>(index)];
    if (index < FIRST_MRECORD_AREA) {
        const json defaults = regular_area_default(index);
        if (!app->document.contains(info.tag) || !app->document[info.tag].is_object()) {
            app->document[info.tag] = defaults;
        } else {
            json &area = app->document[info.tag];
            for (auto field = defaults.begin(); field != defaults.end(); ++field) {
                if (!area.contains(field.key())) {
                    area[field.key()] = field.value();
                }
            }
        }
        return app->document[info.tag];
    }

    if (!app->document.contains(AREA_TAG_MRECORDS) || !app->document[AREA_TAG_MRECORDS].is_array()) {
        app->document[AREA_TAG_MRECORDS] = json::array();
    }

    if (json *record = find_mrecord(app->document, info.picmg_record_id)) {
        return *record;
    }

    app->document[AREA_TAG_MRECORDS].push_back(mrecord_default(info.picmg_record_id));
    return app->document[AREA_TAG_MRECORDS].back();
}

static bool
area_has_field_previews(App *app) {
    const int area = app->current_area;
    if (area == UI_AREA_ACTIVATION_AND_POWER) {
        return true;
    }

    const char *array_tag = nullptr;
    switch (area) {
        case UI_AREA_BACKPLANE_P2P:      array_tag = "slots";            break;
        case UI_AREA_ADDRESS_TABLE:      array_tag = "entries";          break;
        case UI_AREA_POWER_DISTRIBUTION: array_tag = "power_feeds";      break;
        case UI_AREA_BOARD_P2P:          array_tag = "link_descriptors"; break;
        case UI_AREA_RADIAL_IPMB0:       array_tag = "hub_descriptors";  break;
        case UI_AREA_FAN_GEOGRAPHY:      array_tag = "entries";          break;
        case UI_AREA_LED_DESCRIPTOR:     array_tag = "entries";          break;
        default: return false;
    }

    const json &record = ensure_area_document(app, area);
    return record.contains(array_tag) && record[array_tag].is_array() && !record[array_tag].empty();
}

static void
sync_enabled_areas(App *app) {
    for (int i = 0; i < AREA_COUNT; ++i) {
        const AreaInfo &info = AREA_INFOS[static_cast<size_t>(i)];
        if (i < FIRST_MRECORD_AREA) {
            app->area_enabled[i] = app->document.is_object() && app->document.contains(info.tag);
        } else {
            app->area_enabled[i] = find_mrecord(app->document, info.picmg_record_id) != nullptr;
        }
    }
}

static json
enabled_document(App *app) {
    json result = json::object();
    for (int i = 0; i < FIRST_MRECORD_AREA; ++i) {
        if (!app->area_enabled[i]) {
            continue;
        }
        const AreaInfo &info = AREA_INFOS[static_cast<size_t>(i)];
        result[info.tag] = ensure_area_document(app, i);
    }

    json records = json::array();
    for (int i = FIRST_MRECORD_AREA; i < AREA_COUNT; ++i) {
        if (!app->area_enabled[i]) {
            continue;
        }
        records.push_back(ensure_area_document(app, i));
    }

    if (!records.empty()) {
        result[AREA_TAG_MRECORDS] = std::move(records);
    }
    return result;
}

static int
input_text_resize_callback(ImGuiInputTextCallbackData *data) {
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
        auto *value = static_cast<std::string *>(data->UserData);
        value->resize(static_cast<size_t>(data->BufTextLen));
        data->Buf = value->data();
    }
    return 0;
}

static bool
input_text(const char *label, std::string &value, ImGuiInputTextFlags flags = 0) {
    flags |= ImGuiInputTextFlags_CallbackResize;
    return ImGui::InputText(label, value.data(), value.capacity() + 1, flags, input_text_resize_callback, &value);
}

static bool
input_text_with_hint(const char *label, const char *hint, std::string &value) {
    return ImGui::InputTextWithHint(label, hint, value.data(), value.capacity() + 1, ImGuiInputTextFlags_CallbackResize, input_text_resize_callback, &value);
}

static bool
input_text_multiline(const char *label, std::string &value, const ImVec2 &size, ImGuiInputTextFlags flags = 0) {
    flags |= ImGuiInputTextFlags_CallbackResize;
    return ImGui::InputTextMultiline(label, value.data(), value.capacity() + 1, size, flags, input_text_resize_callback, &value);
}

// FIXME: proper localized error messages instead of this mess
static std::string
localized_validation_error(const std::string &error) {
    if (error == "string is empty") {
        return "пустая строка";
    }

    if (error == "invalid date/time values") {
        return "некорректные значения даты или времени";
    }

    if (error == "expected 'YYYY-MM-DD HH:mm' string") {
        return "ожидается строка в формате YYYY-MM-DD HH:mm";
    }

    if (error.rfind("failed at symbol '", 0) == 0) {
        return "некорректный символ; ожидается строка в формате YYYY-MM-DD HH:mm";
    }

    if (error == "date/time can't be less than 00:00 01-01-1996") {
        return "дата и время не могут быть раньше 00:00 01.01.1996";
    }

    if (error.rfind("unsupported charachter ", 0) == 0) {
        return "неподдерживаемый символ: " + error.substr(std::strlen("unsupported charachter "));
    }

    if (error == "uneven number of hexadeciamal charachters") {
        return "количество шестнадцатеричных символов должно быть четным";
    }

    if (error == "encode: unknown encoding") {
        return "неизвестная кодировка";
    }

    if (error == "6 bit ascii text must be encoded in groups of 3 bytes") {
        return "текст 6-bit ASCII должен кодироваться группами по 3 байта";
    }

    if (error == "out of bounds") {
        return "значение выходит за допустимые границы";
    }

    if (error == "encoded more than 64 bytes") {
        return "закодированное значение превышает 64 байта";
    }

    if (error == "unknown type code") {
        return "неизвестный код типа";
    }

    return error;
}

constexpr float FIELD_LABEL_WIDTH        = 270.0F;
constexpr float ENCODED_TYPE_COMBO_WIDTH = 150.0F;
constexpr float FIELD_PREVIEW_WIDTH      = 150.0F;

static bool show_field_preview_column = false;
static int  current_field_area        = UI_AREA_INTERNAL_USE;

static const char *
field_help(const char *label) {
    const auto is = [label](const char *candidate) {
        return std::strcmp(label, candidate) == 0;
    };

    switch (current_field_area) {
        case UI_AREA_INTERNAL_USE:
            if (is("Internal use data")) {
                return "Произвольные данные для контроллера управления или другого устройства, которому принадлежит эта область.\n\n"
                    "Источник: Platform Management FRU Information Storage Definition, таблица 9-1, стр. 5.";
            }
            break;

        case UI_AREA_CHASSIS:
            if (is("Type")) {
                return "Тип корпуса из стандартного перечня System Enclosure and Chassis Types.\n\n"
                    "Источник: Platform Management FRU Information Storage Definition, таблица 10-1 и раздел 14, стр. 6, 11.";
            }

            if (is("Part Number")) {
                return "Номер изделия (part number), присвоенный корпусу изготовителем.\n\n"
                    "Источник: Platform Management FRU Information Storage Definition, таблица 10-1, стр. 6.";
            }

            if (is("Serial Number")) {
                return "Серийный номер корпуса. Поле интерпретируется как англоязычная строка независимо от Language Code.\n\n"
                    "Источник: Platform Management FRU Information Storage Definition, таблица 10-1, стр. 6.";
            }

            if (is("Custom") || is("Value")) {
                return "Дополнительное поле сведений о корпусе, определяемое изготовителем.\n\n"
                    "Источник: Platform Management FRU Information Storage Definition, таблица 10-1, стр. 6.";
            }
            break;

        case UI_AREA_BOARD:
            if (is("Language Code")) {
                return "Код языка для текстовых полей Board Info Area.\n\n"
                    "Источник: Platform Management FRU Information Storage Definition, таблица 11-1 и раздел 15, стр. 7, 12.";
            }

            if (is("Mfg. Date/Time")) {
                return "Дата и время изготовления платы; в образе FRU хранится число минут от 00:00 01.01.1996.\n\n"
                    "Источник: Platform Management FRU Information Storage Definition, таблица 11-1, стр. 7.";
            }

            if (is("Manufacturer")) {
                return "Наименование изготовителя платы.\n\n"
                    "Источник: Platform Management FRU Information Storage Definition, таблица 11-1, стр. 7.";
            }

            if (is("Product Name")) {
                return "Наименование изделия, реализованного этой платой.\n\n"
                    "Источник: Platform Management FRU Information Storage Definition, таблица 11-1, стр. 7.";
            }

            if (is("Serial Number")) {
                return "Серийный номер платы; всегда кодируется как англоязычное поле.\n\n"
                    "Источник: Platform Management FRU Information Storage Definition, таблица 11-1, стр. 7.";
            }

            if (is("FRU File ID")) {
                return "Идентификатор файла FRU, использованного при изготовлении или обновлении устройства. Формат задает изготовитель.\n\n"
                    "Источник: Platform Management FRU Information Storage Definition, таблица 11-1, стр. 7.";
            }

            if (is("Custom") || is("Value")) {
                return "Дополнительное производственное поле, определяемое изготовителем.\n\n"
                    "Источник: Platform Management FRU Information Storage Definition, таблица 11-1, стр. 7.";
            }
            break;

        case UI_AREA_PRODUCT:
            if (is("Language Code")) {
                return "Код языка для текстовых полей Product Info Area.\n\n"
                    "Источник: Platform Management FRU Information Storage Definition, таблица 12-1 и раздел 15, стр. 8, 12.";
            }

            if (is("Manufacturer Name")) {
                return "Наименование изготовителя готового изделия.\n\n"
                    "Источник: Platform Management FRU Information Storage Definition, таблица 12-1, стр. 8.";
            }

            if (is("Product Name")) {
                return "Наименование готового изделия.\n\n"
                    "Источник: Platform Management FRU Information Storage Definition, таблица 12-1, стр. 8.";
            }

            if (is("Product Part/Model Number")) {
                return "Номер изделия или обозначение модели.\n\n"
                    "Источник: Platform Management FRU Information Storage Definition, таблица 12-1, стр. 8.";
            }

            if (is("Product Version")) {
                return "Версия готового изделия.\n\n"
                    "Источник: Platform Management FRU Information Storage Definition, таблица 12-1, стр. 8.";
            }

            if (is("Product Serial Number")) {
                return "Серийный номер готового изделия; всегда кодируется как англоязычное поле.\n\n"
                    "Источник: Platform Management FRU Information Storage Definition, таблица 12-1, стр. 8.";
            }

            if (is("Asset Tag")) {
                return "Инвентарная метка изделия, назначаемая владельцем или изготовителем.\n\n"
                    "Источник: Platform Management FRU Information Storage Definition, таблица 12-1, стр. 8.";
            }

            if (is("FRU File ID")) {
                return "Идентификатор файла FRU, использованного при изготовлении или обновлении изделия.\n\n"
                    "Источник: Platform Management FRU Information Storage Definition, таблица 12-1, стр. 8.";
            }

            if (is("Custom") || is("Value")) {
                return "Дополнительное поле сведений об изделии, определяемое изготовителем.\n\n"
                    "Источник: Platform Management FRU Information Storage Definition, таблица 12-1, стр. 8.";
            }
            break;

        case UI_AREA_BACKPLANE_P2P:
            if (is("Slot Description List")) {
                return "Список описателей слотов и реализованных на объединительной плате соединений точка-точка.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-45, стр. 3-117.";
            }

            if (is("Point-To-Point Channel Type")) {
                return "Тип интерфейса каналов, описываемых для данного слота.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-46, стр. 3-117.";
            }

            if (is("Slot Address")) {
                return "Адрес слота; в системе PICMG 3.0 это Hardware Address.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-46, стр. 3-117.";
            }

            if (is("Channel Descriptors")) {
                return "Соответствия локальных каналов каналам и слотам на удаленной стороне соединения.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-47, стр. 3-118.";
            }

            if (is("Local Channel")) {
                return "Номер канала в локальном слоте.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-47, стр. 3-118.";
            }

            if (is("Remote Channel")) {
                return "Номер канала в удаленном слоте, к которому проложено соединение.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-47, стр. 3-118.";
            }

            if (is("Remote Slot")) {
                return "Hardware Address удаленного слота.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-47, стр. 3-118.";
            }
            break;

        case UI_AREA_ADDRESS_TABLE:
            if (is("Shelf Address")) {
                return "Адрес стойки переменной длины, закодированный как строковое поле FRU.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-7, стр. 3-25.";
            }

            if (is("Address Table Entries")) {
                return "Таблица соответствия Hardware Address физическому адресу устройства (Site Number и Site Type).\n\n"
                    "Источник: PICMG 3.0 R3.0, таблицы 3-7 и 3-8, стр. 3-25.";
            }

            if (is("Hardware Address")) {
                return "Аппаратный адрес устройства на IPMB.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-8, стр. 3-25.";
            }

            if (is("Site Number")) {
                return "Числовая часть физического адреса устройства внутри выбранного Site Type.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-8, стр. 3-25.";
            }

            if (is("Site Type")) {
                return "Тип позиции устройства - плата, вентилятор, модуль питания и т. п.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблицы 3-8 и 3-9, стр. 3-25-3-26.";
            }
            break;

        case UI_AREA_POWER_DISTRIBUTION:
            if (is("Power Feeds")) {
                return "Список вводов питания и FRU-позиций, получающих питание от каждого ввода.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблицы 3-75 и 3-76, стр. 3-168-3-169.";
            }

            if (is("Maximum External Available Current")) {
                return "Максимальный ток, доступный шасси от внешнего источника по этому вводу.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-76, стр. 3-169.";
            }

            if (is("Maximum Internal Current")) {
                return "Максимальный ток, который выдерживают внутренние цепи шасси для этого ввода.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-76, стр. 3-169.";
            }

            if (is("Minimum Expected Operating Voltage")) {
                return "Минимальное ожидаемое рабочее напряжение, используемое для расчета наихудшего потребляемого тока.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-76, стр. 3-169.";
            }

            if (is("Mapping Entries")) {
                return "Перечень FRU, питающихся от данного ввода.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-77, стр. 3-170.";
            }

            if (is("Hardware Address")) {
                return "Hardware Address интеллектуального FRU, представляющего указанную позицию.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-77, стр. 3-170.";
            }

            if (is("FRU Device ID")) {
                return "Номер FRU Device ID по данному Hardware Address; FEh означает все FRU этого контроллера.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-77, стр. 3-170.";
            }
            break;

        case UI_AREA_ACTIVATION_AND_POWER:
            if (is("Allowance for FRU Activation Readiness")) {
                return "Время после запуска системы, за которое FRU должен перейти в состояние M3 и сохранить место в последовательности включения.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-78, стр. 3-171.";
            }

            if (is("Descriptors")) {
                return "Последовательность параметров активации и питания для FRU-позиций; порядок элементов задает порядок включения.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблицы 3-78 и 3-79, стр. 3-171-3-172.";
            }

            if (is("Hardware Address")) {
                return "Hardware Address устройства, к которому относится описатель питания.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-79, стр. 3-172.";
            }

            if (is("FRU Device ID")) {
                return "FRU Device ID позиции; FEh означает все FRU, представленные данным IPM Controller.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-79, стр. 3-172.";
            }

            if (is("Maximum FRU Power Capability")) {
                return "Максимальная мощность, которую разрешено подать на данную Hardware Address/FRU Device ID позицию.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-79, стр. 3-172.";
            }

            if (is("Shelf Manager Controlled Deactivation")) {
                return "Определяет, деактивирует ли Shelf Manager FRU при достижении состояния M5.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-79, стр. 3-172.";
            }

            if (is("Shelf Manager Controlled Activation")) {
                return "Определяет, активирует ли Shelf Manager FRU при достижении состояния M2.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-79, стр. 3-172.";
            }

            if (is("Delay Before Next Power On")) {
                return "Задержка перед включением следующего FRU после включения текущего; хранится с шагом 0,1 секунды.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-79, стр. 3-172.";
            }
            break;

        case UI_AREA_IP_CONNECTION:
            if (is("Shelf Manager IP Address")) {
                return "IPv4-адрес по умолчанию для подключения к Shelf Manager после его инициализации.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-35, стр. 3-83.";
            }

            if (is("Default Gateway Address")) {
                return "IPv4-адрес шлюза для сообщений и уведомлений, отправляемых в другую подсеть.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-35, стр. 3-83.";
            }

            if (is("Subnet Mask")) {
                return "Маска, отделяющая сетевую часть адреса Shelf Manager от адреса узла.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-35, стр. 3-83.";
            }
            break;

        case UI_AREA_BOARD_P2P:
            if (is("OEM GUID List")) {
                return "Список OEM GUID, используемых для сопоставления проприетарных типов соединений.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-49 и 3.7.2.3, стр. 3-123-3-124.";
            }

            if (is("OEM GUID")) {
                return "128-битный идентификатор OEM для проприетарного описания канала.\n\n"
                    "Источник: PICMG 3.0 R3.0, 3.7.2.3, абзац 318, стр. 3-124.";
            }

            if (is("Link Descriptor List")) {
                return "Список протоколов и групп портов, поддерживаемых платой на соединениях точка-точка.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблицы 3-49 и 3-50, стр. 3-123-3-124.";
            }

            if (is("Link Grouping ID")) {
                return "Объединяет порты нескольких каналов в многоканальное соединение; 0 означает отдельный канал.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-50, стр. 3-124.";
            }

            if (is("Link Type Extension")) {
                return "Уточняет вариант протокола внутри дочерней спецификации, выбранной полем Link Type.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-50, стр. 3-124.";
            }

            if (is("Link Type")) {
                return "Указывает дочернюю спецификацию PICMG 3.x или OEM-тип, управляющий этим соединением.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблицы 3-50 и 3-52, стр. 3-124-3-125.";
            }

            if (is("Port 3 Included") || is("Port 2 Included") || is("Port 1 Included") || is("Port 0 Included")) {
                return "Включает соответствующий порт канала в Link Designator.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-51, стр. 3-125.";
            }

            if (is("Interface")) {
                return "Выбирает Base, Fabric или Update Channel Interface для Link Designator.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-51, стр. 3-125.";
            }

            if (is("Channel Number")) {
                return "Номер канала выбранного интерфейса. Допустимое значение зависит от типа интерфейса.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-51, стр. 3-125.";
            }
            break;

        case UI_AREA_RADIAL_IPMB0:
            if (is("IPMB-0 Connector Definer")) {
                return "Трехбайтовый IANA ID организации, определившей разводку IPMB-0 Links на разъеме.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-64, стр. 3-148.";
            }

            if (is("IPMB-0 Connector Version ID")) {
                return "Версия схемы разводки разъема, заданной организацией IPMB-0 Connector Definer.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-64, стр. 3-148.";
            }

            if (is("IPMB-0 Hub Descriptors")) {
                return "Список концентраторов IPMB-0, их покрытия шин и соответствий линий адресам FRU.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблицы 3-64 и 3-65, стр. 3-147-3-148.";
            }

            if (is("Hardware Address")) {
                return "Hardware Address FRU, содержащего Hub, либо адрес FRU из таблицы соответствий - в зависимости от уровня записи.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблицы 3-65 и 3-66, стр. 3-148.";
            }

            if (is("Bus Coverage")) {
                return "Показывает, обслуживает ли Hub шину IPMB-A, IPMB-B или обе шины.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-65, стр. 3-148.";
            }

            if (is("IPMB-0 Link Mappings")) {
                return "Соответствия Hardware Address номерам радиальных линий IPMB-0 данного Hub.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-66, стр. 3-148.";
            }

            if (is("IPMB-0 Link Entry")) {
                return "Номер линии IPMB-0 от 1 до 95, по которой доступен указанный Hardware Address.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-66, стр. 3-148.";
            }
            break;

        case UI_AREA_FAN_GEOGRAPHY:
            if (is("Fan-to-FRU Entries")) {
                return "Соответствия позиций FRU вентиляторам, которые обеспечивают их охлаждение.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблицы 3-80 и 3-81, стр. 3-175-3-176.";
            }

            if (is("Hardware Address")) {
                return "Hardware Address контроллера Fan Tray.\n\nИсточник: PICMG 3.0 R3.0, таблица 3-81, стр. 3-176.";
            }

            if (is("FRU Device ID")) {
                return "FRU Device ID вентилятора; FEh означает все FRU Device ID по указанному адресу.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-81, стр. 3-176.";
            }

            if (is("Site Number")) {
                return "Site Number охлаждаемой FRU-позиции.\n\nИсточник: PICMG 3.0 R3.0, таблица 3-81, стр. 3-176.";
            }

            if (is("Site Type")) {
                return "Site Type охлаждаемой FRU-позиции.\n\nИсточник: PICMG 3.0 R3.0, таблица 3-81, стр. 3-176.";
            }
            break;

        case UI_AREA_FORM_FACTOR:
            if (is("Form Factor")) {
                return "Тип платформы PICMG, управляемой данным Shelf Manager: ATCA, ATCA300 или MicroTCA.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-5, стр. 3-22.";
            }
            break;

        case UI_AREA_LED_DESCRIPTOR:
            if (is("LED Descriptors")) {
                return "Список текстовых описаний светодиодов, идентифицируемых по LED ID.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблицы 3-33 и 3-34, стр. 3-76.";
            }

            if (is("LED ID")) {
                return "Идентификатор светодиода согласно таблице назначений LED ID.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблицы 3-28 и 3-34, стр. 3-71, 3-76.";
            }

            if (is("LED Legend")) {
                return "Текстовая надпись рядом со светодиодом на лицевой панели.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-34, стр. 3-76.";
            }

            if (is("LED Symbol")) {
                return "Текстовая ссылка на обозначение ISO/IEC для графического символа светодиода.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-34, стр. 3-76.";
            }

            if (is("LED Description")) {
                return "Развернутое имя или пояснение назначения светодиода.\n\n"
                    "Источник: PICMG 3.0 R3.0, таблица 3-34, стр. 3-76.";
            }
            break;
    }
    return nullptr;
}

static void
show_field_help(const char *label) {
    const char *help = field_help(label);
    if (help == nullptr || !ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
        return;
    }

    ImGui::BeginTooltip();
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 38.0F);
        {
            ImGui::TextUnformatted(help);
        }
        ImGui::PopTextWrapPos();
    }
    ImGui::EndTooltip();
}

static bool
begin_field(const char *label, bool row_has_preview = false) {
    ImGui::PushID(label);

    const bool                preview_column = show_field_preview_column || row_has_preview;
    constexpr ImGuiTableFlags flags          = ImGuiTableFlags_SizingFixedFit  |
                                               ImGuiTableFlags_NoSavedSettings |
                                               ImGuiTableFlags_NoPadOuterX;
    if (!ImGui::BeginTable("##field", preview_column ? 3 : 2, flags)) {
        ImGui::PopID();
        return false;
    }

    ImGui::TableSetupColumn("##label", ImGuiTableColumnFlags_WidthFixed, FIELD_LABEL_WIDTH);
    ImGui::TableSetupColumn("##editor", ImGuiTableColumnFlags_WidthStretch);
    if (preview_column) {
        ImGui::TableSetupColumn("##preview", ImGuiTableColumnFlags_WidthFixed, FIELD_PREVIEW_WIDTH);
    }
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::AlignTextToFramePadding();
    ImGui::Text("%s:", label);
    show_field_help(label);
    ImGui::TableSetColumnIndex(1);
    return true;
}

static void
end_field() {
    ImGui::EndTable();
    ImGui::PopID();
}

static std::string
number_suffix(int number, int maximum, const char *unit, double unit_scale) {
    const int  hex_digits = maximum <= 0xFF ? 2 : maximum <= 0xFFFF ? 4 : 8;
    const auto hex_value  = static_cast<unsigned int>(number);
    char       suffix[96];

    if (unit == nullptr) {
        std::snprintf(suffix, sizeof(suffix), "(0x%0*X)", hex_digits, hex_value);
    } else if (unit_scale == 1.0) {
        std::snprintf(suffix, sizeof(suffix), "(0x%0*X) %s", hex_digits, hex_value, unit);
    } else {
        std::snprintf(suffix, sizeof(suffix), "(0x%0*X) = %.1f %s", hex_digits, hex_value, number * unit_scale, unit);
    }
    return suffix;
}

static void
draw_bytes(const char *label, json &value) {
    if (!value.is_string()) {
        value = "";
    }

    std::string text = value.get<std::string>();

    if (!begin_field(label)) {
        return;
    }

    if (input_text_multiline("##value", text, ImVec2(-FLT_MIN, 90.0F))) {
        value = text;
    }

    bytes       parsed;
    std::string error;
    if (!hexStrToBytes(text, parsed, error)) {
        const std::string localized = localized_validation_error(error);
        ImGui::TextColored(ImVec4(1.0F, 0.35F, 0.35F, 1.0F), "%s", localized.c_str());
    }
    end_field();
}

static void
draw_encoded_string(const char *label, json &value, int byte_cap = 0) {
    static constexpr EnumTextOption ENCODINGS[] = {
        {"Binary",      "binary"   },
        {"BCD plus",    "bcdp"     },
        {"6-bit ASCII", "ascii6bit"},
        {"Unicode",     "unicode"  },
    };

    if (!value.is_object()) {
        value = encoded_string_default();
    }

    if (!value.contains("type") || !value["type"].is_string()) {
        value["type"] = ENCODINGS[3].value;
    }

    if (!value.contains("data") || !value["data"].is_string()) {
        value["data"] = "";
    }

    std::string encoding = value["type"].get<std::string>();
    std::string text     = value["data"].get<std::string>();

    int selected = 3;
    for (int i = 0; i < 4; ++i) {
        if (encoding == ENCODINGS[i].value) {
            selected = i;
            break;
        }
    }

    if (!begin_field(label)) {
        return;
    }

    const float available_width = ImGui::GetContentRegionAvail().x;
    const float text_width      = std::max(1.0F, available_width - ENCODED_TYPE_COMBO_WIDTH - ImGui::GetStyle().ItemSpacing.x);

    ImGui::SetNextItemWidth(text_width);
    if (input_text("##data", text)) {
        value["data"] = text;
    }

    ImGui::SameLine();
    ImGui::SetNextItemWidth(ENCODED_TYPE_COMBO_WIDTH);
    if (ImGui::BeginCombo("##encoding", ENCODINGS[selected].label)) {
        for (int i = 0; i < 4; ++i) {
            const bool is_selected = selected == i;
            if (ImGui::Selectable(ENCODINGS[i].label, is_selected)) {
                selected = i;
                encoding = ENCODINGS[i].value;
                value["type"] = encoding;
            }

            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    if (encoding_map.find(encoding) == encoding_map.end()) {
        encoding = ENCODINGS[selected].value;
        value["type"] = encoding;
    }

    auto encoding_it = encoding_map.find(encoding);
    if (encoding_it != encoding_map.end()) {
        Encoding    type         = encoding_it->second;
        std::string checked_text = text;
        int         length       = precalcLength(checked_text, type) - 1;
        int         cap          = byte_cap > 0 ? byte_cap : ENCODED_MAX_BYTE_LENGTH;
        if (length > cap) {
            ImGui::TextColored(ImVec4(1.0F, 0.35F, 0.35F, 1.0F), "Закодированное значение занимает больше %d байт", cap);
        } else {
            bytes       output;
            std::string error;
            if (!encode(checked_text, type, output, error)) {
                const std::string localized = localized_validation_error(error);
                ImGui::TextColored(ImVec4(1.0F, 0.35F, 0.35F, 1.0F), "%s", localized.c_str());
            }
        }
    }
    end_field();
}

static void
draw_number(const char *label, json &value, int minimum, int maximum, int step = 1, const char *unit = nullptr, double unit_scale = 1.0) {
    int number = value.is_number() ? value.get<int>() : minimum;

    number = std::clamp(number, minimum, maximum);
    value  = json_unsigned(number);

    if (!begin_field(label, true)) {
        return;
    }

    std::string suffix = number_suffix(number, maximum, unit, unit_scale);

    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::InputInt("##value", &number, step, step * 5)) {
        number = std::clamp(number, minimum, maximum);
        value = json_unsigned(number);
    }

    suffix = number_suffix(number, maximum, unit, unit_scale);
    ImGui::TableSetColumnIndex(2);
    ImGui::AlignTextToFramePadding();
    ImGui::TextDisabled("%s", suffix.c_str());
    end_field();
}

static void
draw_decimal(const char *label, json &value, double minimum, double maximum, double step, const char *unit = nullptr) {
    double number = value.is_number() ? value.get<double>() : minimum;

    number = std::clamp(number, minimum, maximum);
    value  = number;

    if (!begin_field(label, unit != nullptr)) {
        return;
    }

    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::InputDouble("##value", &number, step, step * 10.0, "%.1f")) {
        number = std::clamp(number, minimum, maximum);
        value = number;
    }

    if (unit != nullptr) {
        ImGui::TableSetColumnIndex(2);
        ImGui::AlignTextToFramePadding();
        ImGui::TextDisabled("%s", unit);
    }
    end_field();
}

template <size_t N>
static void
draw_enum(const char *label, json &value, const EnumIntOption (&options)[N]) {
    int    current  = value.is_number_integer() ? value.get<int>() : options[0].value;
    size_t selected = 0;
    for (size_t i = 0; i < N; ++i) {
        if (options[i].value == current) {
            selected = i;
            break;
        }
    }
    value = json_unsigned(options[selected].value);

    if (!begin_field(label)) {
        return;
    }

    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo("##value", options[selected].label)) {
        for (size_t i = 0; i < N; ++i) {
            const bool is_selected = i == selected;
            if (ImGui::Selectable(options[i].label, is_selected)) {
                value = json_unsigned(options[i].value);
                selected = i;
            }

            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    end_field();
}

template <size_t N>
static void
draw_enum(const char *label, json &value, const EnumTextOption (&options)[N]) {
    std::string current  = value.is_string() ? value.get<std::string>() : options[0].value;
    size_t      selected = 0;

    for (size_t i = 0; i < N; ++i) {
        if (current == options[i].value) {
            selected = i;
            break;
        }
    }
    value = options[selected].value;

    if (!begin_field(label)) {
        return;
    }

    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo("##value", options[selected].label)) {
        for (size_t i = 0; i < N; ++i) {
            const bool is_selected = i == selected;
            if (ImGui::Selectable(options[i].label, is_selected)) {
                value = options[i].value;
                selected = i;
            }

            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    end_field();
}

static void
draw_language(const char *label, json &value) {
    constexpr size_t LANGUAGE_COUNT = sizeof(LANGUAGE_NAMES) / sizeof(LANGUAGE_NAMES[0]);
    int              selected       = value.is_number_integer() ? value.get<int>() : 0;

    selected = std::clamp(selected, 0, static_cast<int>(LANGUAGE_COUNT - 1));
    value   = json_unsigned(selected);

    if (!begin_field(label)) {
        return;
    }

    ImGui::SetNextItemWidth(-FLT_MIN);
    if (ImGui::BeginCombo("##value", LANGUAGE_NAMES[selected])) {
        for (size_t i = 0; i < LANGUAGE_COUNT; ++i) {
            const bool is_selected = static_cast<int>(i) == selected;
            std::string display = std::to_string(i) + " - " + LANGUAGE_NAMES[i];
            if (ImGui::Selectable(display.c_str(), is_selected)) {
                value = json_unsigned(static_cast<int>(i));
                selected = static_cast<int>(i);
            }

            if (is_selected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    end_field();
}

static void
draw_checkbox(const char *label, json &value) {
    bool checked = value.is_boolean() ? value.get<bool>() : false;

    if (!begin_field(label)) {
        return;
    }

    if (ImGui::Checkbox("##value", &checked) || !value.is_boolean()) {
        value = checked;
    }
    end_field();
}

static void
draw_datetime(const char *label, json &value) {
    if (!value.is_string()) {
        value = "";
    }

    std::string text = value.get<std::string>();

    if (!begin_field(label)) {
        return;
    }

    ImGui::SetNextItemWidth(-FLT_MIN);
    if (input_text_with_hint("##value", "YYYY-MM-DD HH:mm", text)) {
        value = text;
    }

    std::string error;
    if (parseDateTime(text, error) < 0) {
        const std::string localized = localized_validation_error(error);
        ImGui::TextColored(ImVec4(1.0F, 0.35F, 0.35F, 1.0F), "%s", localized.c_str());
    }
    end_field();
}

enum class ValidatedStringType { Ip, Guid, Hex };

static bool
validate_ip(const std::string &text, std::string &error) {
    char dot = 0;
    int  parts[4]{};

    std::istringstream input(text);
    if (!(input >> parts[0] >> dot >> parts[1] >> dot >> parts[2] >> dot >> parts[3])) {
        error = "IP-адрес должен иметь формат [0-255].[0-255].[0-255].[0-255]";
        return false;
    }

    for (int part : parts) {
        if (part < 0 || part > 255) {
            error = "IP-адрес должен иметь формат [0-255].[0-255].[0-255].[0-255]";
            return false;
        }
    }
    return true;
}

static bool
validate_hex(const std::string &text, size_t expected_length, std::string &error) {
    std::string copy = text;

    bytes parsed;
    if (!hexStrToBytes(copy, parsed, error)) {
        return false;
    }

    if (expected_length > 0 && parsed.size() != expected_length) {
        error = "ожидалось байт: " + std::to_string(expected_length) + ", получено: " + std::to_string(parsed.size());
        return false;
    }
    return true;
}

static bool
validate_guid(const std::string &text, std::string &error) {
    if (text.length() < 32) {
        error = "недостаточно шестнадцатеричных символов в GUID";
        return false;
    }

    if (!validate_hex(text, 16, error)) {
        return false;
    }
    return true;
}

static void
draw_validated_string(const char *label, json &value, ValidatedStringType type, size_t expected_length = 0) {
    const char *hint = "";

    switch (type) {
        case ValidatedStringType::Ip:   hint = "0.0.0.0";                              break;
        case ValidatedStringType::Guid: hint = "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"; break;
        case ValidatedStringType::Hex:  hint = "0A0B0C";                               break;
    }

    if (!value.is_string()) {
        value = "";
    }

    std::string text = value.get<std::string>();
    if (!begin_field(label)) {
        return;
    }

    ImGui::SetNextItemWidth(-FLT_MIN);
    if (input_text_with_hint("##value", hint, text)) {
        value = text;
    }

    std::string error;
    bool        valid = false;
    switch (type) {
        case ValidatedStringType::Ip:   valid = validate_ip(text, error);                   break;
        case ValidatedStringType::Guid: valid = validate_guid(text, error);                 break;
        case ValidatedStringType::Hex:  valid = validate_hex(text, expected_length, error); break;
    }

    if (!valid) {
        const std::string localized = localized_validation_error(error);
        ImGui::TextColored(ImVec4(1.0F, 0.35F, 0.35F, 1.0F), "%s", localized.c_str());
    }
    end_field();
}

template <typename DrawEntry>
static void
draw_array(const char *label, const char *entry_label, json &array, const json &new_entry, DrawEntry draw_entry) {
    if (!array.is_array()) {
        array = json::array();
    }

    ImGui::PushID(label);
    ImGui::SeparatorText(label);
    show_field_help(label);
    for (size_t i = 0; i < array.size(); ++i) {
        bool changed = false;

        ImGui::PushID(static_cast<int>(i));
        {
            ImGui::Text("%s %zu", entry_label, i + 1);

            ImGui::Indent();
            {
                draw_entry(array[i]);
            }
            ImGui::Unindent();

            ImGui::Dummy(ImVec2(0.0F, ImGui::GetStyle().ItemSpacing.y * 2.0F));

            ImGui::BeginDisabled(i == 0);
            if (ImGui::SmallButton("Вверх")) {
                std::swap(array[i], array[i - 1]);
                changed = true;
            }
            ImGui::EndDisabled();

            ImGui::SameLine();

            ImGui::BeginDisabled(i + 1 >= array.size());
            if (ImGui::SmallButton("Вниз")) {
                std::swap(array[i], array[i + 1]);
                changed = true;
            }
            ImGui::EndDisabled();

            ImGui::SameLine();

            if (ImGui::SmallButton("Дублировать")) {
                const json duplicate = array[i];
                array.insert(array.begin() + static_cast<json::difference_type>(i + 1), duplicate);
                changed = true;
            }

            ImGui::SameLine();

            if (ImGui::SmallButton("Удалить")) {
                array.erase(array.begin() + static_cast<json::difference_type>(i));
                changed = true;
            }
        }
        ImGui::PopID();

        if (changed) {
            break;
        }
        ImGui::Spacing();
    }

    if (ImGui::Button("Добавить")) {
        array.push_back(new_entry);
    }
    ImGui::PopID();
}

static void
draw_encoded_string_array(const char *label, json &array) {
    draw_array(label, "Custom Field", array, encoded_string_default(), [](json &entry) {
               draw_encoded_string("Value", entry);
               });
}

static void
draw_internal_use(App *app) {
    json &area = ensure_area_document(app, UI_AREA_INTERNAL_USE);

    draw_bytes("Internal use data", area["data"]);
}

static void
draw_chassis(App *app) {
    json &area = ensure_area_document(app, UI_AREA_CHASSIS);

    draw_enum                ("Type",          area["type"], CHASSIS_TYPES);
    draw_encoded_string      ("Part Number",   area["part_number"]);
    draw_encoded_string      ("Serial Number", area["serial_number"]);
    draw_encoded_string_array("Custom",        area["custom"]);
}

static void
draw_board(App *app) {
    json &area = ensure_area_document(app, UI_AREA_BOARD);

    draw_language            ("Language Code",  area["language_code"]);
    draw_datetime            ("Mfg. Date/Time", area["date_time"]);
    draw_encoded_string      ("Manufacturer",   area["manufacturer"]);
    draw_encoded_string      ("Product Name",   area["product_name"]);
    draw_encoded_string      ("Serial Number",  area["serial_number"]);
    draw_encoded_string      ("FRU File ID",    area["file_id"]);
    draw_encoded_string_array("Custom",         area["custom"]);
}

static void
draw_product(App *app) {
    json &area = ensure_area_document(app, UI_AREA_PRODUCT);

    draw_language            ("Language Code",             area["language_code"]);
    draw_encoded_string      ("Manufacturer Name",         area["manufacturer"]);
    draw_encoded_string      ("Product Name",              area["product_name"]);
    draw_encoded_string      ("Product Part/Model Number", area["part"]);
    draw_encoded_string      ("Product Version",           area["version"]);
    draw_encoded_string      ("Product Serial Number",     area["serial_number"]);
    draw_encoded_string      ("Asset Tag",                 area["asset_tag"]);
    draw_encoded_string      ("FRU File ID",               area["fru_file_id"]);
    draw_encoded_string_array("Custom",                    area["custom"]);
}

static void
draw_backplane_p2p(App *app) {
    json &record = ensure_area_document(app, UI_AREA_BACKPLANE_P2P);

    json channel_default = {
        {"local_channel",  json_unsigned(0)},
        {"remote_channel", json_unsigned(0)},
        {"remote_slot",    json_unsigned(0)},
    };

    json slot_default = {
        {"type",     json_unsigned(CHANNEL_TYPES[0].value)},
        {"address",  json_unsigned(0)                     },
        {"channels", json::array()                        },
    };

    draw_array("Slot Description List", "Slot Description", record["slots"], slot_default, [&](json &slot) {
        if (!slot.is_object()) {
            slot = slot_default;
        }

        draw_enum  ("Point-To-Point Channel Type", slot["type"],    CHANNEL_TYPES);
        draw_number("Slot Address",                slot["address"], 0, 255);

        draw_array("Channel Descriptors", "Channel Descriptor", slot["channels"], channel_default, [&](json &channel) {
            if (!channel.is_object()) {
                channel = channel_default;
            }

            draw_number("Local Channel",  channel["local_channel"],  0, 31);
            draw_number("Remote Channel", channel["remote_channel"], 0, 31);
            draw_number("Remote Slot",    channel["remote_slot"],    0, 255);
        });
    });
}

static void
draw_address_table(App *app) {
    json &record = ensure_area_document(app, UI_AREA_ADDRESS_TABLE);

    draw_encoded_string("Shelf Address", record["shelf_address"], 20);

    const json entry_default = {
        {"hardware_address", json_unsigned(0)                  },
        {"site_number",      json_unsigned(0)                  },
        {"site_type",        json_unsigned(SITE_TYPES[0].value)},
    };

    draw_array("Address Table Entries", "Address Table Entry", record["entries"], entry_default, [&](json &entry) {
        if (!entry.is_object()) {
            entry = entry_default;
        }

        draw_number("Hardware Address", entry["hardware_address"], 0, 255);
        draw_number("Site Number",      entry["site_number"],      0, 255);
        draw_enum  ("Site Type",        entry["site_type"],        SITE_TYPES);
    });
}

static void
draw_power_distribution(App *app) {
    json &record = ensure_area_document(app, UI_AREA_POWER_DISTRIBUTION);

    const json mapping_default = {
        {"hardware_address", json_unsigned(0)},
        {"fru_device_id",    json_unsigned(0)},
    };

    const json feed_default = {
        {"max_external_current", 0.0          },
        {"max_internal_current", 0.0          },
        {"min_expected_voltage", 36.0         },
        {"entries",              json::array()},
    };

    draw_array("Power Feeds", "Power Feed", record["power_feeds"], feed_default, [&](json &feed) {
        if (!feed.is_object()) {
            feed = feed_default;
        }

        draw_decimal("Maximum External Available Current", feed["max_external_current"], 0.0,  25.6, 0.1, "A");
        draw_decimal("Maximum Internal Current",           feed["max_internal_current"], 0.0,  25.6, 0.1, "A");
        draw_decimal("Minimum Expected Operating Voltage", feed["min_expected_voltage"], 36.0, 72.0, 0.5, "V");
        draw_array("Mapping Entries", "Mapping Entry", feed["entries"], mapping_default, [&](json &entry) {
            if (!entry.is_object()) {
                entry = mapping_default;
            }

            draw_number("Hardware Address", entry["hardware_address"], 0, 255);
            draw_number("FRU Device ID", entry["fru_device_id"], 0, 255);
            });
        });
}

static void
draw_activation_and_power_management(App *app) {
    json &record = ensure_area_document(app, UI_AREA_ACTIVATION_AND_POWER);

    draw_number("Allowance for FRU Activation Readiness", record["allowance_for_activation"], 0, 255, 1, "s");

    const json entry_default = {
        {"hardware_address",        json_unsigned(0)},
        {"fru_device_id",           json_unsigned(0)},
        {"max_fru_power_cap",       json_unsigned(0)},
        {"controlled_deactivation", false           },
        {"controlled_activation",   false           },
        {"next_power_on_delay",     json_unsigned(0)},
    };

    draw_array("Descriptors", "Activation Descriptor", record["entries"], entry_default, [&](json &entry) {
        if (!entry.is_object()) {
            entry = entry_default;
        }

        draw_number  ("Hardware Address",                      entry["hardware_address"],  0, 255);
        draw_number  ("FRU Device ID",                         entry["fru_device_id"],     0, 255);
        draw_number  ("Maximum FRU Power Capability",          entry["max_fru_power_cap"], 0, 65535, 1, "W");
        draw_checkbox("Shelf Manager Controlled Deactivation", entry["controlled_deactivation"]);
        draw_checkbox("Shelf Manager Controlled Activation",   entry["controlled_activation"]);
        draw_number  ("Delay Before Next Power On",            entry["next_power_on_delay"], 0, 63, 1, "s", 0.1);
    });
}

static void
draw_ip_connection(App *app) {
    json &record = ensure_area_document(app, UI_AREA_IP_CONNECTION);

    draw_validated_string("Shelf Manager IP Address", record["ip_address"],      ValidatedStringType::Ip);
    draw_validated_string("Default Gateway Address",  record["gateway_address"], ValidatedStringType::Ip);
    draw_validated_string("Subnet Mask",              record["subnet_mask"],     ValidatedStringType::Ip);
}

static void
draw_board_p2p(App *app) {
    json &record = ensure_area_document(app, UI_AREA_BOARD_P2P);
    draw_array("OEM GUID List", "OEM GUID", record["guids"], "", [](json &entry) {
        draw_validated_string("OEM GUID", entry, ValidatedStringType::Guid);
    });

    const json link_default = {
        {"link_grouping_id",    json_unsigned(0)                            },
        {"link_type_extension", json_unsigned(LINK_TYPE_EXTENSIONS[0].value)},
        {"link_type",           json_unsigned(LINK_TYPES[0].value)          },
        {"port_3",              false                                       },
        {"port_2",              false                                       },
        {"port_1",              false                                       },
        {"port_0",              false                                       },
        {"interface",           INTERFACE_TYPES[0].value                    },
        {"channel_number",      json_unsigned(0)                            },
    };

    draw_array("Link Descriptor List", "Link Descriptor", record["link_descriptors"], link_default, [&](json &entry) {
        if (!entry.is_object()) {
            entry = link_default;
        }

        draw_number  ("Link Grouping ID",    entry["link_grouping_id"], 0, 63);
        draw_enum    ("Link Type Extension", entry["link_type_extension"], LINK_TYPE_EXTENSIONS);
        draw_enum    ("Link Type",           entry["link_type"], LINK_TYPES);
        draw_checkbox("Port 3 Included",     entry["port_3"]);
        draw_checkbox("Port 2 Included",     entry["port_2"]);
        draw_checkbox("Port 1 Included",     entry["port_1"]);
        draw_checkbox("Port 0 Included",     entry["port_0"]);
        draw_enum    ("Interface",           entry["interface"], INTERFACE_TYPES);
        draw_number  ("Channel Number",      entry["channel_number"], 0, 63);
    });
}

static void
draw_radial_ipmb0_topology(App *app) {
    json &record = ensure_area_document(app, UI_AREA_RADIAL_IPMB0);

    draw_validated_string("IPMB-0 Connector Definer",    record["connector_definer"],    ValidatedStringType::Hex, 3);
    draw_validated_string("IPMB-0 Connector Version ID", record["connector_version_id"], ValidatedStringType::Hex, 2);

    const json mapping_default = {
        {"hardware_address", json_unsigned(0)},
        {"ipmb0_link_entry", json_unsigned(1)},
    };

    const json hub_default = {
        {"hardware_address", json_unsigned(0)     },
        {"bus_coverage",     BUS_COVERAGE[0].value},
        {"link_mappings",    json::array()        },
    };

    draw_array("IPMB-0 Hub Descriptors", "IPMB-0 Hub Descriptor", record["hub_descriptors"], hub_default, [&](json &hub) {
        if (!hub.is_object()) {
            hub = hub_default;
        }

        draw_number("Hardware Address",     hub["hardware_address"], 0, 255);
        draw_enum  ("Bus Coverage",         hub["bus_coverage"], BUS_COVERAGE);
        draw_array ("IPMB-0 Link Mappings", "IPMB-0 Link Mapping", hub["link_mappings"], mapping_default, [&](json &entry) {
            if (!entry.is_object()) {
                entry = mapping_default;
            }

            draw_number("Hardware Address",  entry["hardware_address"], 0, 255);
            draw_number("IPMB-0 Link Entry", entry["ipmb0_link_entry"], 1, 95);
        });
    });
}

static void
draw_fan_geography(App *app) {
    json &record = ensure_area_document(app, UI_AREA_FAN_GEOGRAPHY);

    const json entry_default = {
        {"hardware_address", json_unsigned(0)                  },
        {"fru_device_id",    json_unsigned(0)                  },
        {"site_number",      json_unsigned(0)                  },
        {"site_type",        json_unsigned(SITE_TYPES[0].value)},
    };

    draw_array("Fan-to-FRU Entries", "Fan-to-FRU Entry", record["entries"], entry_default, [&](json &entry) {
        if (!entry.is_object()) {
            entry = entry_default;
        }

        draw_number("Hardware Address", entry["hardware_address"], 0, 255);
        draw_number("FRU Device ID",    entry["fru_device_id"],    0, 255);
        draw_number("Site Number",      entry["site_number"],      0, 255);
        draw_enum  ("Site Type",        entry["site_type"],        SITE_TYPES);
    });
}

static void
draw_form_factor(App *app) {
    json &record = ensure_area_document(app, UI_AREA_FORM_FACTOR);
    draw_enum("Form Factor", record["form_factor"], FORM_FACTORS);
}

static void
draw_led_descriptor(App *app) {
    json &record = ensure_area_document(app, UI_AREA_LED_DESCRIPTOR);

    const json entry_default = {
        {"led_id",          json_unsigned(0)        },
        {"led_legend",      encoded_string_default()},
        {"led_symbol",      encoded_string_default()},
        {"led_description", encoded_string_default()},
    };

    draw_array("LED Descriptors", "LED Descriptor", record["entries"], entry_default, [&](json &entry) {
        if (!entry.is_object()) {
            entry = entry_default;
        }

        draw_number        ("LED ID", entry["led_id"], 0, 255);
        draw_encoded_string("LED Legend", entry["led_legend"]);
        draw_encoded_string("LED Symbol", entry["led_symbol"]);
        draw_encoded_string("LED Description", entry["led_description"]);
    });
}

static void
show_error(App *app, const std::string &message) {
    app->error_message = message;
    app->show_error    = true;
}

static void
reset_document(App *app) {
    app->document = json::object();
    std::fill(std::begin(app->area_enabled), std::end(app->area_enabled), false);
    app->current_area = 0;
    app->manager = Manager{};
}

static std::string
manager_error(const char *context, Errs &errors) {
    std::string detail = errors.getPlainText();
    if (detail.empty()) {
        return context;
    }
    return std::string(context) + ":\n" + detail;
}

static bool
validate_json_mrecord_types(const json &document, std::string &error) {
    if (!document.is_object() || !document.contains(AREA_TAG_MRECORDS) || !document[AREA_TAG_MRECORDS].is_array()) {
        return true;
    }

    const json &records = document[AREA_TAG_MRECORDS];
    for (size_t i = 0; i < records.size(); ++i) {
        const json &record = records[i];
        if (!record.is_object() || !record.contains(FIELD_TAG_MRECORD_ID) ||
            !record[FIELD_TAG_MRECORD_ID].is_number_integer()) {
            continue;
        }

        if (record[FIELD_TAG_MRECORD_ID].get<int>() != MRECORD_PICMG_RECORD) {
            error = "multi_records[" + std::to_string(i) + "]: редактирование этого типа PICMG MultiRecord не поддерживается";
            return false;
        }
    }
    return true;
}

static bool
validate_toml_mrecord_types(const toml::value &document, std::string &error) {
    if (!document.contains(AREA_TAG_MRECORDS)) {
        return true;
    }

    const toml::value &records = document.at(AREA_TAG_MRECORDS);
    if (!records.is_array()) {
        return true;
    }

    const auto &record_array = records.as_array();
    for (size_t i = 0; i < record_array.size(); ++i) {
        const toml::value &record = record_array[i];
        if (!record.is_table() || !record.contains(FIELD_TAG_MRECORD_ID) ||
            !record.at(FIELD_TAG_MRECORD_ID).is_integer()) {
            continue;
        }

        if (record.at(FIELD_TAG_MRECORD_ID).as_integer() != MRECORD_PICMG_RECORD) {
            error = "multi_records[" + std::to_string(i) + "]: редактирование этого типа PICMG MultiRecord не поддерживается";
            return false;
        }
    }
    return true;
}

static bool
validate_encoded_area_layout(const bytes &input, size_t area_end, size_t first_field, size_t required_fields, std::string &error) {
    const size_t checksum = area_end - 1;
    size_t       cursor   = first_field;

    auto skip_encoded_field = [&]() {
        if (cursor >= checksum) {
            return false;
        }

        const uchar type_length = static_cast<uchar>(input[cursor++]);
        const size_t length = type_length & MASK_6b;
        if (cursor + length > checksum) {
            return false;
        }

        cursor += length;
        return true;
    };

    for (size_t i = 0; i < required_fields; ++i) {
        if (!skip_encoded_field()) {
            error = "закодированная строка выходит за границы области FRU";
            return false;
        }
    }

    while (cursor < checksum && input[cursor] != END_OF_FIELDS_BYTE) {
        if (!skip_encoded_field()) {
            error = "закодированная строка выходит за границы области FRU";
            return false;
        }
    }

    if (cursor >= checksum || input[cursor] != END_OF_FIELDS_BYTE) {
        error = "в области FRU отсутствует маркер конца полей";
        return false;
    }
    return true;
}

static bool
validate_binary_layout(const bytes &input, std::string &error) {
    if (input.size() < 8) {
        error = "файл меньше 8-байтового заголовка FRU";
        return false;
    }

    struct Offset {
        size_t header_index;
        size_t byte_offset;
    };

    std::vector<Offset> offsets;
    size_t previous_offset = 0;
    for (size_t i = 1; i <= 5; ++i) {
        const size_t offset = static_cast<uchar>(input[i]) * 8U;
        if (offset == 0) {
            continue;
        }

        if (offset < 8 || offset >= input.size() ||
            (previous_offset != 0 && offset <= previous_offset)) {
            error = "заголовок FRU содержит некорректное смещение области или нарушенный порядок областей";
            return false;
        }

        offsets.push_back({i, offset});
        previous_offset = offset;
    }

    for (size_t i = 0; i < offsets.size(); ++i) {
        const size_t start = offsets[i].byte_offset;
        const size_t end   = i + 1 < offsets.size() ? offsets[i + 1].byte_offset : input.size();
        const size_t span  = end - start;

        if (offsets[i].header_index == 1) {
            if (span < 1) {
                error = "область Internal Use пуста";
                return false;
            }
            continue;
        }

        if (offsets[i].header_index < 5) {
            if (span < 8) {
                error = "стандартная область FRU меньше 8 байт";
                return false;
            }

            const size_t declared_length = static_cast<uchar>(input[start + 1]) * 8U;
            if (declared_length < 8 || declared_length > span) {
                error = "стандартная область FRU имеет некорректную длину";
                return false;
            }

            const size_t area_end        = start + declared_length;
            const size_t first_field     = start + (offsets[i].header_index == 3 ? 6U : 3U);
            const size_t required_fields = offsets[i].header_index == 2 ? 2U : offsets[i].header_index == 3 ? 4U : 7U;
            if (!validate_encoded_area_layout(input, area_end, first_field, required_fields, error)) {
                return false;
            }
            continue;
        }

        size_t record_offset     = start;
        bool   found_end_of_list = false;
        while (record_offset < end) {
            if (end - record_offset < MRECORD_HEADER_LEN_PICMG) {
                error = "MultiRecord меньше заголовка PICMG";
                return false;
            }

            if (static_cast<uchar>(input[record_offset]) != MRECORD_PICMG_RECORD) {
                error = "редактирование этого типа PICMG MultiRecord не поддерживается";
                return false;
            }

            const size_t data_length              = static_cast<uchar>(input[record_offset + 2]);
            const size_t picmg_data_header_length = MRECORD_HEADER_LEN_PICMG - MRECORD_HEADER_LEN_IPMI;
            const size_t record_length            = MRECORD_HEADER_LEN_IPMI + data_length;
            if (data_length < picmg_data_header_length || record_offset + record_length > end) {
                error = "MultiRecord имеет некорректную длину";
                return false;
            }

            found_end_of_list = (static_cast<uchar>(input[record_offset + 1]) & MRECORD_EOL_BYTE) != 0;
            record_offset    += record_length;
            if (found_end_of_list) {
                break;
            }
        }

        if (!found_end_of_list) {
            error = "в области MultiRecord отсутствует маркер конца списка";
            return false;
        }
    }
    return true;
}

static void
import_document(App *app, DialogAction action, const std::string &path) {
    try {
        app->manager = Manager{};

        Errs errors;
        json normalized      = json::object();
        bool emit_normalized = true;

        if (action == DialogAction::ImportJson) {
            json        input;
            std::string error;
            if (!app->manager.loadJSON(path, input, error)) {
                show_error(app, "Не удалось загрузить JSON: " + error);
                return;
            }

            if (!validate_json_mrecord_types(input, error)) {
                show_error(app, "Не удалось разобрать JSON:\n" + error);
                return;
            }

            if (!app->manager.parseJSON(input, errors)) {
                show_error(app, manager_error("Не удалось разобрать JSON", errors));
                return;
            }

            normalized = std::move(input);
            emit_normalized = false;
        } else if (action == DialogAction::ImportToml) {
            toml::value input;
            std::string error;
            if (!app->manager.loadTOML(path, input, error)) {
                show_error(app, "Не удалось загрузить TOML: " + error);
                return;
            }

            if (!validate_toml_mrecord_types(input, error)) {
                show_error(app, "Не удалось разобрать TOML:\n" + error);
                return;
            }

            if (!app->manager.parseTOML(input, errors)) {
                show_error(app, manager_error("Не удалось разобрать TOML", errors));
                return;
            }
        } else if (action == DialogAction::ImportBinary) {
            bytes input;
            std::string error;
            if (!app->manager.loadBinary(path, input, error)) {
                show_error(app, "Не удалось загрузить образ FRU: " + error);
                return;
            }

            if (!validate_binary_layout(input, error)) {
                show_error(app, "Не удалось разобрать образ FRU:\n" + error);
                return;
            }

            if (!app->manager.parseBinary(input, errors)) {
                show_error(app, manager_error("Не удалось разобрать образ FRU", errors));
                return;
            }
        } else {
            return;
        }

        if (emit_normalized) {
            app->manager.emitJSON(normalized);
        }
        app->document = std::move(normalized);
        sync_enabled_areas(app);
    } catch (const std::exception &error) {
        show_error(app, "Не удалось импортировать файл:\n" + std::string(error.what()));
    } catch (...) {
        show_error(app, "Не удалось импортировать файл: неизвестная ошибка");
    }
}

static void
export_document(App *app, DialogAction action, const std::string &path) {
    try {
        json input = enabled_document(app);
        app->manager = Manager{};
        Errs errors;
        if (!app->manager.parseJSON(input, errors)) {
            show_error(app, manager_error("Не удалось проверить документ", errors));
            return;
        }

        bool saved = false;
        switch (action) {
            case DialogAction::ExportJson:   saved = app->manager.saveJSON(path, errors);   break;
            case DialogAction::ExportToml:   saved = app->manager.saveTOML(path, errors);   break;
            case DialogAction::ExportBinary: saved = app->manager.saveBinary(path, errors); break;
            default: return;
        }

        if (!saved) {
            show_error(app, manager_error("Не удалось сохранить файл", errors));
        }
    } catch (const std::exception &error) {
        show_error(app, "Не удалось экспортировать файл:\n" + std::string(error.what()));
    } catch (...) {
        show_error(app, "Не удалось экспортировать файл: неизвестная ошибка");
    }
}

static bool
is_import(DialogAction action) {
    return action == DialogAction::ImportJson || action == DialogAction::ImportToml || action == DialogAction::ImportBinary;
}

static const char *
dialog_title(DialogAction action) {
    switch (action) {
        case DialogAction::ImportJson:   return "Импорт JSON";
        case DialogAction::ImportToml:   return "Импорт TOML";
        case DialogAction::ImportBinary: return "Импорт образа FRU";
        case DialogAction::ExportJson:   return "Экспорт JSON";
        case DialogAction::ExportToml:   return "Экспорт TOML";
        case DialogAction::ExportBinary: return "Экспорт образа FRU";
        default:                         return "Выбор файла";
    }
}

static const char *
dialog_filter(DialogAction action) {
    switch (action) {
        case DialogAction::ImportJson:
        case DialogAction::ExportJson:   return ".json";
        case DialogAction::ImportToml:
        case DialogAction::ExportToml:   return ".toml";
        case DialogAction::ImportBinary:
        case DialogAction::ExportBinary: return ".bin";
        default:                         return ".*";
    }
}

static const char *
dialog_default_name(DialogAction action) {
    switch (action) {
        case DialogAction::ExportJson:   return "out.json";
        case DialogAction::ExportToml:   return "out.toml";
        case DialogAction::ExportBinary: return "out.bin";
        default:                         return "";
    }
}

static void
open_file_dialog(App *app, DialogAction action) {
    constexpr const char *DIALOG_KEY = "fruit-file-dialog";

    IGFD::FileDialogConfig config;
    config.path              = ".";
    config.fileName          = dialog_default_name(action);
    config.countSelectionMax = 1;
    config.flags             = ImGuiFileDialogFlags_Modal | ImGuiFileDialogFlags_HideColumnType | ImGuiFileDialogFlags_DontShowHiddenFiles;

    if (is_import(action)) {
        config.flags |= ImGuiFileDialogFlags_ReadOnlyFileNameField;
    } else {
        config.flags |= ImGuiFileDialogFlags_ConfirmOverwrite;
    }

    app->dialog_action = action;
    ImGuiFileDialog::Instance()->OpenDialog(DIALOG_KEY, dialog_title(action), dialog_filter(action), config);
}

static void
draw_file_dialog(App *app) {
    constexpr const char *DIALOG_KEY = "fruit-file-dialog";
    if (app->dialog_action == DialogAction::None) {
        return;
    }

    ImGuiIO &io = ImGui::GetIO();
    const ImVec2 minimum(540.0F, 360.0F);
    const ImVec2 maximum(std::max(minimum.x, io.DisplaySize.x * 0.9F),
                         std::max(minimum.y, io.DisplaySize.y * 0.9F));
    if (!ImGuiFileDialog::Instance()->Display(DIALOG_KEY, ImGuiWindowFlags_NoCollapse, minimum, maximum)) {
        return;
    }

    const DialogAction action = app->dialog_action;
    if (ImGuiFileDialog::Instance()->IsOk()) {
        const std::string path = ImGuiFileDialog::Instance()->GetFilePathName();
        if (is_import(action)) {
            import_document(app, action, path);
        } else {
            export_document(app, action, path);
        }
    }
    ImGuiFileDialog::Instance()->Close();
    app->dialog_action = DialogAction::None;
}

static void
draw_menu_bar(App *app) {
    if (!ImGui::BeginMenuBar()) {
        return;
    }

    if (ImGui::BeginMenu("Файл")) {
        if (ImGui::MenuItem("Новый")) {
            app->request_new = true;
        }

        ImGui::Separator();

        if (ImGui::BeginMenu("Импорт")) {
            if (ImGui::MenuItem("JSON...")) {
                open_file_dialog(app, DialogAction::ImportJson);
            }

            if (ImGui::MenuItem("TOML...")) {
                open_file_dialog(app, DialogAction::ImportToml);
            }

            if (ImGui::MenuItem("Образ FRU...")) {
                open_file_dialog(app, DialogAction::ImportBinary);
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Экспорт")) {
            if (ImGui::MenuItem("JSON...")) {
                open_file_dialog(app, DialogAction::ExportJson);
            }

            if (ImGui::MenuItem("TOML...")) {
                open_file_dialog(app, DialogAction::ExportToml);
            }

            if (ImGui::MenuItem("Образ FRU...")) {
                open_file_dialog(app, DialogAction::ExportBinary);
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Вид")) {
        if (ImGui::BeginMenu("Тема")) {
            if (ImGui::MenuItem("Темная", nullptr, app->dark_theme)) {
                app->dark_theme = true;
                apply_theme(true);
            }

            if (ImGui::MenuItem("Светлая", nullptr, !app->dark_theme)) {
                app->dark_theme = false;
                apply_theme(false);
            }

            ImGui::EndMenu();
        }

        ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
}

static void
draw_sidebar(App *app) {
    ImGui::TextUnformatted("Области");

    ImGui::Separator();

    for (int i = 0; i < AREA_COUNT; ++i) {
        ImGui::PushID(i);
        {
            bool enabled = app->area_enabled[i];
            if (ImGui::Checkbox("##enabled", &enabled)) {
                app->area_enabled[i] = enabled;
                if (enabled) {
                    ensure_area_document(app, i);
                }
            }

            ImGui::SameLine();

            if (ImGui::Selectable(AREA_INFOS[static_cast<size_t>(i)].name, app->current_area == i, 0, ImVec2(0.0F, 0.0F))) {
                app->current_area = i;
            }
        }
        ImGui::PopID();
    }
}

static void
draw_current_area(App *app) {
    ImGui::TextUnformatted(AREA_INFOS[static_cast<size_t>(app->current_area)].name);

    ImGui::SameLine();

    ImGui::TextDisabled(app->area_enabled[app->current_area] ? "(включена)" : "(выключена)");

    ImGui::Separator();

    show_field_preview_column = area_has_field_previews(app);
    current_field_area        = app->current_area;

    ImGui::PushID(app->current_area);
    {
        switch (app->current_area) {
            case UI_AREA_INTERNAL_USE:         draw_internal_use(app);                    break;
            case UI_AREA_CHASSIS:              draw_chassis(app);                         break;
            case UI_AREA_BOARD:                draw_board(app);                           break;
            case UI_AREA_PRODUCT:              draw_product(app);                         break;
            case UI_AREA_BACKPLANE_P2P:        draw_backplane_p2p(app);                   break;
            case UI_AREA_ADDRESS_TABLE:        draw_address_table(app);                   break;
            case UI_AREA_POWER_DISTRIBUTION:   draw_power_distribution(app);              break;
            case UI_AREA_ACTIVATION_AND_POWER: draw_activation_and_power_management(app); break;
            case UI_AREA_IP_CONNECTION:        draw_ip_connection(app);                   break;
            case UI_AREA_BOARD_P2P:            draw_board_p2p(app);                       break;
            case UI_AREA_RADIAL_IPMB0:         draw_radial_ipmb0_topology(app);           break;
            case UI_AREA_FAN_GEOGRAPHY:        draw_fan_geography(app);                   break;
            case UI_AREA_FORM_FACTOR:          draw_form_factor(app);                     break;
            case UI_AREA_LED_DESCRIPTOR:       draw_led_descriptor(app);                  break;
            default: break;
        }
    }
    ImGui::PopID();

    show_field_preview_column = false;
}

static void
draw_new_popup(App *app) {
    if (app->request_new) {
        ImGui::OpenPopup("Новый документ?");
        app->request_new = false;
    }

    if (ImGui::BeginPopupModal("Новый документ?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted("Все введенные данные будут удалены. Продолжить?");

        ImGui::Spacing();

        if (ImGui::Button("Отмена", ImVec2(100.0F, 0.0F))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();
        if (ImGui::Button("ОК", ImVec2(100.0F, 0.0F))) {
            reset_document(app);
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

static void
draw_error_popup(App *app) {
    if (app->show_error) {
        ImGui::OpenPopup("Ошибка");
        app->show_error = false;
    }

    if (ImGui::BeginPopupModal("Ошибка", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        input_text_multiline("##error-text", app->error_message, ImVec2(620.0F, 220.0F), ImGuiInputTextFlags_ReadOnly);

        if (ImGui::Button("Копировать", ImVec2(120.0F, 0.0F))) {
            ImGui::SetClipboardText(app->error_message.c_str());
        }

        ImGui::SameLine();

        if (ImGui::Button("Закрыть", ImVec2(100.0F, 0.0F))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

static void
draw_ui(App *app) {
    const ImGuiViewport *viewport = ImGui::GetMainViewport();

    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration    |
                                       ImGuiWindowFlags_NoMove          |
                                       ImGuiWindowFlags_NoSavedSettings |
                                       ImGuiWindowFlags_MenuBar;

    ImGui::Begin("fruit-editor", nullptr, flags);
    {
        draw_menu_bar(app);

        ImGui::BeginChild("sidebar", ImVec2(285.0F, 0.0F), true);
        {
            draw_sidebar(app);
        }
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("area-editor", ImVec2(0.0F, 0.0F), true);
        {
            draw_current_area(app);
        }
        ImGui::EndChild();
    }
    ImGui::End();

    draw_new_popup(app);
    draw_error_popup(app);
    draw_file_dialog(app);
}

static void
glfw_error_callback(int error, const char *description) {
    std::fprintf(stderr, "Ошибка GLFW %d: %s\n", error, description);
}

int
main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        return 1;
    }

    const char *glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow *window = glfwCreateWindow(1100, 720, "fruit - редактор FRU", nullptr, nullptr);
    if (window == nullptr) {
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;

    load_application_font(io);
    apply_theme(true);

    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    if (!ImGui_ImplOpenGL3_Init(glsl_version)) {
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    App app{};

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        draw_ui(&app);

        ImGui::Render();

        int display_width = 0;
        int display_height = 0;

        glfwGetFramebufferSize(window, &display_width, &display_height);
        glViewport(0, 0, display_width, display_height);

        const ImVec4 clear_color = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
