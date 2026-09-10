#include "mRecordFormFactor.hpp"

//    ##        #######   ######     ###    ##
//    ##       ##     ## ##    ##   ## ##   ##
//    ##       ##     ## ##        ##   ##  ##
//    ##       ##     ## ##       ##     ## ##
//    ##       ##     ## ##       ######### ##
//    ##       ##     ## ##    ## ##     ## ##
//    ########  #######   ######  ##     ## ########

void
MRecordFormFactor::debug_printOutVals() {
    std::cout << "=== " << label << " ===" << std::endl;
    std::cout << "record_id:       " << record_id << std::endl;
    std::cout << "picmg_record_id: " << picmg_record_id << std::endl;

    std::cout << "form_factor:     " << FormFactorToStr(form_factor) << std::endl;
}

void
MRecordFormFactor::clear() {
    form_factor = FFACTOR_UNDEFINED;
}

uchar
MRecordFormFactor::getLength() {
    uchar len  = PICMG_HEADER_LEN;
    len       += 1; // form factor byte

    return len;
}

inline bool
MRecordFormFactor::isFormFactorValid(uchar ff) {
    return ff < static_cast<uchar>(FFACTOR_UNDEFINED);
}

inline std::string
MRecordFormFactor::FormFactorToStr(FormFactor ff) {
    switch (ff) {
    case FFACTOR_ATCA    : return "ATCA";
    case FFACTOR_ATCA300 : return "ATCA300";
    case FFACTOR_MicroTCA: return "MicroTCA";

    default: return "undefined";
    }

    return "undefined";
}

inline MRecordFormFactor::FormFactor
MRecordFormFactor::StrToFormFactor(std::string s) {
    if (s.compare("ATCA") == 0) {
        return FFACTOR_ATCA;
    } else if (s.compare("ATCA300") == 0) {
        return FFACTOR_ATCA300;
    } else if (s.compare("MicroTCA") == 0) {
        return FFACTOR_MicroTCA;
    }

    return FFACTOR_UNDEFINED;
}

//    ########     ###    ########   ######  #### ##    ##  ######
//    ##     ##   ## ##   ##     ## ##    ##  ##  ###   ## ##    ##
//    ##     ##  ##   ##  ##     ## ##        ##  ####  ## ##
//    ########  ##     ## ########   ######   ##  ## ## ## ##   ####
//    ##        ######### ##   ##         ##  ##  ##  #### ##    ##
//    ##        ##     ## ##    ##  ##    ##  ##  ##   ### ##    ##
//    ##        ##     ## ##     ##  ######  #### ##    ##  ######

template <typename T>
bool
MRecordFormFactor::tryParseImpl(T v, Errs &errs) {
    std::string s;

    if (!tryParseField_str(v, "form_factor", s, errs)) {
        return false;
    }

    form_factor = StrToFormFactor(s);
    if (form_factor == FFACTOR_UNDEFINED) {
        std::stringstream ss;
        ss << "value \"" << s << "\" is invalid";
        errs.append(tag, "form_factor", ss.str());
        return false;
    }

    debug_printOutVals();
    return true;
}

bool
MRecordFormFactor::tryParse(nlohmann::json j, Errs &errs) {
    return tryParseImpl(j, errs);
}

bool
MRecordFormFactor::tryParse(toml::value &t, Errs &errs) {
    return tryParseImpl(t, errs);
}

bool
MRecordFormFactor::tryParseBinary(ibytes begin, ibytes end, Errs &errs) {
    OUT_OF_BOUNDS_GUARD_OFFSET("common", PICMG_HEADER_LEN + 1)

    begin += PICMG_HEADER_LEN;

    uchar ff = DR_BYTE(begin);
    if (!isFormFactorValid(ff)) {
        std::stringstream ss;
        ss << "form factor \"" << static_cast<int>(ff) << "\" is invalid";
        errs.append(tag, "form_factor", ss.str());
        return false;
    }

    form_factor = FormFactor(ff);
    debug_printOutVals();
    return true;
}

//    ######## ##     ## #### ######## #### ##    ##  ######
//    ##       ###   ###  ##     ##     ##  ###   ## ##    ##
//    ##       #### ####  ##     ##     ##  ####  ## ##
//    ######   ## ### ##  ##     ##     ##  ## ## ## ##   ####
//    ##       ##     ##  ##     ##     ##  ##  #### ##    ##
//    ##       ##     ##  ##     ##     ##  ##   ### ##    ##
//    ######## ##     ## ####    ##    #### ##    ##  ######

void
MRecordFormFactor::emitJSON(nlohmann::json &j) {
    j["record_id"]       = record_id;
    j["picmg_record_id"] = picmg_record_id;

    std::string s = FormFactorToStr(form_factor);

    j["form_factor"] = s;
}

void
MRecordFormFactor::emitTOML(toml::table &t) {
    t["record_id"]       = toml::value(SC_I(record_id));
    t["picmg_record_id"] = toml::value(SC_I(picmg_record_id));

    std::string s = FormFactorToStr(form_factor);

    t["form_factor"] = toml::value(s);
}

bool
MRecordFormFactor::emitBinary(bytes &out_bin, bool eol, Errs &errs) {
    UNUSED(errs);

    bytes header;
    bytes payload;

    prependPICMGHeader(payload);

    payload.emplace_back(std::byte{static_cast<uchar>(form_factor)});

    buildMRecordHeader(header, eol, payload);

    APPEND_BYTES(out_bin, header);
    APPEND_BYTES(out_bin, payload);

    return true;
}

//    #### ##    ## #### ########
//     ##  ###   ##  ##     ##
//     ##  ####  ##  ##     ##
//     ##  ## ## ##  ##     ##
//     ##  ##  ####  ##     ##
//     ##  ##   ###  ##     ##
//    #### ##    ## ####    ##

MRecordFormFactor::MRecordFormFactor() : MRecordBase("form_factor", "Form Factor Information") {
    record_id       = MRECORD_PICMG_RECORD;
    picmg_record_id = PICMGREC_FORM_FACTOR;
}
