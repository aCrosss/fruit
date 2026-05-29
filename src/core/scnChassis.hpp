#pragma once

#include "section.hpp"

class scnChassis : public Section {
  private:
    /* data */
  public:
    bool   validate();
    void   emmitBinary(bytes &out_bin);
    size_t getByteLen();

    scnChassis(/* args */);
    ~scnChassis();
};

scnChassis::scnChassis(/* args */) : Section("chassis", "Chassis Info Area") {
}

scnChassis::~scnChassis() {
}
