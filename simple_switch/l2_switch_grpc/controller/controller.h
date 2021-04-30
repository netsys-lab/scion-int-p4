#pragma once

#include "common.h"
#include "connection.h"

#include <p4/v1/p4runtime.pb.h>
#include <p4/v1/p4runtime.grpc.pb.h>
#include <p4/config/v1/p4info.pb.h>


/// \brief A simple controller for the L2 learning switch.
class Controller
{
public:
    /// \brief Create a controller with the give P4Info and device configuration.
    /// \param[in] p4Info Switch API definition.
    /// \param[in] config Device specific configuration blob.
    /// \param[in] address Address and port of the switch's gRPC server.
    /// \param[in] deviceId Identifies a forwarding device to control within the switch.
    /// \param[in] electionId Election ID of the controller. Higher IDs win.
    /// \exception std::runtime_error
    Controller(
        std::unique_ptr<p4::config::v1::P4Info> p4Info, DeviceConfig config,
        const grpc::string& address = "localhost:9559",
        DeviceId deviceId = 0, uint64_t electionId = 1);

    /// \brief Run the controller. Returns when the connection has been closed by the switch.
    void run();

private:
    /// \name Initialization Functions
    ///@{
    bool createFloodMulticastGroup();
    bool installStaticTableEntries();
    bool configDigestMessages();
    ///@}

    /// \name Stream Message Handlers
    ///@{
    void handleArbitrationUpdate(const p4::v1::MasterArbitrationUpdate& arbUpdate);
    void handlePacketIn(const p4::v1::PacketIn& packetIn) { /* not needed */ }
    void handleDigest(const p4::v1::DigestList& digestList);
    void handleIdleTimeout(const p4::v1::IdleTimeoutNotification& idleTimeout) { /* not needed */ }
    void handleError(const p4::v1::StreamError& error);
    ///@}

private:
    std::unique_ptr<p4::config::v1::P4Info> p4Info;
    uint32_t macLearnDigestId;
    DeviceConfig deviceConfig;
    SwitchConnection connection;
};
