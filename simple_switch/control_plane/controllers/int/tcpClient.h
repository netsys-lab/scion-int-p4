#pragma once

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class tcpClient
{
    public:
        // Methods: 
        tcpClient(const std::string& tcpAddress);
        void send(const std::string& report);
    private:
        // Methods:
        void write(const void* data, size_t size);
        void write(const std::string& data);
        
        // Attributes:
        tcp::endpoint ep;
        std::unique_ptr<tcp::socket> tcpSocket;
        uint8_t isActive;
};
