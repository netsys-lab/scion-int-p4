#pragma once

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class tcpClient
{
    public:
        // Methods: 
        tcpClient();
        bool createClient(const tcp::endpoint& ep);
        void send(const std::string& report);
    private:
        // Methods:
        void write(const void* data, size_t size);
        void write(const std::string& data);
        
        // Attributes:
        std::unique_ptr<tcp::socket> tcpSocket;
        bool isActive;
};
