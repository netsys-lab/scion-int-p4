#include "tcpClient.h"

#include <boost/asio.hpp>
#include <iostream>


using boost::asio::ip::tcp;

// Create TCP client object
tcpClient::tcpClient() : isActive(false)
{}

// Create a TCP client and instantiate a connection
bool tcpClient::createClient(const tcp::endpoint& ep)
{
    boost::asio::io_context io_context;
    boost::system::error_code err;
    
    tcpSocket = std::make_unique<tcp::socket>(io_context);
    tcpSocket->connect(ep, err);
    // Check, if the connection could be established
    if (err)
        std::cout << "Error: Could not connect to TCP server: " << err.message() << std::endl << "TCP server is not used." << std::endl;
    else
        isActive = true;
    return isActive;
}

// Get TCP client state
bool tcpClient::getIsActive()
{
    return isActive;
}

// Send a string
bool tcpClient::send(const std::string& report)
{
    if (isActive)
    {
        uint32_t len = report.length();
        write(&len, sizeof(uint32_t));
        write(report);
    }
    return isActive;
}

// Helper to send const void data
bool tcpClient::write(const void* data, size_t size)
{
    if (isActive)
    {
        boost::system::error_code err;
        boost::asio::write(*tcpSocket, boost::asio::buffer(data, size), err);
        // Error handling if write fails
        // If error close connection and set inactive
        if (err)
        {
            std::cout << "TCP Error: " << err.message() << std::endl;
            tcpSocket->close();
            isActive = false;
        }
    }
    return isActive;
}

// Helper to send a string
bool tcpClient::write(const std::string& data)
{
    write(data.c_str(), data.length());
    return isActive;
}
