#pragma once

#include <vector>

#include "errs.hpp"
#include "section.hpp"
#include "types.hpp"

class Manager {
  private:
    std::vector<std::unique_ptr<Section>> sections;

    void initSections();

  public:
    bool loadJSON(std::string path, nlohmann::json &j, std::string &err);
    bool loadTOML(std::string path, toml::value &t, std::string &err);
    bool loadBinary(std::string path, bytes &bs, std::string &err);

    bool parseJSON(nlohmann::json &j, Errs &errs);
    bool parseTOML(toml::value &t, Errs &errs);
    bool parseBinary(bytes &bs, Errs &errs);

    void emitJSON(nlohmann::json &j);
    bool saveJSON(std::string patch, Errs &errs);
    bool saveTOML(std::string patch, Errs &errs);
    bool saveBinary(std::string patch, Errs &errs);

    Manager(/* args */);
    ~Manager();
};
