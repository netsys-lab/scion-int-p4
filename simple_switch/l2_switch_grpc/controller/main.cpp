#include "common.h"
#include "controller.h"

#include <p4/config/v1/p4info.pb.h>

#include <google/protobuf/message.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include <fcntl.h>
#include <unistd.h>

#include <cstdlib>
#include <fstream>
#include <functional>
#include <iostream>
#include <stdexcept>

using p4::config::v1::P4Info;


/// \brief Parse a P4Info message from a file.
std::unique_ptr<P4Info> loadP4Info(const char* filename)
{
    int fd = open(filename, 0);
    if (fd < 0) throw std::runtime_error(std::string("File not found: ") + filename);
    try {
        google::protobuf::io::FileInputStream stream(fd);

        auto p4Info = std::make_unique<P4Info>();
        if (!google::protobuf::TextFormat::Parse(&stream, p4Info.get()))
            throw std::runtime_error("Invalid P4 Info");

        return p4Info;
    }
    catch (...) {
        close(fd);
        throw;
    }
}

/// \brief Load a device configuration from a file.
DeviceConfig loadDeviceConfig(const char* filename)
{
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open())
        throw std::runtime_error(std::string("File not found: ") + filename);

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0);

    DeviceConfig config(size);
    file.read(reinterpret_cast<char*>(config.data()), config.size());
    return config;
}

int main(int argc, char* argv[])
{
    if (argc != 6)
    {
        std::cout << "Usage: " << argv[0]
            << " <p4Info file> <config file> <switch address> <device id> <election id>\n";
        return 0;
    }
    try {
        Controller controller(loadP4Info(argv[1]), loadDeviceConfig(argv[2]), argv[3],
            std::atoi(argv[4]), std::atoll(argv[5]));
        controller.run();
        return 0;
    }
    catch (std::exception &e) {
        std::cout << "Error: " << e.what() << '\n';
        return 1;
    }
}
