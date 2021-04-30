#pragma once

#include <p4/v1/p4runtime.pb.h>
#include <p4/v1/p4runtime.grpc.pb.h>
#include <p4/config/v1/p4info.pb.h>

#include <grpc/grpc.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>

#include "common.h"


/// \brief Encapsulated a write request for the dataplane. Constructed by
/// SwitchConnection::createWriteRequest.
class WriteRequest
{
public:
    /// \brief Add an update the the request.
    /// \param[in] type Type of the update (p4::v1::Update::{INSERT|MODIFY|DELETE}).
    /// \param[out] entity
    void addUpdate(p4::v1::Update_Type type, std::unique_ptr<p4::v1::Entity> entity);

private:
    WriteRequest(DeviceId deviceId, ElectionId electionId);
    friend class SwitchConnection;

private:
    std::unique_ptr<p4::v1::WriteRequest> request;
};


/// \brief P4Runtime connection to a switch.
class SwitchConnection
{
public:
    /// \brief Connects to the switch and opens the persistent stream channel.
    /// \param[in] address Address and port of the switch's gRPC server.
    /// \param[in] deviceId Identifies a forwarding device to control within the switch.
    /// \param[in] electionId Election ID of the controller. Higher IDs win.
    SwitchConnection(const grpc::string& address, DeviceId deviceId, ElectionId electionId);

    /// \brief Send a master arbitration update to the switch to announce the  controller's
    /// presence. Must be called before anything else.
    /// \return True on success, false on failure.
    bool sendMasterArbitrationUpdate();

    /// \brief Apply a new pipeline configuration to the switch.
    /// \return True on success, false on failure.
    bool setPipelineConfig(const p4::config::v1::P4Info& p4Info, const DeviceConfig& deviceConfig);

    /// \brief Return an empty WriteRequest to be populated with updates by the caller.
    WriteRequest createWriteRequest() const
    {
        return WriteRequest(deviceId, electionId);
    }

    /// \brief Send a write request to the switch.
    /// \return True on success, false on failure.
    bool sendWriteRequest(const WriteRequest &request);

    /// \brief Read the next message from the persistent stream.
    bool readStream(p4::v1::StreamMessageResponse& response)
    {
        return stream->Read(&response);
    }

    /// \brief Acknowledge reception of a digest list received on the persistent stream.
    /// \param[in] digestId ID of the digest extern.
    /// \param[in] listId List ID which will be acknowledged.
    /// \return True on success, false on failure.
    bool ackDigestList(uint32_t digestId, uint64_t listId);

private:
    const DeviceId deviceId;
    const ElectionId electionId;
    std::shared_ptr<grpc::Channel> channel;
    std::unique_ptr<grpc::ClientContext> streamClientCtx;
    std::unique_ptr<p4::v1::P4Runtime::Stub> stub;
    std::unique_ptr<grpc::ClientReaderWriterInterface<
        p4::v1::StreamMessageRequest, p4::v1::StreamMessageResponse>>
        stream;
};
