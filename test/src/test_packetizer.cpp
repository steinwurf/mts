// Copyright (c) Steinwurf ApS 2017.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <mts/packetizer.hpp>

#include <gtest/gtest.h>

namespace
{
std::vector<uint8_t> generate_ts_packets(uint32_t ts_packets, uint32_t offset)
{
    assert(offset < 188U);

    std::vector<uint8_t> buffer(ts_packets * 188);

    // Create data.
    // auto size = offset == 0 ? 188 : offset;
    for (uint32_t i = 0; i < ts_packets; i += 1)
    {
        for (uint32_t j = 0; j < 188; j += 1)
        {
            // To make it possible to distinguish which packet the data is
            // associated with, the content of the is the index of the packet.
            // Unless it's a coincidental sync byte,
            uint8_t c = rand();
            if (c != 0x47)
                c  = i + 1;

            buffer[(i * 188) + j] = c;
        }
    }
    // Add offset data. this can be random.
    buffer.insert(buffer.begin(), offset, 0);
    std::generate(buffer.begin(), buffer.begin() + offset, rand);

    // Add sync bytes
    for (uint32_t i = offset; i < buffer.size(); i += 188)
    {
        buffer[i] = 0x47;
    }

    return buffer;
}

void test(uint32_t ts_packets, uint32_t udp_packet_size, uint32_t data_offset)
{
    std::vector<std::vector<uint8_t>> results;
    mts::packetizer packetizer([&results](auto data, auto size)
    {
        EXPECT_EQ(mts::packetizer::sync_byte(), data[0]);
        results.push_back({data, data + size});
    });

    // Generate packets
    auto ts_data = generate_ts_packets(ts_packets, data_offset);

    // Calculate how many "udp" packets needed
    auto udp_packets = ts_data.size() / udp_packet_size;

    // Resize to only contain what we can send
    ts_data.resize(udp_packets * udp_packet_size);

    auto data_to_read = ts_data.data();
    for (uint32_t i = 0; i < udp_packets; i++)
    {
        packetizer.read(data_to_read, udp_packet_size);
        data_to_read += udp_packet_size;
    }

    ASSERT_LT(packetizer.buffered(), 188U);

    // shorten data to not include whatever the packetizer has buffered.
    std::vector<uint8_t> expected_data =
        {
            ts_data.begin() + data_offset, ts_data.end() - packetizer.buffered()
        };

    // Split the expected data up into packets
    std::vector<std::vector<uint8_t>> expected_packets;
    auto it = expected_data.cbegin();
    while (it != expected_data.cend())
    {
        expected_packets.push_back({it, it + 188});
        it += 188;
    }

    EXPECT_NE(0U, results.size());
    ASSERT_LE(results.size(), expected_packets.size());

    // offset the expected packets with the received packets.
    uint32_t packet_id = expected_packets.size() - results.size();
    bool sync = false;
    for (auto result : results)
    {
        auto& expected_packet = expected_packets.at(packet_id);
        if (!sync && expected_packet == result)
        {
            // We should not use more than 3 packets to get in sync.
            // This can happen but it should be very unlikely, as it requires
            // the sync bytes to be placed
            EXPECT_LE(packet_id, 3U);
            sync = true;
        }
        if (sync)
        {
            EXPECT_EQ(expected_packet.at(0), result.at(0));
            EXPECT_EQ(expected_packet.at(1), result.at(1));
            EXPECT_EQ(expected_packet.at(187), result.at(187));
            EXPECT_EQ(expected_packet, result);
        }
        packet_id += 1;
    }
    EXPECT_TRUE(sync);
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
