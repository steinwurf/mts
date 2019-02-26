// Copyright (c) Steinwurf ApS 2017.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <fstream>
#include <iostream>

#include <mts/parser.hpp>
#include <mts/pes.hpp>
#include <mts/stream_type.hpp>

#include <boost/iostreams/device/mapped_file.hpp>

int main(int argc, char* argv[])
{
    if (argc != 3 || std::string(argv[1]) == "--help")
    {
        auto usage = "./mpegts_to_h264 MPEG_TS_INPUT H264_OUTPUT";
        std::cout << usage << std::endl;
        return 0;
    }

    auto filename = std::string(argv[1]);

    boost::iostreams::mapped_file_source file;
    file.open(filename);
    assert(file.is_open());

    // Create the h264 output file
    std::ofstream h264_file(argv[2], std::ios::binary);

    mts::parser parser;

    uint64_t offset = 0;
    for (uint32_t i = 0; i < file.size() / mts::parser::packet_size(); ++i)
    {
        std::error_code error;
        parser.read((uint8_t*)file.data() + offset, error);
        offset += mts::parser::packet_size();
        if (parser.has_pes())
        {
            auto pid = parser.pes_pid();
            mts::stream_type type = (mts::stream_type)parser.stream_type(pid);
            if (type != mts::stream_type::avc_video_stream)
                continue;

            auto& pes_data = parser.pes_data();
            std::error_code error;
            auto pes = mts::pes::parse(pes_data.data(), pes_data.size(), error);
            if (error)
                continue;

            h264_file.write((char*)pes->payload_data(), pes->payload_size());
        }
    }
    if ((std::size_t)h264_file.tellp() == 0)
    {
        std::cout << "No H.264 data found." << std::endl;
    }

    file.close();
    h264_file.close();
    return 0;
}
