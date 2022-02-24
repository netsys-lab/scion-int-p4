/// \file
/// Utility function for loading P4 compiler artifacts.

#include "p4_util.h"

#include <google/protobuf/message.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include <fcntl.h>
#include <unistd.h>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <stdexcept>


/// \brief Parse a P4Info message from a file.
/// \exception std::runtime_error File could not be read or parsed.
std::unique_ptr<p4::config::v1::P4Info> loadP4Info(const char* filename)
{
    int fd = open(filename, 0);
    if (fd < 0) throw std::runtime_error(std::string("File not found: ") + filename);
    try {
        google::protobuf::io::FileInputStream stream(fd);

        auto p4Info = std::make_unique<p4::config::v1::P4Info>();
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
/// \exception std::runtime_error File could not be read.
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
