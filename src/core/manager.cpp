#include <fstream>
#include <iostream>

#include "manager.hpp"

// clang-format off
#include "areas/areaCommonHeader.hpp"
#include "areas/areaInternalUse.hpp"
#include "areas/areaChassis.hpp"
#include "areas/areaBoard.hpp"
#include "areas/areaProductInfo.hpp"
// clang-format on

void
Manager::initSections() {
    sections.push_back(std::make_unique<AreaCommonHeader>());
    sections.push_back(std::make_unique<AreaInternalUse>());
    sections.push_back(std::make_unique<AreaChassis>());
    sections.push_back(std::make_unique<AreaBoard>());
    sections.push_back(std::make_unique<AreaProductInfo>());
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
    // TODO: add proper sections parsing loop
    // auto          &area = sections[0];
    bool valid = true;

    for (auto &&area : sections) {
        if (!j.contains(area->getTag())) {
            std::cout << "> area " << area->getTag() << " not present" << std::endl;
            continue;
        }

        nlohmann::json jarea = j[area->getTag()];
        if (!area->tryParseJSON(jarea, errs)) {
            valid = false;
        }
    }

    return valid;
}

bool
Manager::parseTOML(toml::value &t, Errs &errs) {
    auto       &area = sections[0];
    toml::value tarea;

    if (!t.contains(area->getTag())) {
        std::cout << "> area " << area->getTag() << " not present" << std::endl;
        return false;
    }

    tarea = t.at(area->getTag());
    return area->tryParseTOML(tarea, errs);
}

bool
Manager::parseBinary(bytes &bs, Errs &errs) {
    auto *header = dynamic_cast<AreaCommonHeader *>(sections[0].get());

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

bool
Manager::saveJSON(std::string path, Errs &errs) {
    nlohmann::json j;

    for (auto &&area : sections) {
        nlohmann::json jarea;
        area->emitJSON(jarea);

        if (jarea.empty()) {
            continue;
        }

        j[area->getTag()] = jarea;
    }

    std::ofstream fout(path);
    if (!fout.is_open()) {
        return false;
    }

    fout << j.dump();
    fout.close();
    return true;
}

bool
Manager::saveTOML(std::string path, Errs &errs) {
    toml::value  root   = toml::table{};
    toml::table &rtable = root.as_table();

    for (auto &&area : sections) {
        toml::table table;
        area->emitTOML(table);

        if (table.empty()) {
            continue;
        }

        rtable[area->getTag()] = toml::value(std::move(table));
    }

    std::string out_toml = toml::format(root);

    std::ofstream fout(path);
    if (!fout.is_open()) {
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

    auto *header = dynamic_cast<AreaCommonHeader *>(sections[0].get());
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
        foutb.write(reinterpret_cast<const char *>(bs.data()), bs.size());
        foutb.close();
        return true;
    }

    return false;
}

Manager::Manager(/* args */) {
    initSections();
}

Manager::~Manager() {
    //
}
