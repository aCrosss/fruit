#include <fstream>
#include <iostream>
#include <string>

#include "json.hpp"
#include "scnChassis.hpp"

int
main() {
    // Test section parsing
    nlohmann::json j = R"(
       {
        "type": {
            "type": "bcdp",
            "data": "666"
        },
        "part_number": {
            "type": "bcdp",
            "data": "22.22"
        },
        "serial_number": {
            "type": "binary",
            "data": "AF 00 FA"
        },
        "custom": [
            {
            "type": "ascii6bit",
            "data": "IPMI"
            },
            {
            "type": "ascii6bit",
            "data": "BEAR!"
            }
        ]
    }
    )"_json;

    scnChassis chassis;
    Errs       jerrs;
    chassis.tryParseJSON(j, jerrs);
    std::string eplain = jerrs.getPlainText();
    std::cout << eplain;

    std::ofstream foutb("out.bin", std::ios::binary);
    bytes         outb;

    if (chassis.emitBinary(outb, jerrs)) {
        std::cout << "bin len=" << outb.size() << std::endl;

        foutb.write(reinterpret_cast<const char *>(outb.data()), outb.size());
        foutb.close();
    } else {
        std::cout << "=== errors ===" << std::endl;
        std::string eplain = jerrs.getPlainText();
        std::cout << eplain;
    }

    std::ifstream finb("out.bin", std::ios::binary | std::ios::ate);

    std::streamsize file_size = finb.tellg();
    finb.seekg(0, std::ios::beg);

    bytes in_bin(file_size);

    finb.read(reinterpret_cast<char *>(in_bin.data()), file_size);
    finb.close();

    Errs berrs;
    if (!chassis.tryParseBinary(in_bin.begin(), berrs)) {
        std::cout << "=== errors ===" << std::endl;
        std::string bplain = berrs.getPlainText();
        std::cout << bplain;
    }

    return 0;
}
