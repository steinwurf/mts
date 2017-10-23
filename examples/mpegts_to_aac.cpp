// Copyright (c) Steinwurf ApS 2017.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <fstream>
#include <iostream>

#include <mts/parser.hpp>
#include <mts/pes.hpp>
#include <mts/stream_type.hpp>
#include <mts/stream_type_to_string.hpp>

int main(int argc, char* argv[])
{
    if (argc != 3 || std::string(argv[1]) == "--help")
    {
        auto usage = "./mpegts_to_aac MPEG_TS_INPUT AAC_OUTPUT";
        std::cout << usage << std::endl;
        return 0;
    }

    auto filename = std::string(argv[1]);

    std::ifstream file(filename, std::ios::binary|std::ios::ate);
    assert(file.is_open());

    // Create the AAC output file
    std::ofstream aac_file(argv[2], std::ios::binary);

    auto size = file.tellg();

    file.seekg(0, std::ios::beg);

    auto packet_size = 188;

    mts::parser parser(packet_size);
    std::vector<uint8_t> packet(packet_size);

    for (uint32_t i = 0; i < size / packet.size(); ++i)
    {
        file.read((char*)packet.data(), packet.size());
        std::error_code error;
        parser.read(packet.data(), error);
        if (parser.has_pes())
        {
            auto pid = parser.pes_pid();
            mts::stream_type type = (mts::stream_type)parser.stream_type(pid);
            if (type != mts::stream_type::adts_transport_13818_7)
                continue;
            auto& pes_data = parser.pes_data();
            std::error_code error;
            auto pes = mts::pes::parse(pes_data.data(), pes_data.size(), error);
            if (error)
                continue;
            aac_file.write((char*)pes->payload_data(), pes->payload_size());
        }
    }
    if ((std::size_t)aac_file.tellp() == 0)
    {
        std::cout << "No AAC data found." << std::endl;
    }
    aac_file.close();
    return 0;
}
