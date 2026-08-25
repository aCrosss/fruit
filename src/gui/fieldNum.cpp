#include "fieldNum.hpp"
#include "json.hpp"

void
FieldNum::clear() {
    return;
}

void
FieldNum::get(nlohmann::json &j) {
    if (is_integer) {
        if (is_signed) {
            long n = static_cast<long>(entry.get_value_as_int());
            j[tag] = nlohmann::json::number_integer_t{n};
        } else {
            ulong n = static_cast<ulong>(entry.get_value_as_int());
            j[tag]  = nlohmann::json::number_unsigned_t{n};
        }
    } else {
        double n = entry.get_value();
        j[tag]   = nlohmann::json::number_float_t{n};
    }
}

void
FieldNum::set(nlohmann::json &j) {
    if (!j.contains(tag)) {
        return;
    }

    entry.set_value(j[tag].get<double>());
}

bool
FieldNum::validate() {
    return true;
}

FieldNum::FieldNum(std::string tag, std::string label, FieldNumProps props)
    : FieldBase(tag, label) {
    //
    auto adj = entry.get_adjustment();
    adj->configure(
        props.initial, props.min, props.max, props.step_increment, props.page_increment, 0);
    entry.set_digits(props.digits);

    is_integer = (props.step_increment == 1.0);
    is_signed  = (props.min < 0);

    container.add(entry);
}
