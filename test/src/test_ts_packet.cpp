// Copyright (c) Steinwurf ApS 2017.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <mts/ts_packet.hpp>

#include <fstream>

#include <gtest/gtest.h>

TEST(test_ts_packet, read_and_verify)
{
    auto filename = "test.ts";
    std::ifstream file(filename, std::ios::binary);
    std::vector<uint8_t> buffer(188);

    // Read the 3 first packets and check their content
    {
        file.read((char*)buffer.data(), buffer.size());
        std::error_code error;
        bnb::stream_reader<endian::big_endian> reader(
            buffer.data(), buffer.size(), error);
        auto packet = mts::ts_packet::parse(reader);
        ASSERT_FALSE((bool)error);
        ASSERT_NE(boost::none, packet);

        EXPECT_FALSE(packet->is_null_packet());
        EXPECT_FALSE(packet->has_adaptation_field());
        EXPECT_TRUE(packet->has_payload_field());
        EXPECT_FALSE(packet->transport_error_indicator());
        EXPECT_TRUE(packet->payload_unit_start_indicator());
        EXPECT_FALSE(packet->transport_priority());
        EXPECT_EQ(17U, packet->pid());
        EXPECT_EQ(0U, packet->transport_scrambling_control());
        EXPECT_EQ(0U, packet->continuity_counter());
    }
    {
        file.read((char*)buffer.data(), buffer.size());
        std::error_code error;
        bnb::stream_reader<endian::big_endian> reader(
            buffer.data(), buffer.size(), error);
        auto packet = mts::ts_packet::parse(reader);
        ASSERT_FALSE((bool)error);
        ASSERT_NE(boost::none, packet);

        EXPECT_FALSE(packet->is_null_packet());
        EXPECT_FALSE(packet->has_adaptation_field());
        EXPECT_TRUE(packet->has_payload_field());
        EXPECT_FALSE(packet->transport_error_indicator());
        EXPECT_TRUE(packet->payload_unit_start_indicator());
        EXPECT_FALSE(packet->transport_priority());
        EXPECT_EQ(0U, packet->pid());
        EXPECT_EQ(0U, packet->transport_scrambling_control());
        EXPECT_EQ(0U, packet->continuity_counter());
    }
    {
        file.read((char*)buffer.data(), buffer.size());
        std::error_code error;
        bnb::stream_reader<endian::big_endian> reader(
            buffer.data(), buffer.size(), error);
        auto packet = mts::ts_packet::parse(reader);
        ASSERT_FALSE((bool)error);
        ASSERT_NE(boost::none, packet);

        EXPECT_FALSE(packet->is_null_packet());
        EXPECT_FALSE(packet->has_adaptation_field());
        EXPECT_TRUE(packet->has_payload_field());
        EXPECT_FALSE(packet->transport_error_indicator());
        EXPECT_TRUE(packet->payload_unit_start_indicator());
        EXPECT_FALSE(packet->transport_priority());
        EXPECT_EQ(4096U, packet->pid());
        EXPECT_EQ(0U, packet->transport_scrambling_control());
        EXPECT_EQ(0U, packet->continuity_counter());
    }
}
