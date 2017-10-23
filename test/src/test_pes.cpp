// Copyright (c) Steinwurf ApS 2017.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <mts/pes.hpp>
#include <mts/parser.hpp>

#include <fstream>

#include <gtest/gtest.h>

TEST(test_pes, test_pes_parsing)
{
    auto filename = "test.ts";
    std::ifstream file(filename, std::ios::binary|std::ios::ate);
    ASSERT_TRUE(file.is_open());
    auto size = file.tellg();

    file.seekg(0, std::ios::beg);
    auto packet_size = 188;
    mts::parser parser(packet_size);
    std::vector<uint8_t> packet(packet_size);
    ASSERT_EQ(0U, size % packet.size());

    uint32_t pes_index = 0;
    for (uint32_t i = 0; i < size / packet.size(); ++i)
    {
        file.read((char*)packet.data(), packet.size());
        std::error_code error;
        parser.read(packet.data(), error);
        ASSERT_FALSE((bool) error);
        if (parser.has_pes())
        {
            auto& pes_data = parser.pes_data();
            auto pes = mts::pes::parse(pes_data.data(), pes_data.size(), error);
            EXPECT_FALSE((bool)error) << pes_index;
            pes_index++;
        }
    }
    EXPECT_EQ(198U, pes_index);
}

TEST(test_pes, pes_dump)
{
    auto filename = "pes_dump";
    std::ifstream file(filename, std::ios::binary|std::ios::ate);
    ASSERT_TRUE(file.is_open());
    auto size = file.tellg();

    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    file.read((char*)buffer.data(), buffer.size());

    std::error_code error;
    auto pes = mts::pes::parse(buffer.data(), buffer.size(), error);
    ASSERT_TRUE(!error);

    EXPECT_TRUE(pes->has_presentation_timestamp());
    EXPECT_TRUE(pes->has_decoding_timestamp());
    EXPECT_FALSE(pes->has_elementary_stream_clock_reference());
    EXPECT_EQ(1U, pes->packet_start_code_prefix());
    EXPECT_EQ(224U, pes->stream_id());
    EXPECT_EQ(0U, pes->scrambling_control());
    EXPECT_EQ(1U, pes->priority());
    EXPECT_TRUE(pes->data_alignment_indicator());
    EXPECT_FALSE(pes->copyright());
    EXPECT_EQ(0U, pes->original_or_copy());
    EXPECT_FALSE(pes->has_es_rate());
    EXPECT_FALSE(pes->has_dsm_trick_mode());
    EXPECT_FALSE(pes->has_additional_copy_info());
    EXPECT_FALSE(pes->has_previous_crc());
    EXPECT_FALSE(pes->has_extension());
    EXPECT_EQ(1483615137U, pes->presentation_timestamp());
    EXPECT_EQ(1483600122U, pes->decoding_timestamp());

    EXPECT_NE(nullptr, pes->payload_data());
    EXPECT_EQ(3017U, pes->payload_size());
}
