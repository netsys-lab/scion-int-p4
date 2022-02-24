#pragma once

#include "common.h"
#include "connection.h"
#include "controller.h"

#include <p4/v1/p4runtime.pb.h>
#include <p4/v1/p4runtime.grpc.pb.h>
#include <p4/config/v1/p4info.pb.h>

#include <memory>
#include <utility>
#include <vector>


/// \brief A P4Runtime controller consisting of a stack of subcontrollers implementing the
/// \ref Controller interface.
///
/// Controllers are added to the stack by calling addController(). Events received from the
/// switch trigger callbacks in the subcontrollers. Most callbacks are invoked from the top of the
/// stack (last controller added) to the bottom (first controller added). If any controller
/// indicates handling of the event is complete it will not trave further down the stack.
///
/// The handleArbitrationUpdate() callback is processed in reverse order (from bottom to top of the
/// stack), since it is used to perform data plane initialization when this controller is elected as
/// primary.
class ControlPlane
{
public:
    /// \brief Create a controller with the give P4Info and device configuration.
    /// \param[in] connection SwitchConnection object already connected to the switch.
    /// \param[in] p4Info Switch API definition.
    /// \param[in] config Device specific configuration blob.
    /// \param[in] nCtrls Expected number of controllers to be added to the subcontroller stack.
    /// \exception std::runtime_error
    ControlPlane(
        std::unique_ptr<SwitchConnection> connection,
        std::unique_ptr<p4::config::v1::P4Info> p4Info,
        DeviceConfig config, size_t nCtrls = 0);

    /// \brief Construct a new controller on top of the current controller stack.
    template <typename T, typename... Args>
    void addController(Args&&... args)
    {
        ctrls.emplace_back(std::make_unique<T>(*con, *p4Info, std::forward<Args>(args)...));
    }

    /// \brief Run the controller. Returns when the connection has been closed by the switch.
    void run();

private:
    void handleArbitrationUpdate(const p4::v1::MasterArbitrationUpdate& arbUpdate);

private:
    std::unique_ptr<SwitchConnection> con;
    std::unique_ptr<p4::config::v1::P4Info> p4Info;
    DeviceConfig deviceConfig;
    std::vector<std::unique_ptr<Controller>> ctrls;
};
