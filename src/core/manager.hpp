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
    // TODO: TOML
    bool loadBinary(std::string path, bytes &bs, std::string &err);

    bool parseJSON(nlohmann::json &j, Errs &errs);
    //
    // TODO: TOML
    bool parseBinary(bytes &bs, Errs &errs);

    bool saveJSON(std::string patch, Errs &errs);
    // bool saveTOML(std::string patch, Errs &errs);
    bool saveBinary(std::string patch, Errs &errs);

    Manager(/* args */);
    ~Manager();
};
