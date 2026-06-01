#include <sstream>

#include "fru_errs.hpp"

void
FRU_errs::append(std::string section, std::string field, std::string text) {
    errs.emplace_back(section, field, text);
}

errors
FRU_errs::getSectionErrs(std::string section) {
    errors sec_errs;
    for (size_t i = 0; i < errs.size(); i++) {
        auto e = errs.at(i);
        if (std::get<0>(e) == section) {
            sec_errs.emplace_back(e);
        }
    }

    return errors();
}

std::string
FRU_errs::getPlainText() {
    std::stringstream s;

    for (auto &&e : errs) {
        s << std::get<0>(e) << ":" << std::get<1>(e) << " " << std::get<2>(e) << std::endl;
    }

    return s.str();
}

FRU_errs::FRU_errs(/* args */) {
}

FRU_errs::~FRU_errs() {
}
