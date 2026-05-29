#pragma once

#include <vector>

#include "fru_errs.hpp"
#include "section.hpp"
#include "types.hpp"

class Context {
  private:
    std::vector<Section> sections;

    void initSections();

  public:
    Context(/* args */);
    ~Context();
};

Context::Context(/* args */) {
}

Context::~Context() {
}
