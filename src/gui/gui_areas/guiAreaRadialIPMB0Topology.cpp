#include "guiAreaRadialIPMB0Topology.hpp"
#include "fieldArr.hpp"
#include "fieldStr.hpp"
#include "guiAreaBase.hpp"

GUIAreaRadialIPMB0Topology::GUIAreaRadialIPMB0Topology()
    : GUIAreaMRecBase(
          "Radial IPMB-0 Link Mapping", MRECORD_PICMG_RECORD, PICMGREC_RADIAL_IPMB0_LINK_MAP) {
    INIT_FIELD(FieldStr, "connector_definer", "IPMB-0 Connector\nDefiner", FSTR_TYPE_HEX, 3);
    INIT_FIELD(
        FieldStr, "connector_version_id", "IPMB-0 Connector\nVersion ID", FSTR_TYPE_HEX, 2);
    INIT_FIELD(FieldArr, "hub_descriptors", "IPMB-0 Hub Descriptors", hub_descriptor);
}
