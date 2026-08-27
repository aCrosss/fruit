#include "mRecordIPConnection.hpp"

//    ##        #######   ######     ###    ##
//    ##       ##     ## ##    ##   ## ##   ##
//    ##       ##     ## ##        ##   ##  ##
//    ##       ##     ## ##       ##     ## ##
//    ##       ##     ## ##       ######### ##
//    ##       ##     ## ##    ## ##     ## ##
//    ########  #######   ######  ##     ## ########

#define APPEND_IP_BYTES(to, from)   \
    to.emplace_back(from.bytes[0]); \
    to.emplace_back(from.bytes[1]); \
    to.emplace_back(from.bytes[2]); \
    to.emplace_back(from.bytes[3]);

void
MRecordIPConnection::debug_printOutVals() {
    std::cout << "=== " << label << " ===" << std::endl;
    std::cout << "record_id:       " << record_id << std::endl;
    std::cout << "picmg_record_id: " << picmg_record_id << std::endl;

    std::string ip_address_s;
    std::string gateway_address_s;
    std::string subnet_mask_s;

    IPtoStr(ip_address, ip_address_s);
    IPtoStr(gateway_address, gateway_address_s);
    IPtoStr(subnet_mask, subnet_mask_s);

    std::cout << "ip_address:      " << ip_address_s << std::endl;
    std::cout << "gateway_address: " << gateway_address_s << std::endl;
    std::cout << "subnet_mask:     " << subnet_mask_s << std::endl;
}

void
MRecordIPConnection::clear() {
}

uchar
MRecordIPConnection::getLength() {
    // picmg heder + 3 ip addresses 4 bytes each
    return PICMG_HEADER_LEN + 3 * 4;
}

void
MRecordIPConnection::bytesToIP(biterator begin, IP &ip) {
    ip.raw[0] = DR_BYTE(begin + 0);
    ip.raw[1] = DR_BYTE(begin + 1);
    ip.raw[2] = DR_BYTE(begin + 2);
    ip.raw[3] = DR_BYTE(begin + 3);
}

bool
MRecordIPConnection::strToIP(std::string s, IP &ip, std::string &err) {
    char dot;
    int  bs[4];

    std::istringstream iss(s);
    if (iss >> bs[0] >> dot >> bs[1] >> dot >> bs[2] >> dot >> bs[3]) {
        for (size_t i = 0; i < 4; i++) {
            if (bs[i] < 0 || bs[i] > 255) {
                err = "ip must be in [0-255].[0-255].[0-255].[0-255] format";
                return false;
            }

            ip.raw[i] = static_cast<uchar>(bs[i]);
        }
    } else {
        err = "ip must be in [0-255].[0-255].[0-255].[0-255] format";
        return false;
    }

    return true;
}

void
MRecordIPConnection::IPtoStr(IP ip, std::string &out) {
    std::stringstream s;
    s << static_cast<int>(ip.raw[0]) << ".";
    s << static_cast<int>(ip.raw[1]) << ".";
    s << static_cast<int>(ip.raw[2]) << ".";
    s << static_cast<int>(ip.raw[3]);
    out = s.str();
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
MRecordIPConnection::tryParseImpl(T v, Errs &errs) {
    std::string err;
    bool        valid = true;

    std::string ip_address_s;
    std::string gateway_address_s;
    std::string subnet_mask_s;

    if (!tryParseField_str(v, "ip_address", ip_address_s, errs)) {
        valid = false;
    } else {
        if (!strToIP(ip_address_s, ip_address, err)) {
            errs.append(tag, "ip_address", err);
            valid = false;
        }
    }

    if (!tryParseField_str(v, "gateway_address", gateway_address_s, errs)) {
        valid = false;
    } else {
        if (!strToIP(gateway_address_s, gateway_address, err)) {
            errs.append(tag, "gateway_address", err);
            valid = false;
        }
    }

    if (!tryParseField_str(v, "subnet_mask", subnet_mask_s, errs)) {
        valid = false;
    } else {
        if (!strToIP(subnet_mask_s, subnet_mask, err)) {
            errs.append(tag, "subnet_mask", err);
            valid = false;
        }
    }

    debug_printOutVals();
    return valid;
}

bool
MRecordIPConnection::tryParse(nlohmann::json j, Errs &errs) {
    clear();

    return tryParseImpl(j, errs);
}

bool
MRecordIPConnection::tryParse(toml::value &t, Errs &errs) {
    clear();

    return tryParseImpl(t, errs);
}

bool
MRecordIPConnection::tryParseBinary(biterator begin, biterator end, Errs &errs) {
    if (begin + PICMG_HEADER_LEN + 3 * 4 > end) {
        errs.append(tag, "common", "out of bounds");
        return false;
    }

    clear();
    begin += PICMG_HEADER_LEN;

    bytesToIP(begin + 0, ip_address);
    bytesToIP(begin + 4, gateway_address);
    bytesToIP(begin + 8, subnet_mask);

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
MRecordIPConnection::emitJSON(nlohmann::json &j) {
    j["record_id"]       = record_id;
    j["picmg_record_id"] = picmg_record_id;

    std::string ip_address_s;
    std::string gateway_address_s;
    std::string subnet_mask_s;

    IPtoStr(ip_address, ip_address_s);
    IPtoStr(gateway_address, gateway_address_s);
    IPtoStr(subnet_mask, subnet_mask_s);

    j["ip_address"]      = ip_address_s;
    j["gateway_address"] = gateway_address_s;
    j["subnet_mask"]     = subnet_mask_s;
}

void
MRecordIPConnection::emitTOML(toml::table &t) {
    t["record_id"]       = toml::value(SC_I(record_id));
    t["picmg_record_id"] = toml::value(SC_I(picmg_record_id));

    std::string ip_address_s;
    std::string gateway_address_s;
    std::string subnet_mask_s;

    IPtoStr(ip_address, ip_address_s);
    IPtoStr(gateway_address, gateway_address_s);
    IPtoStr(subnet_mask, subnet_mask_s);

    t["ip_address"]      = toml::value(ip_address_s);
    t["gateway_address"] = toml::value(gateway_address_s);
    t["subnet_mask"]     = toml::value(subnet_mask_s);
}

bool
MRecordIPConnection::emitBinary(bytes &out_bin, Errs &errs) {
    UNUSED(errs);

    bytes header;
    bytes payload;

    prependPICMGHeader(payload);

    APPEND_IP_BYTES(payload, ip_address);
    APPEND_IP_BYTES(payload, gateway_address);
    APPEND_IP_BYTES(payload, subnet_mask);

    buildMRecordHeader(header, false, payload);

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

MRecordIPConnection::MRecordIPConnection() : MRecordBase("ip_connection", "IP Connection") {
    record_id       = MRECORD_PICMG_RECORD;
    picmg_record_id = PICMGREC_IP_CONNECTION;
}
