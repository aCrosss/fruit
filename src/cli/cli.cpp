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
#if 1
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

#if 0
    json juc = json::parse(R"({"text": "Hello, 世界! Café"})");

    // Get string value
    std::string text = juc["text"].get<std::string>();

    // Encode as UTF-8 raw bytes (std::string already contains UTF-8 bytes)
    std::vector<uint8_t> raw_bytes(text.begin(), text.end());

    // Print raw bytes
    std::cout << "Raw bytes (hex): ";
    for (uint8_t byte : raw_bytes) {
        printf("%02X ", byte);
    }
    std::cout << std::endl;

    std::vector<uint8_t> back = {0x48, 0x65, 0x6C, 0x6C, 0x6F, 0x20, 0xE4, 0xB8, 0x96,
                                 0xE7, 0x95, 0x8C, 0x20, 0xF0, 0x9F, 0x8C, 0x8D, 0x21,
                                 0x20, 0x65, 0x6D, 0x6F, 0x6A, 0x69, 0x3A, 0x20, 0xF0,
                                 0x9F, 0x98, 0x80, 0xF0, 0x9F, 0x8E, 0x89};

    std::string bstr(back.begin(), back.end());
    std::cout << bstr << std::endl;

    return 0;
#endif

#if 0
    if (!manager.loadTOML(TEST_TOML_PATH, t, err)) {
        std::cout << "failed to load TOML: " << err << std::endl;
        return -1;
    }

    if (!manager.parseTOML(t, errs)) {
        std::cout << "failed to parse TOML: " << std::endl;
        std::cout << errs.getPlainText();
        return -1;
    }
#endif

    if (!manager.saveBinary(TEST_OUT_BINARY, errs)) {
        std::cout << "failed to build FRU binary image: " << std::endl;
        std::cout << errs.getPlainText();
        return -1;
    }

    return 0;
}
