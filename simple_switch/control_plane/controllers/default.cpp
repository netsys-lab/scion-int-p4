#include "default.h"

#include "common.h"
#include "connection.h"

#include <iostream>


bool DefaultController::handlePacketIn(SwitchConnection& con, const p4::v1::PacketIn& packetIn)
{
    std::cout << "Unhandled packet" << std::endl;
    return true;
}

bool DefaultController::handleDigest(SwitchConnection& con, const p4::v1::DigestList& digestList)
{
    std::cout << "Received unknown digest" << std::endl;
    return true;
}

bool DefaultController::handleIdleTimeout(
    SwitchConnection& con, const p4::v1::IdleTimeoutNotification& idleTimeout)
{
    std::cout << "Unhandled idle timeout notification" << std::endl;
    return true;
}

bool DefaultController::handleError(SwitchConnection &con, const p4::v1::StreamError& error)
{
    std::cout << "Stream Error " << error.canonical_code() << ": ";
    std::cout << error.message() << std::endl;
    return true;
}
