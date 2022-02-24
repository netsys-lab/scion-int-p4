#pragma once

#include "controller.h"


/// \brief Controller that prints callback invocations to stdout. Useful at the bottom of the
/// controller stack to report unhandeld events.
class DefaultController : public Controller
{
public:
    DefaultController(SwitchConnection& con, const p4::config::v1::P4Info &p4Info)
    {};

public:
    bool handlePacketIn(SwitchConnection& con, const p4::v1::PacketIn& packetIn) override;
    bool handleDigest(SwitchConnection& con, const p4::v1::DigestList& digestList) override;
    bool handleIdleTimeout(
        SwitchConnection& con, const p4::v1::IdleTimeoutNotification& idleTimeout) override;
    bool handleError(SwitchConnection& con, const p4::v1::StreamError& error) override;
};
