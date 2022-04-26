#pragma once

#include <string>
#include <sstream>

/// \brief Split a string "ip.address:port" into address and port.
/// \param[in] address The original address string.
/// \param[out] ipAddress The IP address as string.
/// \param[out] port The port as uint16.
static void splitIpAddress(std::string& address, std::string& ipAddress, uint16_t& port)
{
    if (address.length() > 0)
    {
        std::stringstream addrStr(address);
        std::string portStr;
        std::getline(addrStr, ipAddress, ':');
        std::getline(addrStr, portStr, ':');
        port = std::stoi(portStr);
    }
}

/// \brief Split a string "isdAddress-AS:Address" into ISD address and AS address.
/// \param[in] address The original SCION address string.
/// \param[out] isdAddress The ISD address as uint16.
/// \param[out] asAddress The AS address as uint16.
static void splitScionAddress(std::string& address, uint16_t& isdAddress, uint64_t& asAddress)
{
    std::string addressNamePart;
    std::stringstream addressStream(address);
    std::getline(addressStream, addressNamePart, '-');
    isdAddress = std::stoi("0x" + addressNamePart, nullptr, 16);
    // Get address of AS the switch belongs to
    asAddress = 0;
    while (std::getline(addressStream, addressNamePart, ':'))
    {
        asAddress = (asAddress << 16) + std::stoull("0x" + addressNamePart, nullptr, 16);
    }
}
