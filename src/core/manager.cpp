#include <fstream>
#include <iostream>
#include <memory>

#include "manager.hpp"

// clang-format off
#include "areas/areaCommonHeader.hpp"
#include "areas/areaInternalUse.hpp"
#include "areas/areaChassis.hpp"
#include "areas/areaBoard.hpp"
#include "areas/areaProductInfo.hpp"
#include "areas/areaMRecords.hpp"
// clang-format on

#define REGULAR_AREAS_COUNT 5

#define CLEAR_REGULAR_AREAS()                       \
    for (int i = 0; i < REGULAR_AREAS_COUNT; ++i) { \
        sections[i]->clear();                       \
    }

void
Manager::initSections() {
    sections.push_back(std::make_shared<AreaCommonHeader>());
    sections.push_back(std::make_shared<AreaInternalUse>());
    sections.push_back(std::make_shared<AreaChassis>());
    sections.push_back(std::make_shared<AreaBoard>());
    sections.push_back(std::make_shared<AreaProductInfo>());
    sections.push_back(std::make_shared<AreaMRecords>());
}

bool
Manager::loadJSON(std::string path, nlohmann::json &j, std::string &err) {
    if (!std::filesystem::exists(path)) {
        std::stringstream s;
        s << "file " << path << " doesn't exist";
        err = s.str();
        return false;
    }

    std::ifstream f(path);
    j = json::parse(f);

    return j != nlohmann::json{};
}

bool
Manager::loadTOML(std::string path, toml::value &t, std::string &err) {
    std::stringstream s;

    if (!std::filesystem::exists(path)) {
        s << "file " << path << " doesn't exist";
        err = s.str();
        return false;
    }

    auto input = toml::try_parse(path);
    if (!input.is_ok()) {
        s << "failed to parse " << path << " as TOML file";
        err = s.str();
        return false;
    }

    t = input.unwrap();
    return true;
}

bool
Manager::loadBinary(std::string path, bytes &bs, std::string &err) {
    if (!std::filesystem::exists(path)) {
        std::stringstream s;
        s << "file " << path << " doesn't exist";
        err = s.str();
        return false;
    }

    std::ifstream f(path, std::ios::binary | std::ios::ate);

    std::streamsize file_size = f.tellg();
    f.seekg(0, std::ios::beg);

    bs.resize(file_size);

    f.read(reinterpret_cast<char *>(bs.data()), file_size);
    f.close();
    return true;
}

bool
Manager::parseJSON(nlohmann::json &j, Errs &errs) {
    bool valid = true;
    CLEAR_REGULAR_AREAS();

    for (auto &&area : sections) {
        if (!j.contains(area->getTag())) {
            std::cout << "> area " << area->getTag() << " not present" << std::endl;
            continue;
        }

        nlohmann::json jarea = j[area->getTag()];
        if (!area->tryParse(jarea, errs)) {
            valid = false;
        }
    }

    return valid;
}

bool
Manager::parseTOML(toml::value &t, Errs &errs) {
    bool valid = true;
    CLEAR_REGULAR_AREAS();

    for (auto &&area : sections) {
        if (!t.contains(area->getTag())) {
            std::cout << "> area " << area->getTag() << " not present" << std::endl;
            continue;
        }

        toml::value tarea = t.at(area->getTag());
        if (!area->tryParse(tarea, errs)) {
            valid = false;
        }
    }

    return valid;
}

bool
Manager::parseBinary(bytes &bs, Errs &errs) {
    CLEAR_REGULAR_AREAS();

    auto header = std::static_pointer_cast<AreaCommonHeader>(sections[0]);

    size_t offset_to_end = bs.end() - bs.begin();

    // TODO: explain this with comments, beging you
    if (!header->tryParseBinary(bs.begin(), bs.begin() + 8, errs)) {
        return false;
    }

    // skip common header and point to next area
    for (size_t i = 1; i < sections.size(); i++) {
        auto &&section = sections[i];

        ssize_t offset = header->getOffset(Areas(i - 1)) * 8;
        // area not present
        if (offset == 0) {
            continue;
        }

        ssize_t next = header->getNextOffset(Areas(i));
        next         = next != 0 ? next * 8 : offset_to_end;

        bytes::iterator begin = bs.begin() + offset;
        bytes::iterator end   = bs.begin() + next;

        if (!section->tryParseBinary(begin, end, errs)) {
            return false;
        }
    }

    return true;
}

void
Manager::emitJSON(nlohmann::json &j) {
    for (auto &&area : sections) {
        nlohmann::json jarea;
        area->emitJSON(jarea);

        if (jarea.empty()) {
            continue;
        }

        j[area->getTag()] = jarea;
    }
}

bool
Manager::saveJSON(std::string path, Errs &errs) {
    nlohmann::json j;
    emitJSON(j);

    std::ofstream fout(path);
    if (!fout.is_open()) {
        errs.append("manager", "saveJSON", "failed to create file");
        return false;
    }

    fout << j.dump();
    fout.close();
    return true;
}

bool
Manager::saveTOML(std::string path, Errs &errs) {
    UNUSED(errs);

    toml::value  root   = toml::table{};
    toml::table &rtable = root.as_table();

    // order of areas shouldn't be changed
    for (size_t i = 0; i < sections.size() - 1; i++) {
        auto      &&area = sections[i];
        toml::table table;

        area->emitTOML(table);

        if (table.empty()) {
            continue;
        }

        rtable[area->getTag()] = toml::value(std::move(table));
    }

    // special case in TOML: can't pass toml::array between methods in current implementation
    auto &&area_mrecord = sections[5];

    toml::table tmrecord;
    area_mrecord->emitTOML(tmrecord);
    if (!tmrecord.empty()) {
        rtable[area_mrecord->getTag()] =
            toml::value(std::move(tmrecord[area_mrecord->getTag()]));
    }

    std::string   out_toml = toml::format(root);
    std::ofstream fout(path);
    if (!fout.is_open()) {
        errs.append("manager", "saveTOML", "failed to create file");
        return false;
    }

    fout << out_toml;
    fout.close();
    return true;
}

bool
Manager::saveBinary(std::string path, Errs &errs) {
    bytes bs;
    bool  valid = true;

    auto header = std::static_pointer_cast<AreaCommonHeader>(sections[0]);
    header->setOffsets(sections);

    for (auto &&area : sections) {
        bytes out_bin;

        if (!area->emitBinary(out_bin, errs)) {
            valid = false;
        } else {
            bs.insert(bs.end(), out_bin.begin(), out_bin.end());
        }
    }

    if (valid) {
        std::ofstream foutb(path, std::ios::binary);
        if (!foutb.is_open()) {
            errs.append("manager", "saveBinary", "failed to create file");
            return false;
        }
        foutb.write(reinterpret_cast<const char *>(bs.data()), bs.size());
        foutb.close();
        return true;
    }

    return false;
}

Manager::Manager() {
    initSections();
}
