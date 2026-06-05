#include <fstream>
#include <iostream>
#include <string>

#include "json.hpp"
#include "manager.hpp"

#define TEST_JSON_PATH  "test.json"
#define TEST_TOML_PATH  "test.toml"
#define TEST_OUT_BINARY "out.bin"

int
main() {
    Manager        manager{};
    nlohmann::json j;
    toml::value    t;
    Errs           errs;

    std::string err;
#if 0
    if (!manager.loadJSON(TEST_JSON_PATH, j, err)) {
        std::cout << "failed to load JSON: " << err << std::endl;
        return -1;
    }

    if (!manager.parseJSON(j, errs)) {
        std::cout << "failed to parse JSON: " << std::endl;
        std::cout << errs.getPlainText();
        return -1;
    }
#endif

    if (!manager.loadTOML(TEST_TOML_PATH, t, err)) {
        std::cout << "failed to load TOML: " << err << std::endl;
        return -1;
    }

    if (!manager.parseTOML(t, errs)) {
        std::cout << "failed to parse TOML: " << std::endl;
        std::cout << errs.getPlainText();
        return -1;
    }

    if (!manager.saveBinary(TEST_OUT_BINARY, errs)) {
        std::cout << "failed to build FRU binary image: " << std::endl;
        std::cout << errs.getPlainText();
        return -1;
    }

    return 0;
}
