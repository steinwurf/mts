// Copyright (c) Steinwurf ApS 2017.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <mts/parser.hpp>

#include <mts/packetizer.hpp>
#include <fstream>

#include <gtest/gtest.h>

TEST(test_parser, test_ts_parsing)
{
    auto filename = "test.ts";
    std::ifstream file(filename, std::ios::binary|std::ios::ate);
    ASSERT_TRUE(file.is_open());
    auto size = file.tellg();

    file.seekg(0, std::ios::beg);

    mts::parser parser;
    std::vector<uint8_t> packet(mts::parser::packet_size());

    ASSERT_EQ(0U, size % packet.size());

    std::vector<uint16_t> first_pids =
        {
            256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256, 256,
            257, 256, 256, 256, 256, 256, 257, 256, 256, 256, 256, 256
        };

    uint32_t pes_found = 0;
    for (uint32_t i = 0; i < size / packet.size(); ++i)
    {
        file.read((char*)packet.data(), packet.size());
        std::error_code error;
        parser.read(packet.data(), error);
        if (parser.has_pes())
        {
            if (first_pids.size() > pes_found)
            {
                auto expected_pid = first_pids[pes_found];
                EXPECT_EQ(expected_pid, parser.pes_pid())
                    << "pes_found: " << pes_found;
            }
            pes_found++;
        }
        ASSERT_FALSE((bool) error);
    }
    EXPECT_EQ(198U, pes_found);
}
