#include "guiAreaPowerDistribution.hpp"
#include "guiAreaMRecBase.hpp"
#include "types.hpp"

GUIAreaPowerDistribution::GUIAreaPowerDistribution()
    : GUIAreaMRecBase("Power Distribuiton", MRECORD_PICMG_RECORD, PICMGREC_POWER_DISTRIBUTION) {
    //
    INIT_FIELD(FieldArr, "power_feeds", "Power Feeds", mapping_descr);
}
