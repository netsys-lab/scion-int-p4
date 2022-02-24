#pragma once

#include "common.h"
#include "connection.h"

#include <p4/v1/p4runtime.pb.h>
#include <p4/v1/p4runtime.grpc.pb.h>
#include <p4/config/v1/p4info.pb.h>


/// \brief Interface for P4Runtime controllers.
///
/// Controllers must have a constructor taking at least two arguments:
/// 1. A reference to the connection object.
/// 2. A constant reference to the P4Info message describing the control plane interface of the
/// data plane program.
class Controller
{
public:
    virtual ~Controller() = default;

public:
    /// \brief Handle an arbitration update message.
    /// \details An arbitration update is send to all controllers for a certain device and role
    /// combination when the primary controller for that role changes.
    /// \return Returning true indicates that the event has been handled and no more processing is
    /// required by other controllers. If false is returned, the event will continue to travel down
    /// the stack of controllers.
    virtual void handleArbitrationUpdate(
        SwitchConnection& con, const p4::v1::MasterArbitrationUpdate& arbUpdate)
    {};

    /// \brief Handle packet-in message.
    /// \return Returning true indicates that the event has been handled and no more processing is
    /// required by other controllers. If false is returned, the event will continue to travel down
    /// the stack of controllers.
    virtual bool handlePacketIn(SwitchConnection& con, const p4::v1::PacketIn& packetIn)
    { return false; };

    /// \brief Handle a digest list received from the dataplane.
    /// \details Digests are buffered before the are sent to the controller bundeled in a digest list.
    /// \return Returning true indicates that the event has been handled and no more processing is
    /// required by other controllers. If false is returned, the event will continue to travel down
    /// the stack of controllers.
    virtual bool handleDigest(SwitchConnection& con, const p4::v1::DigestList& digestList)
    { return false; };

    /// \brief Handle idle timout notifications.
    /// \return Returning true indicates that the event has been handled and no more processing is
    /// required by other controllers. If false is returned, the event will continue to travel down
    /// the stack of controllers.
    virtual bool handleIdleTimeout(
        SwitchConnection& con, const p4::v1::IdleTimeoutNotification& idleTimeout)
    { return false; };

    /// \brief Handle error status notification.
    /// \details Stream errors indicate an error with a previous StreamMessageRequest.
    /// \return Returning true indicates that the event has been handled and no more processing is
    /// required by other controllers. If false is returned, the event will continue to travel down
    /// the stack of controllers.
    virtual bool handleError(SwitchConnection& con, const p4::v1::StreamError& error)
    { return false; };
};
