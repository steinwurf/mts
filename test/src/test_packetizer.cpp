// Copyright (c) Steinwurf ApS 2017.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <mts/packetizer.hpp>

#include <algorithm>
#include <gtest/gtest.h>

TEST(test_packetizer, packets_of_size_187)
{
    srand(static_cast<uint32_t>(time(0)));

    uint32_t packet_size = 187;
    std::vector<std::vector<uint32_t>> test_packets = {
        {0, 65},
        {1, 155},
        {2, 152},
        {3},
        {4, 71, 125},
        {5, 116},
        {6, 121},
        {7},
        {8, 155},
        {2, 9},
        {10, 28},
        {11, 50},
        {12},
        {13, 179},
        {14, 162, 172, 183},
        {15}
    };

    mts::packetizer packetizer;

    std::vector<uint8_t> result;
    packetizer.set_on_data([&result](auto data, auto size){
        EXPECT_EQ(mts::packetizer::sync_byte(), data[0]);
        result.insert(result.end(), data, data + size);
    });

    std::vector<uint8_t> expected_data(test_packets.size() * packet_size);
    std::generate(expected_data.begin(), expected_data.end(), rand);
    auto offset = 0;
    for(const auto& test_packet : test_packets)
    {
        for(auto sync_index : test_packet)
        {
            expected_data[offset + sync_index] = mts::packetizer::sync_byte();
        }
        packetizer.read(expected_data.data() + offset, packet_size);
        offset += packet_size;
    }
    EXPECT_EQ(188 - 16U, packetizer.buffered());
    EXPECT_EQ(expected_data.size(), result.size() + packetizer.buffered());
    expected_data.resize(expected_data.size() -  packetizer.buffered());
    EXPECT_EQ(expected_data, result);
}