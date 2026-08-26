#include "guiAreaIPConnection.hpp"
#include "fieldStr.hpp"
#include "guiAreaBase.hpp"
#include "types.hpp"

GUIAreaIPConnection::GUIAreaIPConnection()
    : GUIAreaMRecBase("IP Connection", MRECORD_PICMG_RECORD, PICMGREC_IP_CONNECTION) {
    //
    INIT_FIELD(FieldStr, "ip_address", "Shelf Manager\nIP Address", FSTR_TYPE_IP);
    INIT_FIELD(FieldStr, "gateway_address", "Default Gateway\nAddress", FSTR_TYPE_IP);
    INIT_FIELD(FieldStr, "subnet_mask", "Subnet Mask", FSTR_TYPE_IP);
}
