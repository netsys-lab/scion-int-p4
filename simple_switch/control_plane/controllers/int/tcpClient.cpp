#include "tcpClient.h"

#include <boost/asio.hpp>
#include <iostream>


using boost::asio::ip::tcp;

// Create TCP client
tcpClient::tcpClient(const std::string& tcpAddress) : isActive(0)
{
    if (tcpAddress.length() > 0)
    {
        try
        {
            boost::asio::io_context io_context;
            
            std::stringstream tcpAddrStr(tcpAddress);
            std::string tcpAddr;
            std::string tcpPortStr;
            std::getline(tcpAddrStr, tcpAddr, ':');
            std::getline(tcpAddrStr, tcpPortStr, ':');
            uint16_t tcpPort = std::stoi(tcpPortStr);
            
            boost::system::error_code err;
            auto addr = boost::asio::ip::make_address(tcpAddr, err);
            if (err)
                throw boost::system::system_error(err);
            ep = tcp::endpoint(addr, tcpPort);
            
            tcpSocket = std::make_unique<tcp::socket>(io_context);
            tcpSocket->connect(ep, err);
            // Check, if the connection could be established
            if (err == boost::asio::error::broken_pipe || err == boost::asio::error::connection_refused)
            {
                std::cout << "Info: Could not connect to TCP server on port " << tcpPort << ". TCP server is not used." << std::endl;
            }
            else if (err)
                throw boost::system::system_error(err);
            else
                isActive = 1;
        }
        catch (std::exception& e)
        {
            std::cout << "TCP Error: " << e.what() << std::endl;
        }
    }
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
