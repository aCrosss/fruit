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
            "data": "typeabc"
        },
        "part_number": {
            "type": "bcdp",
            "data": "part_numberabc"
        },
        "serial_number": {
            "type": "bcdp",
            "data": "serial_numberabc"
        },
        "custom": [
            {
            "type": "bcdp",
            "data": "custom1"
            },
            {
            "type": "bcdp",
            "data": "custom2"
            }
        ]
    }
    )"_json;

    scnChassis chassis;
    FRU_errs   jerrs;
    chassis.tryParseJSON(j, jerrs);
    std::string eplain = jerrs.getPlainText();
    std::cout << eplain;

    return 0;
}
