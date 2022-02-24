#include "int.h"


IntController::IntController(SwitchConnection& con, const p4::config::v1::P4Info &p4Info)
{
}

void IntController::handleArbitrationUpdate(
    SwitchConnection &con, const p4::v1::MasterArbitrationUpdate& arbUpdate)
{
    if (!arbUpdate.status().code())
    {
    }
}

bool IntController::handlePacketIn(SwitchConnection& con, const p4::v1::PacketIn& packetIn)
{
    return false;
}

bool IntController::handleDigest(SwitchConnection& con, const p4::v1::DigestList& digestList)
{
    return false;
}

bool IntController::handleIdleTimeout(
    SwitchConnection& con, const p4::v1::IdleTimeoutNotification& idleTimeout)
{
    return false;
}

bool IntController::handleError(SwitchConnection& con, const p4::v1::StreamError& error)
{
    return false;
}
