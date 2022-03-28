#include "tcpClient.h"

#include <boost/asio.hpp>
#include <iostream>


using boost::asio::ip::tcp;

// Create TCP client
tcpClient::tcpClient() : isActive(false)
{}

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

void tcpClient::send(const std::string& report)
{
    if (isActive)
    {
        uint32_t len = report.length();
        write(&len, sizeof(uint32_t));
        write(report);
    }
}

void tcpClient::write(const void* data, size_t size)
{
    if (isActive)
    {
        try
        {
            boost::system::error_code err;
            boost::asio::write(*tcpSocket, boost::asio::buffer(data, size), err);
            // Error handling if write fails
            // If connection was closed...
            if (err == boost::asio::error::broken_pipe
                || err == boost::asio::error::connection_reset
                || err == boost::asio::error::connection_refused
                || err == boost::asio::error::bad_descriptor)
            {
                std::cout << "Info: TCP connection was closed." << std::endl;
                tcpSocket->close();
                isActive = 0;
            }
            // ...else
            else if (err)
            {
                isActive = 0;
                throw boost::system::system_error(err);
            }
        }
        catch (std::exception& e)
        {
            std::cout << "TCP Error: " << e.what() << std::endl;
        }
    }
}

void tcpClient::write(const std::string& data)
{
    if (isActive)
    {
        try
        {
            boost::system::error_code err;
            boost::asio::write(*tcpSocket, boost::asio::buffer(data), err);
            // Error handling if write fails
            // If connection was closed...
            if (err == boost::asio::error::broken_pipe
                || err == boost::asio::error::connection_reset
                || err == boost::asio::error::connection_refused
                || err == boost::asio::error::bad_descriptor)
            {
                std::cout << "Info: TCP connection was closed." << std::endl;
                tcpSocket->close();
                isActive = 0;
            }
            // ...else
            else if (err)
            {
                isActive = 0;
                throw boost::system::system_error(err);
            }
        }
        catch (std::exception& e)
        {
            std::cout << "TCP Error: " << e.what() << std::endl;
        }
    }
}
