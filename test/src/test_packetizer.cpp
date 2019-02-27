// Copyright (c) Steinwurf ApS 2017.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <mts/packetizer.hpp>
#include <mts/packetizer.hpp>
#include <mts/packetizer.hpp>

#include <algorithm>
#include <gtest/gtest.h>
// Copyright (c) Steinwurf ApS 2017.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <mts/packetizer.hpp>

#include <algorithm>
#include <gtest/gtest.h>

namespace
{
std::vector<uint8_t> generate_ts_packets(uint32_t ts_packets, uint32_t offset)
{
    assert(offset < 188U);

    // Create data
    std::vector<uint8_t> buffer(offset + (ts_packets * 188));
    std::generate(buffer.begin(), buffer.end(), rand);

    // Add sync bytes
    for (uint32_t i = offset; i < buffer.size(); i += 188)
    {
        buffer[i] = 0x47;
    }

    return buffer;
}

template <typename T>
std::vector<T> flatten(const std::vector<std::vector<T>>& in)
{
    std::vector<T> out;
    for (const auto& v : in)
        std::copy(v.begin(), v.end(), std::inserter(out, out.end()));
    return out;
}

void test(uint32_t ts_packets, uint32_t udp_packet_size, uint32_t data_offset)
{
    std::vector<std::vector<uint8_t>> result;
    mts::packetizer packetizer([&result](auto data, auto size)
    {
        EXPECT_EQ(mts::packetizer::sync_byte(), data[0]);
        result.push_back({data, data + size});
    });

    // Generate packets
    auto ts_data = generate_ts_packets(ts_packets, data_offset);

    // Calculate how many udp packets needed
    auto udp_packets = ts_data.size() / udp_packet_size;

    // Resize result to only contain what we can send
    ts_data.resize(udp_packets * udp_packet_size);

    auto offset = 0;
    for (uint32_t i = 0; i < udp_packets; i++)
    {
        packetizer.read(ts_data.data() + offset, udp_packet_size);
        offset += udp_packet_size;
    }

    std::vector<uint8_t> expected_data =
        {
            ts_data.begin() + data_offset, ts_data.end() - packetizer.buffered()
        };

    std::vector<std::vector<uint8_t>> expected_packets;

    auto it = expected_data.cbegin();
    while (it != expected_data.cend())
    {
        expected_packets.push_back({it, it + 188});
        it += 188;
    }

    EXPECT_NE(0U, result.size());

    EXPECT_EQ(expected_packets.size(), result.size());
    auto packet_id = 0;
    for (auto expected_packet : expected_packets)
    {
        EXPECT_EQ(expected_packet, result[packet_id]);
        packet_id++;
    }
    EXPECT_EQ(expected_packets, result);
    EXPECT_EQ(expected_data, flatten(result));
}
}

TEST(test_packetizer, packets_of_size_187_no_offset)
{
    test(100, 187, 0);
}

TEST(test_packetizer, packets_of_size_187_offset)
{
    test(100, 187, 33);
}

TEST(test_packetizer, packets_of_size_188_no_offset)
{
    test(100, 188, 0);
}

TEST(test_packetizer, packets_of_size_188_offset)
{
    test(100, 188, 33);
}

TEST(test_packetizer, packets_of_size_1490_no_offset)
{
    test(100, 1490, 0);
}

TEST(test_packetizer, packets_of_size_1490_offset)
{
    test(100, 1490, 33);
}
