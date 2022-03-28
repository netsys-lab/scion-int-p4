#pragma once

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class tcpClient
{
    public:
        // Methods: 
        tcpClient();
        bool createClient(const tcp::endpoint& ep);
        bool send(const std::string& report);
        
        // Getter:
        bool getIsActive();
    private:
        // Methods:
        bool write(const void* data, size_t size);
        bool write(const std::string& data);
        
        // Attributes:
        std::unique_ptr<tcp::socket> tcpSocket;
        bool isActive;
};
