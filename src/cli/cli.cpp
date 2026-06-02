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
            "type": "ascii6bit",
            "data": "IPMI"
        },
        "custom": [
            {
            "type": "ascii6bit",
            "data": "IPMI"
            },
            {
            "type": "ascii6bit",
            "data": "BEAR"
            }
        ]
    }
    )"_json;

    scnChassis chassis;
    FRU_errs   jerrs;
    chassis.tryParseJSON(j, jerrs);
    std::string eplain = jerrs.getPlainText();
    std::cout << eplain;

    std::ofstream foutb("out.bin", std::ios::binary);
    bytes         outb;

    if (chassis.emmitBinary(outb, jerrs)) {
        std::cout << "bin len=" << outb.size() << std::endl;

        foutb.write(reinterpret_cast<const char *>(outb.data()), outb.size());
        foutb.close();
    } else {
        std::cout << "=== errors ===" << std::endl;
        std::string eplain = jerrs.getPlainText();
        std::cout << eplain;
    }

    return 0;
}
