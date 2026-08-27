#pragma once

#include <string>
#include <tuple>
#include <vector>

typedef std::tuple<std::string, std::string, std::string> error;
typedef std::vector<error>                                errors;

class Errs {
  private:
    errors errs;

  public:
    void        append(std::string section, std::string field, std::string text);
    errors      getSectionErrs(std::string section);
    std::string getPlainText();
};
