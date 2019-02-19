// Copyright (c) Steinwurf ApS 2017.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <fstream>
#include <iostream>
#include <thread>
#include <functional>

#include <boost/asio.hpp>

#include <mts/parser.hpp>
#include <mts/pes.hpp>
#include <mts/stream_type.hpp>
#include <mts/stream_type_to_string.hpp>
#include <mts/packetizer.hpp>

struct receiver
{
    receiver(
        boost::asio::io_service& io_service,
        const boost::asio::ip::address_v4& ip,
        uint16_t port,
        mts::stream_type type) :
        m_socket(io_service),
        m_type(type),
        m_receive_buffer(65001),
        m_parser(188)
    {
        m_packetizer.set_on_data(
            std::bind(&receiver::parse_ts_packet, this, std::placeholders::_1));

        boost::asio::ip::udp::endpoint endpoint = { ip, port };

        m_socket.open(endpoint.protocol());
        if (ip.is_multicast())
        {
            // bind to IF_ANY if multicast addr
            m_socket.bind({endpoint.protocol(), port});
            m_socket.set_option(boost::asio::ip::multicast::join_group(ip));
        }
        else if (ip == ip.broadcast()) // is broadcast
        {
            // bind to IF_ANY if broadcast addr
            m_socket.bind({endpoint.protocol(), port});
        }
        else
        {
            // bind to specified address
            m_socket.bind(endpoint);
        }
    }

    void do_async_receive()
    {
        if (!m_socket.is_open())
            return;

        using namespace std::placeholders;
        m_socket.async_receive(
            boost::asio::buffer(m_receive_buffer),
            std::bind(&receiver::handle_async_receive, this, _1, _2));
    }

    void handle_async_receive(const boost::system::error_code& ec, size_t bytes)
    {
        if (ec)
            return;

        m_packetizer.read(m_receive_buffer.data(), bytes);
        do_async_receive();
    }

    void parse_ts_packet(const uint8_t* data)
    {
        assert(data[0] == 0x47);
        std::error_code error;
        m_parser.read(data, error);
        if (error)
        {
            return;
        }

        if (m_parser.has_pes())
        {
            auto pid = m_parser.pes_pid();
            mts::stream_type t = (mts::stream_type)m_parser.stream_type(pid);
            if (std::find(m_types.begin(), m_types.end(), t) == m_types.end())
            {
                m_types.push_back(t);
                std::cout << "Found (" << pid << ") "
                          << mts::stream_type_to_string(t) << std::endl;
            }
            if (t != m_type)
            {
                return;
            }

            auto& pes_data = m_parser.pes_data();
            auto pes = mts::pes::parse(pes_data.data(), pes_data.size(), error);
            if (error)
            {
                std::cout << "Invalid PES" << std::endl;
                return;
            }
            if (m_callback)
            {
                m_callback(pes->payload_data(), pes->payload_size());
            }
        }
    }

    void cancel()
    {
        m_socket.cancel();
        m_socket.close();
    }

    void set_callback(std::function<void(const uint8_t*, uint32_t)> callback)
    {
        m_callback = callback;
    }

private:

    boost::asio::ip::udp::socket m_socket;
    mts::stream_type m_type;
    std::vector<uint8_t> m_receive_buffer;

    mts::parser m_parser;
    mts::packetizer m_packetizer;
    std::vector<mts::stream_type> m_types;
    std::vector<uint8_t> m_buffer;

    std::function<void(const uint8_t*, uint32_t)> m_callback;
};


int main(int argc, char* argv[])
{
    if (argc != 4 || std::string(argv[1]) == "--help")
    {
        auto usage = "./mpegts_stream_to_file STREAM_IP STREAM_PORT OUTPUT.[h264|aac]";
        std::cout << usage << std::endl;
        return 0;
    }

    auto ip_string = std::string(argv[1]);
    auto port_string = std::string(argv[2]);
    auto filename = std::string(argv[3]);
    auto extension = filename.substr(filename.find_last_of(".") + 1);

    mts::stream_type type;
    if (extension == "h264")
    {
        type = mts::stream_type::avc_video_stream;
    }
    else if (extension == "aac")
    {
        type = mts::stream_type::adts_transport_13818_7;
    }
    else
    {
        std::cout << "Unsupported extension '" << extension << "'" << std::endl;
        return 1;
    }

    std::cout << "Creating file " << filename << std::endl;
    // Create the output file
    std::ofstream out_file(argv[3], std::ios::binary);

    boost::asio::io_service io_service;
    auto ip = boost::asio::ip::address_v4::from_string(ip_string);
    uint16_t port = std::stoi(port_string);

    receiver r(io_service, ip, port, type);

    std::thread io_thread([&io_service]()
    {
        io_service.run();
    });

    r.set_callback([&out_file](auto data, auto size)
    {
        out_file.write((char*)data, size);
    });
    r.do_async_receive();

    std::cout << '\n' << "Press a key to stop recording..." << std::endl;
    std::cin.get();
    std::cout << "Stopping..." << std::endl;
    r.cancel();
    io_service.stop();
    io_thread.join();
    std::cout << "  Stopped." << std::endl;

    if ((std::size_t)out_file.tellp() == 0)
    {
        std::cout << "No " << mts::stream_type_to_string(type) << " data found." << std::endl;
    }
    else
    {
        std::cout << "wrote " << out_file.tellp() << "kb" << std::endl;
    }
    out_file.close();
    return 0;
}