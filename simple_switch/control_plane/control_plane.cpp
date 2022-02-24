#include "control_plane.h"
#include "bitstring.h"

#include <p4/v1/p4data.pb.h>
#include <p4/v1/p4runtime.pb.h>
#include <p4/v1/p4runtime.grpc.pb.h>
#include <p4/config/v1/p4info.pb.h>
#include <boost/range/adaptor/reversed.hpp>

#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

using p4::config::v1::P4Info;


ControlPlane::ControlPlane(
    std::unique_ptr<SwitchConnection> connection,
    std::unique_ptr<P4Info> p4Info, DeviceConfig config,
    size_t nCtrls)
    : con(std::move(connection))
    , p4Info(std::move(p4Info))
    , deviceConfig(std::move(config))
{
    ctrls.reserve(nCtrls);
}

void ControlPlane::run()
{
    using p4::v1::StreamMessageResponse;
    using boost::adaptors::reverse;

    if (!con->sendMasterArbitrationUpdate())
        return;

    StreamMessageResponse msg;
    while(con->readStream(msg))
    {
        switch (msg.update_case())
        {
        case StreamMessageResponse::kArbitration:
            handleArbitrationUpdate(msg.arbitration());
            for (const auto &ctrl : ctrls)
                ctrl->handleArbitrationUpdate(*con, msg.arbitration());
            break;
        case StreamMessageResponse::kPacket:
            for (const auto &ctrl : reverse(ctrls))
                if (ctrl->handlePacketIn(*con, msg.packet()))
                    break;
            break;
        case StreamMessageResponse::kDigest:
            for (const auto &ctrl : reverse(ctrls))
                if (ctrl->handleDigest(*con, msg.digest()))
                    break;
            break;
        case StreamMessageResponse::kIdleTimeoutNotification:
            for (const auto &ctrl : reverse(ctrls))
                if (ctrl->handleIdleTimeout(*con, msg.idle_timeout_notification()))
                    break;
            break;
        case StreamMessageResponse::kError:
            for (const auto &ctrl : reverse(ctrls))
                if (ctrl->handleError(*con, msg.error()))
                    break;
            break;
        default:
            std::cout << "Unknown data plane event" << std::endl;
            break;
        }
    }
}

void ControlPlane::handleArbitrationUpdate(const p4::v1::MasterArbitrationUpdate& arbUpdate)
{
    if (!arbUpdate.status().code())
    {
        std::cout << "Elected as primary controller" << std::endl;
        con->setPipelineConfig(*p4Info.get(), deviceConfig);
    }
    else
    {
        std::cout << "Other controller elected as primary" << std::endl;
    }
}
