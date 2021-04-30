#include "PI/pi.h"
#include "PI/pi_mc.h"
#include "PI/frontends/generic/pi.h"

#include <iomanip>
#include <iostream>
#include <vector>

using std::size_t;


int main(int argc, char* argv[])
{
    pi_status_t status = PI_STATUS_SUCCESS;

    // Initialize the PI library
    // First parameter is not used anymore.
    // Second parameter is not used by bmv2 backend.
    std::cerr << "Initialize PI\n";
    status = pi_init(0, NULL);
    if (status != PI_STATUS_SUCCESS)
    {
        std::cerr << "pi_init failed with status " << status << '\n';
        return -1;
    }

    // Load dataplane configuration from a file
    std::cerr << "Load dataplane configuration\n";
    static const char* configPath = "build/l2_switch.json";
    pi_p4info_t *p4info = nullptr;
    status = pi_add_config_from_file(configPath, PI_CONFIG_TYPE_BMV2_JSON, &p4info);
    if (status != PI_STATUS_SUCCESS)
    {
        std::cerr << "pi_add_config_from_file failed with status " << status << '\n';
        return -1;
    }

    // Not implemented for bmv2
    // const size_t numDevices = pi_num_devices();
    // std::vector<pi_dev_id_t> deviceIds(numDevices);
    // pi_get_device_ids(deviceIds.data(), deviceIds.size());
    // std::cerr << "Available devices: ";
    // for (auto id : deviceIds)
    //     std::cerr << std::hex << id <<", ";
    // std::cerr << '\n';

    // Connect to bmv2 and set dataplane
    std::cerr << "Attach to device\n";
    const pi_dev_id_t deviceId = 0x0001; // not really used by bmv2 backend
    pi_assign_extra_t assignExtras[] = {
        {0, "port", "9090"}, // thrift port
        {1, nullptr, nullptr}
    };
    status = pi_assign_device(deviceId, p4info, assignExtras);
    if (status != PI_STATUS_SUCCESS)
    {
        std::cerr << "pi_assign_device failed with status " << status << '\n';
        return -1;
    }

    // Not used by bmv2
    // pi_session_handle_t session = {};
    // pi_session_init(&session);

    std::cerr << "Seting up multicast group\n";
    // no-op in bmv2
    pi_mc_session_handle_t mcSession = {};
    pi_mc_session_init(&mcSession);

    const pi_mc_rid_t rid = 1;
    pi_mc_port_t egressPorts[] = {1, 2, 3};
    pi_mc_node_handle_t mcNode = {};
    status = pi_mc_node_create(mcSession, deviceId, rid, sizeof(egressPorts) / sizeof(pi_mc_port_t),
        egressPorts, &mcNode);
    if (status != PI_STATUS_SUCCESS)
    {
        std::cerr << "pi_mc_node_create failed with status " << status << '\n';
    }

    const pi_mc_grp_id_t mcGroupId = 1;
    pi_mc_grp_handle_t mcGroup = {};
    status = pi_mc_grp_create(mcSession, deviceId, mcGroupId, &mcGroup);
    if (status != PI_STATUS_SUCCESS)
    {
        std::cerr << "pi_mc_grp_create failed with status " << status << '\n';
    }

    status = pi_mc_grp_attach_node(mcSession, deviceId, mcGroup, mcNode);
    if (status != PI_STATUS_SUCCESS)
    {
        std::cerr << "pi_mc_grp_attach_node failed with status " << status << '\n';
    }

    std::cout << "Setup completed";
    std::cin.get();

    pi_mc_grp_detach_node(mcSession, deviceId, mcGroup, mcNode);
    pi_mc_grp_delete(mcSession, deviceId, mcGroupId);
    pi_mc_node_delete(mcSession, deviceId, mcNode);

    // no-op in bmv2
    pi_mc_session_cleanup(mcSession);

    // Not used by bmv2
    // pi_session_cleanup(session);

    // Close connection
    std::cerr << "Disconnect from device\n";
    status = pi_remove_device(deviceId);
    if (status != PI_STATUS_SUCCESS)
    {
        std::cerr << "pi_remove_device failed with status " << status << '\n';
        return -1;
    }

    // Deallocate dataplane configuration
    pi_destroy_config(p4info);

    return 0;
}
