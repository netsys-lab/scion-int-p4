#pragma once

#include "controller.h"


class IntController : public Controller
{
public:
    IntController(SwitchConnection& con, const p4::config::v1::P4Info &p4Info);

public:
    /// \name Stream Message Handlers
    ///@{
    void handleArbitrationUpdate(
        SwitchConnection &con, const p4::v1::MasterArbitrationUpdate& arbUpdate) override;
    bool handlePacketIn(SwitchConnection& con, const p4::v1::PacketIn& packetIn) override;
    bool handleDigest(SwitchConnection& con, const p4::v1::DigestList& digestList) override;
    bool handleIdleTimeout(
        SwitchConnection& con, const p4::v1::IdleTimeoutNotification& idleTimeout) override;
    bool handleError(SwitchConnection& con, const p4::v1::StreamError& error) override;
    ///@}
};
