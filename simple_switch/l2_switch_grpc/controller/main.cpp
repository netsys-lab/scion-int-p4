#include "control_plane.h"
#include "p4_util.h"
#include "controllers/default.h"
#include "controllers/mac_learn.h"

#include <iostream>
#include <stdexcept>


int main(int argc, char* argv[])
{
    // TODO: Better command line parsing
    if (argc < 10 || argc > 11)
    {
        std::cout << "Usage: " << argv[0]
            << " <p4Info file> <config file> <switch address> <device id> <election id>\n";
        return 0;
    }
    try {
        ControlPlane control(
            std::make_unique<SwitchConnection>(argv[3], std::atoi(argv[4]), std::atoll(argv[5])),
            loadP4Info(argv[1]),
            loadDeviceConfig(argv[2])
        );
        control.addController<DefaultController>();
        control.addController<MacLearningCtrl>();
        control.run();
        return 0;
    }
    catch (std::exception &e) {
        std::cout << "Error: " << e.what() << '\n';
        return 1;
    }
}
