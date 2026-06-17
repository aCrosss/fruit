#include <fstream>
#include <iostream>
#include <string>

#include "CLI11.hpp"
#include "json.hpp"
#include "manager.hpp"

enum FileType {
    FTYPE_NONE,
    FTYPE_JSON,
    FTYPE_TOML,
    FTYPE_BINARY,
};

Manager manager{};

FileType
getFTypeFromPath(std::string path) {
    if (path.find(".json", 0) != path.npos) {
        return FTYPE_JSON;
    }

    if (path.find(".toml", 0) != path.npos) {
        return FTYPE_TOML;
    }

    if (path.find(".bin", 0) != path.npos) {
        return FTYPE_BINARY;
    }

    return FTYPE_NONE;
}

bool
cliInputFile(std::string path, FileType type) {
    Errs        errs;
    std::string err;

    nlohmann::json j;
    toml::value    t;
    bytes          bs;

    switch (type) {
    case FTYPE_JSON:
        if (!manager.loadJSON(path, j, err)) {
            std::cout << "failed to load JSON: " << err << std::endl;
            return false;
        }
        if (!manager.parseJSON(j, errs)) {
            std::cout << "failed to parse JSON: " << std::endl;
            std::cout << errs.getPlainText() << std::endl;
            return false;
        }
        return true;

    case FTYPE_TOML:
        if (!manager.loadTOML(path, t, err)) {
            std::cout << "failed to load TOML: " << err << std::endl;
            return false;
        }
        if (!manager.parseTOML(t, errs)) {
            std::cout << "failed to parse TOML: " << std::endl;
            std::cout << errs.getPlainText() << std::endl;
            return false;
        }
        return true;

    case FTYPE_BINARY:
        if (!manager.loadBinary(path, bs, err)) {
            std::cout << "failed to load binary: " << err << std::endl;
            return false;
        }
        if (!manager.parseBinary(bs, errs)) {
            std::cout << "failed to parse binary: " << std::endl;
            std::cout << errs.getPlainText() << std::endl;
            return false;
        }
        return true;

    default: std::cout << "unknown input file type" << std::endl; break;
    }

    return false;
}

bool
cliOutputFile(std::string path, FileType type) {
    Errs errs;

    switch (type) {
    case FTYPE_JSON:
        if (!manager.saveJSON(path, errs)) {
            std::cout << "failed to save json: " << std::endl;
            std::cout << errs.getPlainText() << std::endl;
            return false;
        }
        return true;

    case FTYPE_TOML: /* TODO: forgot to implement toml */ break;

    case FTYPE_BINARY:
        if (!manager.saveBinary(path, errs)) {
            std::cout << "failed to save binary: " << std::endl;
            std::cout << errs.getPlainText() << std::endl;
            return false;
        }
        return true;

    default: std::cout << "unknown output file type" << std::endl; break;
    }

    return false;
}

int
main(int argc, char **argv) {

    toml::value t;
    Errs        errs;

    std::string in_path;
    std::string out_path = "out.bin";

    CLI::App fruitCLI{"CLI interface for FRU Image generaTor"};
    fruitCLI.add_option("-i,--input", in_path, "Input JSON/TOML/binary file")->required();
    fruitCLI.add_option("-o,--output", out_path, "Output JSON/TOML/binary file");

    CLI11_PARSE(fruitCLI, argc, argv);

    FileType in_type = getFTypeFromPath(in_path);
    if (in_type == FTYPE_NONE) {
        std::cout << "Error: invalid typef of input file: " << in_path << std::endl;
        return 1;
    }

    FileType out_type = getFTypeFromPath(out_path);
    if (out_type == FTYPE_NONE) {
        std::cout << "Error: invalid typef of output file: " << out_path << std::endl;
        return 1;
    }

    if (!cliInputFile(in_path, in_type)) {
        return 1;
    }

    if (!cliOutputFile(out_path, out_type)) {
        return 1;
    }

    return 0;
}
