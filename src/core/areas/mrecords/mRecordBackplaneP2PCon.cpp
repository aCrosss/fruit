#include "mRecordBackplaneP2PCon.hpp"
#include "section.hpp"
#include "types.hpp"

//    ##        #######   ######     ###    ##
//    ##       ##     ## ##    ##   ## ##   ##
//    ##       ##     ## ##        ##   ##  ##
//    ##       ##     ## ##       ##     ## ##
//    ##       ##     ## ##       ######### ##
//    ##       ##     ## ##    ## ##     ## ##
//    ########  #######   ######  ##     ## ########

void
MRecordBackplaneP2PCon::debug_printOutVals() {
    std::cout << "=== " << label << " ===" << std::endl;
    std::cout << "record_id:       " << record_id << std::endl;
    std::cout << "picmg_record_id: " << picmg_record_id << std::endl;

    for (size_t i = 0; i < slots.size(); i++) {
        std::cout << "slot[" << i << "]:" << std::endl;
        std::cout << "  type   : " << SC_I(slots[i].type) << std::endl;
        std::cout << "  address: " << SC_I(slots[i].address) << std::endl;

        for (size_t j = 0; j < slots[i].channels.size(); j++) {
            ChannelDescr &cd = slots[i].channels[j];
            std::cout << "  channel[" << j << "]:" << std::endl;
            std::cout << "    local_channel: " << SC_I(cd.local_channel) << std::endl;
            std::cout << "    remote_channel: " << SC_I(cd.remote_channel) << std::endl;
            std::cout << "    remote_slot: " << SC_I(cd.remote_slot) << std::endl;
        }
    }
}

void
MRecordBackplaneP2PCon::clear() {
    slots.clear();
}

uchar
MRecordBackplaneP2PCon::getLength() {
    uchar len = PICMG_HEADER_LEN;

    for (size_t i = 0; i < slots.size(); i++) {
        len += 1 /*type*/ + 1 /*address*/ + 1 /*count*/ + slots[i].channels.size() * 3;
    }

    return len;
}

//
//   ####  #    #   ##   #    # #    # ###### #         #####  ######  ####   ####  #####
//  #    # #    #  #  #  ##   # ##   # #      #         #    # #      #      #    # #    #
//  #      ###### #    # # #  # # #  # #####  #         #    # #####   ####  #      #    #
//  #      #    # ###### #  # # #  # # #      #         #    # #           # #      #####
//  #    # #    # #    # #   ## #   ## #      #         #    # #      #    # #    # #   #
//   ####  #    # #    # #    # #    # ###### ######    #####  ######  ####   ####  #    #
//

template <typename T>
bool
MRecordBackplaneP2PCon::tryParseChannelDescrImpl(T v, ChannelDescr &cd, Errs &errs) {
    bool valid = true;

    uchar local_channel;
    uchar remote_channel;
    uchar remote_slot;

    if (!tryParseField_uchar(v, "local_channel", local_channel, errs)) {
        valid = false;
    }

    if (!tryParseField_uchar(v, "remote_channel", remote_channel, errs)) {
        valid = false;
    }

    if (!tryParseField_uchar(v, "remote_slot", remote_slot, errs)) {
        valid = false;
    }

    if (!valid) {
        return false;
    }

    cd.local_channel  = local_channel;
    cd.remote_channel = remote_channel;
    cd.remote_slot    = remote_slot;

    return true;
}

bool
MRecordBackplaneP2PCon::tryParseChannelDescr(biterator    &begin,
                                             biterator     end,
                                             ChannelDescr &cd,
                                             Errs         &errs) {
    OUT_OF_BOUNDS_GUARD_OFFSET("slots", CHANNEL_BYTE_LEN)

    uchar b1 = DR_BYTE(begin + 0);
    uchar b2 = DR_BYTE(begin + 1);
    uchar b3 = DR_BYTE(begin + 2);

    // 17:13
    cd.local_channel  = ((b2 >> 5) & MASK_3b) | ((b3 & MASK_2b) << 3);
    // 12:8
    cd.remote_channel = b2 & MASK_5b;
    //  7:0
    cd.remote_slot    = b1;

    // always 3 bytes, move iterator accordingly
    begin += CHANNEL_BYTE_LEN;
    return true;
}

void
MRecordBackplaneP2PCon::emitChannelDescr(nlohmann::json &j, ChannelDescr &cd) {
    j["local_channel"]  = cd.local_channel;
    j["remote_channel"] = cd.remote_channel;
    j["remote_slot"]    = cd.remote_slot;
}

void
MRecordBackplaneP2PCon::emitChannelDescr(toml::table &t, ChannelDescr &cd) {
    t["local_channel"]  = toml::value(cd.local_channel);
    t["remote_channel"] = toml::value(cd.remote_channel);
    t["remote_slot"]    = toml::value(cd.remote_slot);
}

void
MRecordBackplaneP2PCon::emitChannelDescr(bytes &out_bin, ChannelDescr &cd) {
    uchar b1 = 0;
    uchar b2 = 0;
    uchar b3 = 0;

    b1 = cd.remote_slot;
    b2 = (cd.remote_channel & MASK_5b) | ((cd.local_channel & MASK_3b) << 5);
    b3 = (cd.local_channel >> 3) & MASK_2b;

    out_bin.emplace_back(std::byte{b1});
    out_bin.emplace_back(std::byte{b2});
    out_bin.emplace_back(std::byte{b3});
}

//
//      ####  #       ####  #####    #####  ######  ####   ####  #####
//     #      #      #    #   #      #    # #      #      #    # #    #
//      ####  #      #    #   #      #    # #####   ####  #      #    #
//          # #      #    #   #      #    # #           # #      #####
//     #    # #      #    #   #      #    # #      #    # #    # #   #
//      ####  ######  ####    #      #####  ######  ####   ####  #    #
//

template <typename T>
bool
MRecordBackplaneP2PCon::tryParseSlotDescrImpl(T v, SlotDescriptor &sd, Errs &errs) {
    bool valid = true;

    uchar type;
    uchar address;

    if (!tryParseField_uchar(v, "type", type, errs)) {
        valid = false;
    }

    if (!tryParseField_uchar(v, "address", address, errs)) {
        valid = false;
    }

    sd.type    = type;
    sd.address = address;

    T array;
    if (!tryParseField_arr(v, "channels", array, errs)) {
        return false;
    }

    for (size_t i = 0; i < array.size(); i++) {
        T            entry = array[i];
        ChannelDescr cd;

        std::stringstream s;
        s << "channels[" << i << "]";
        if (!tryParseChannelDescrImpl(entry, cd, errs)) {
            valid = false;
            continue;
        }

        sd.channels.push_back(cd);
    }

    return valid;
}

bool
MRecordBackplaneP2PCon::tryParseSlotDescr(biterator      &begin,
                                          biterator       end,
                                          SlotDescriptor &sd,
                                          Errs           &errs) {
    OUT_OF_BOUNDS_GUARD_OFFSET("slots", SLOT_BASE_BYTE_LEN)

    sd.type    = DR_BYTE(begin + 0);
    sd.address = DR_BYTE(begin + 1);

    uchar channel_count  = DR_BYTE(begin + 2);
    begin               += SLOT_BASE_BYTE_LEN;
    for (uchar i = 0; i < channel_count; i++) {
        ChannelDescr cd;
        if (!tryParseChannelDescr(begin, end, cd, errs)) {
            return false;
        }

        sd.channels.push_back(cd);
    }

    return true;
}

void
MRecordBackplaneP2PCon::emitSlotDescr(nlohmann::json &j, SlotDescriptor &sd) {
    j["type"]    = sd.type;
    j["address"] = sd.address;

    if (sd.channels.size() == 0) {
        return;
    }

    json jarray;
    for (size_t i = 0; i < sd.channels.size(); i++) {
        json jentry;
        emitChannelDescr(jentry, sd.channels[i]);
        jarray[i] = jentry;
    }

    j["channels"] = jarray;
}

void
MRecordBackplaneP2PCon::emitSlotDescr(toml::table &t, SlotDescriptor &sd) {
    t["type"]    = toml::value(sd.type);
    t["address"] = toml::value(sd.address);

    if (sd.channels.size() == 0) {
        return;
    }

    toml::array tarray;
    for (size_t i = 0; i < sd.channels.size(); i++) {
        toml::table tentry;
        emitChannelDescr(tentry, sd.channels[i]);
        tarray.push_back(toml::value(std::move(tentry)));
    }

    t["channels"] = toml::value(std::move(tarray));
}

void
MRecordBackplaneP2PCon::emitSlotDescr(bytes &out_bin, SlotDescriptor &sd) {
    out_bin.emplace_back(std::byte{sd.type});
    out_bin.emplace_back(std::byte{sd.address});

    uchar channels_count = static_cast<uchar>(sd.channels.size());
    out_bin.emplace_back(std::byte{channels_count});
    for (uchar i = 0; i < channels_count; i++) {
        emitChannelDescr(out_bin, sd.channels[i]);
    }
}

//    ########     ###    ########   ######  #### ##    ##  ######
//    ##     ##   ## ##   ##     ## ##    ##  ##  ###   ## ##    ##
//    ##     ##  ##   ##  ##     ## ##        ##  ####  ## ##
//    ########  ##     ## ########   ######   ##  ## ## ## ##   ####
//    ##        ######### ##   ##         ##  ##  ##  #### ##    ##
//    ##        ##     ## ##    ##  ##    ##  ##  ##   ### ##    ##
//    ##        ##     ## ##     ##  ######  #### ##    ##  ######

template <typename T>
inline bool
MRecordBackplaneP2PCon::tryParseImpl(T v, Errs &errs) {
    bool valid = true;

    T array;
    if (!tryParseField_arr(v, "slots", array, errs)) {
        return false;
    }

    for (size_t i = 0; i < array.size(); i++) {
        T              entry = array[i];
        SlotDescriptor sd;

        std::stringstream s;
        s << "slots[" << i << "]";
        if (!tryParseSlotDescrImpl(entry, sd, errs)) {
            valid = false;
            continue;
        }

        slots.push_back(sd);
    }

    debug_printOutVals();
    return valid;
}

bool
MRecordBackplaneP2PCon::tryParse(nlohmann::json j, Errs &errs) {
    return tryParseImpl(j, errs);
}

bool
MRecordBackplaneP2PCon::tryParse(toml::value &t, Errs &errs) {
    return tryParseImpl(t, errs);
}

bool
MRecordBackplaneP2PCon::tryParseBinary(biterator begin, biterator end, Errs &errs) {
    OUT_OF_BOUNDS_GUARD_OFFSET("common", PICMG_HEADER_LEN)

    begin += PICMG_HEADER_LEN;

    while (begin < end) {
        SlotDescriptor sd;
        if (!tryParseSlotDescr(begin, end, sd, errs)) {
            return false;
        }

        slots.push_back(sd);
    }

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
MRecordBackplaneP2PCon::emitJSON(nlohmann::json &j) {
    j["record_id"]       = record_id;
    j["picmg_record_id"] = picmg_record_id;

    json jslots;
    for (size_t i = 0; i < slots.size(); i++) {
        SlotDescriptor &sd = slots[i];

        json jslot;
        jslot["type"]    = sd.type;
        jslot["address"] = sd.address;

        json jchannels;
        for (size_t i = 0; i < sd.channels.size(); i++) {
            json jchannel;
            jchannel["local_channel"]  = sd.channels[i].local_channel;
            jchannel["remote_channel"] = sd.channels[i].remote_channel;
            jchannel["remote_slot"]    = sd.channels[i].remote_slot;

            jchannels[i] = jchannel;
        }

        jslot["channels"] = jchannels;

        jslots[i] = jslot;
    }

    j["slots"] = jslots;
}

void
MRecordBackplaneP2PCon::emitTOML(toml::table &t) {
    t["record_id"]       = toml::value(SC_I(record_id));
    t["picmg_record_id"] = toml::value(SC_I(picmg_record_id));

    toml::array tslots;
    for (size_t i = 0; i < slots.size(); i++) {
        SlotDescriptor &sd = slots[i];

        toml::table tslot;
        tslot["type"]    = toml::value(sd.type);
        tslot["address"] = toml::value(sd.address);

        toml::array tchannels;
        for (size_t i = 0; i < sd.channels.size(); i++) {
            toml::table tchannel;
            tchannel["local_channel"]  = toml::value(sd.channels[i].local_channel);
            tchannel["remote_channel"] = toml::value(sd.channels[i].remote_channel);
            tchannel["remote_slot"]    = toml::value(sd.channels[i].remote_slot);

            tchannels.push_back(toml::value(std::move(tchannel)));
        }

        tslot["channels"] = toml::value(std::move(tchannels));

        tslots.push_back(toml::value(std::move(tslot)));
    }

    t["slots"] = toml::value(std::move(tslots));
}

bool
MRecordBackplaneP2PCon::emitBinary(bytes &out_bin, bool eol, Errs &errs) {
    UNUSED(errs);

    if (slots.size() == 0) {
        return true;
    }

    bytes header;
    bytes payload;
    bytes tmp;
    bytes tmppl;

    size_t i = 0;

    while (i < slots.size()) {
        // one full record: push into out_bin, prepend next one
        if (payload.size() + tmp.size() + MRECORD_HEADER_LEN_IPMI >= MAX_AREA_LEN) {
            prependPICMGHeader(payload);
            APPEND_BYTES(payload, tmppl);

            buildMRecordHeader(header, false, payload);
            APPEND_BYTES(out_bin, header);
            APPEND_BYTES(out_bin, payload);

            header.clear();
            payload.clear();
            tmppl.clear();
        }

        APPEND_BYTES(tmppl, tmp);
        tmp.clear();

        emitSlotDescr(tmp, slots[i]);
        i++;
    }

    APPEND_BYTES(tmppl, tmp);

    prependPICMGHeader(payload);
    APPEND_BYTES(payload, tmppl);

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

MRecordBackplaneP2PCon::MRecordBackplaneP2PCon()
    : MRecordBase("backplane_p2p_con", "Backplane Point-to-Point Connectivity") {
    record_id       = MRECORD_PICMG_RECORD;
    picmg_record_id = PICMGREC_BACKPLANE_P2PCON;
}
