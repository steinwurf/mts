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
    if (argc != 2 || std::string(argv[1]) == "--help")
    {
        auto usage = "./mpegts_inspect MPEG_TS_INPUT";
        std::cout << usage << std::endl;
        return 0;
    }

    auto filename = std::string(argv[1]);

    std::ifstream file(filename, std::ios::binary|std::ios::ate);
    assert(file.is_open());

    auto size = file.tellg();

    file.seekg(0, std::ios::beg);

    auto packet_size = 188;

    mts::parser parser(packet_size);
    std::vector<uint8_t> packet(packet_size);

    std::map<mts::stream_type, uint32_t> stream_types;
    for (uint32_t i = 0; i < size / packet.size(); ++i)
    {
        file.read((char*)packet.data(), packet.size());
        std::error_code error;
        parser.read(packet.data(), error);
        if (parser.has_pes())
        {
            auto pid = parser.pes_pid();
            mts::stream_type type = (mts::stream_type)parser.stream_type(pid);

            if (stream_types.count(type) == 0)
            {
                stream_types[type] = 0;
            }
            stream_types[type]++;
        }
    }
    if (stream_types.size() == 0)
    {
        std::cout << "No stream types found" << std::endl;
        return 1;
    }

    for (auto& item : stream_types)
    {
        auto type = item.first;
        auto count = item.second;
        std::cout << mts::stream_type_to_string(type) << ": " << count << std::endl;
    }

    return 0;
}
