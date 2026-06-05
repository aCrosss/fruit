#include <fstream>
#include <iostream>

#include "manager.hpp"

#include "areaChassis.hpp"

void
Manager::initSections() {
    sections.push_back(std::make_unique<AreaChassis>());
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
    auto          &area = sections[0];
    nlohmann::json jarea;

    if (!j.contains(area->getTag())) {
        return false;
    }

    jarea = j[area->getTag()];
    return area->tryParseJSON(jarea, errs);
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

    auto &area = sections[0];
    bytes out_bin;

    if (!area->emitBinary(out_bin, errs)) {
        // TODO: redo to actualu use common header offsets
        valid = false;
    } else {
        bs.insert(bs.end(), out_bin.begin(), out_bin.end());
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
