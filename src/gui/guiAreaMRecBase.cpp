#include "guiAreaMRecBase.hpp"
#include "json.hpp"
#include "types.hpp"

int
findMRecordEntry(nlohmann::json jarray, uchar mrec_id, uchar picmg_mrec_id) {
    if (!jarray.is_array() || jarray.empty()) {
        return -1;
    }

    for (size_t i = 0; i < jarray.size(); ++i) {
        uchar          rec_id       = 0;
        uchar          picmg_rec_id = 0;
        nlohmann::json jentry       = jarray[i];

        if (!jentry.contains(FIELD_TAG_MRECORD_ID)) {
            continue;
        }
        rec_id = jentry[FIELD_TAG_MRECORD_ID].get<int>();

        if (jentry.contains(FIELD_TAG_PICMG_MRECORD_ID)) {
            picmg_rec_id = jentry[FIELD_TAG_PICMG_MRECORD_ID].get<int>();
        }

        if (mrec_id == rec_id && (picmg_mrec_id == 0 || picmg_mrec_id == picmg_rec_id)) {
            return i;
        }
    }

    return -1;
}

// get reference to json array containing multirecords
// records *shall* contain multirecord id field and *may* contain PICMG record id
void
GUIAreaMRecBase::get(nlohmann::json &j) {
    nlohmann::json jarea;

    jarea[FIELD_TAG_MRECORD_ID] = record_id;
    if (picmg_record_id != 0) {
        jarea[FIELD_TAG_PICMG_MRECORD_ID] = picmg_record_id;
    }

    for (auto &&f : fields) {
        f->get(jarea);
    }

    j.push_back(jarea);
}

// get reference to json array containing multirecords
// records *shall* contain multirecord id field and *may* contain PICMG record id
void
GUIAreaMRecBase::set(nlohmann::json &j) {
    nlohmann::json jarray;

    jarray  = j.get<nlohmann::json::array_t>();
    int ind = -1;
    if ((ind = findMRecordEntry(jarray, record_id, picmg_record_id) < 0)) {
        return;
    }

    nlohmann::json jarea = jarray[ind];
    for (auto &&i : fields) {
        i->set(jarea);
    }
}

GUIAreaMRecBase::GUIAreaMRecBase(std::string label, uchar record_id, uchar picmg_record_id)
    : GUIAreaBase("", label), record_id(record_id), picmg_record_id(picmg_record_id) {
    //
}
