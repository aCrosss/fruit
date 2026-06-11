#include <fstream>
#include <iostream>

#include "manager.hpp"

// clang-format off
#include "areas/areaChassis.hpp"
#include "areas/areaBoard.hpp"
// clang-format on

void
Manager::initSections() {
    sections.push_back(std::make_unique<AreaChassis>());
    sections.push_back(std::make_unique<AreaBoard>());
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
            std::cout << "> area " << area->getTag() << " not present";
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
        return false;
    }

    tarea = t.at(area->getTag());
    return area->tryParseTOML(tarea, errs);
}

bool
Manager::parseBinary(bytes &bs, Errs &errs) {
    // TODO: have to parse common header to get offsets for other areas
    return false;
}

bool
Manager::saveJSON(std::string path, Errs &errs) {
    return false;
}

bool
Manager::saveBinary(std::string path, Errs &errs) {
    bytes bs;
    bool  valid = true;

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
}
