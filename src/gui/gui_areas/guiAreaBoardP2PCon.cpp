#include "guiAreaBoardP2PCon.hpp"
#include "fieldArr.hpp"
#include "guiAreaBase.hpp"
#include "types.hpp"

GUIAreaBoardP2PCon::GUIAreaBoardP2PCon()
    : GUIAreaMRecBase(
          "Board Point-to-Point Connectivity", MRECORD_PICMG_RECORD, PICMGREC_BOARD_P2PCON) {
    //
    INIT_FIELD(FieldArr, "guids", "OEM GUID List", guids_ad, true);
    INIT_FIELD(FieldArr, "link_descriptors", "Link Descriptor\nlist", link_descriptors_ad);
}
