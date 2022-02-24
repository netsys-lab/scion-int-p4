#include "connection.h"

#include <grpc/support/time.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>

#include <iostream>
#include <stdexcept>

using p4::config::v1::P4Info;


//////////////////
// WriteRequest //
//////////////////

WriteRequest::WriteRequest(DeviceId deviceId, ElectionId electionId)
    : request(std::make_unique<p4::v1::WriteRequest>())
{
    request->set_device_id(deviceId);
    auto election = request->mutable_election_id();
    election->set_high(0);
    election->set_low(electionId);
    request->set_atomicity(p4::v1::WriteRequest::CONTINUE_ON_ERROR);
}

void WriteRequest::addUpdate(p4::v1::Update_Type type, std::unique_ptr<p4::v1::Entity> entity)
{
    auto update = request->add_updates();
    update->set_type(type);
    update->set_allocated_entity(entity.release());
}


//////////////////////
// SwitchConnection //
//////////////////////

SwitchConnection::SwitchConnection(
    const grpc::string& address, DeviceId deviceId, ElectionId electionId)
    : deviceId(deviceId), electionId(electionId)
{
    channel = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
    auto t = gpr_time_add(gpr_now(GPR_CLOCK_REALTIME),
        gpr_time_from_seconds(10, GPR_TIMESPAN));
    if (!channel->WaitForConnected(t))
        throw std::runtime_error("Not connected");

    stub = p4::v1::P4Runtime::NewStub(channel);
    streamClientCtx = std::make_unique<grpc::ClientContext>();
    stream = stub->StreamChannel(streamClientCtx.get());
}

bool SwitchConnection::sendMasterArbitrationUpdate()
{
    p4::v1::StreamMessageRequest request;
    auto arbUpdate = request.mutable_arbitration();
    arbUpdate->set_device_id(deviceId);
    // Leave role unset to request full access
    // arbUpdate->mutable_role()->set_id(0);
    auto election = arbUpdate->mutable_election_id();
    election->set_high(0);
    election->set_low(electionId);
    return stream->Write(request);
}

bool SwitchConnection::setPipelineConfig(const P4Info& p4Info, const DeviceConfig& deviceConfig)
{
    using PipelineConfigRequest = p4::v1::SetForwardingPipelineConfigRequest;

    PipelineConfigRequest request;
    request.set_device_id(deviceId);
    auto election = request.mutable_election_id();
    election->set_high(0);
    election->set_low(electionId);
    request.set_action(PipelineConfigRequest::VERIFY_AND_COMMIT);
    auto config = request.mutable_config();
    *config->mutable_p4info() = p4Info;
    config->set_p4_device_config(deviceConfig.data(), deviceConfig.size());

    grpc::ClientContext ctx;
    p4::v1::SetForwardingPipelineConfigResponse response;
    grpc::Status status = stub->SetForwardingPipelineConfig(&ctx, request, &response);
    if (!status.ok())
        std::cout << "Setting pipeline config failed: " << status.error_message() << std::endl;
    return status.ok();
}

bool SwitchConnection::sendWriteRequest(const WriteRequest &request)
{
    grpc::ClientContext ctx;
    p4::v1::WriteResponse response;
    grpc::Status status = stub->Write(&ctx, *request.request.get(), &response);
    if (!status.ok())
        std::cout << "Write request failed: " << status.error_message() << std::endl;
    return status.ok();
}

bool SwitchConnection::ackDigestList(uint32_t digestId, uint64_t listId)
{
    p4::v1::StreamMessageRequest request;
    auto digestAck = request.mutable_digest_ack();
    digestAck->set_digest_id(digestId);
    digestAck->set_list_id(listId);
    return stream->Write(request);
}
