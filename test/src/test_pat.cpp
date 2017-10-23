// Copyright (c) Steinwurf ApS 2017.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <mts/pat.hpp>

#include <gtest/gtest.h>

TEST(test_pat, init)
{
    std::vector<uint8_t> buffer =
        {
            0x00, 0xb0, 0x15, 0x00, 0x02, 0xc1, 0x00, 0x00,
            0x00, 0x00, 0xe0, 0x10, 0x00, 0x01, 0xe0, 0x64,
            0x00, 0x02, 0xe0, 0xc8, 0x79, 0xd1, 0x50, 0xd6
        };
    std::error_code error;
    bnb::stream_reader<endian::big_endian> reader(
        buffer.data(), buffer.size(), error);

    auto pat = mts::pat::parse(reader);
    ASSERT_FALSE((bool)error);

    EXPECT_EQ(0U, pat->table_id());
    EXPECT_TRUE(pat->section_syntax_indicator());
    EXPECT_EQ(2U, pat->transport_stream_id());
    EXPECT_EQ(0U, pat->version_number());
    EXPECT_TRUE(pat->current_next_indicator());
    EXPECT_EQ(0U, pat->section_number());
    EXPECT_EQ(0U, pat->last_section_number());

    auto program_entries = pat->program_entries();
    ASSERT_EQ(3U, program_entries.size());

    std::vector<uint16_t> expected_program_numbers = { 0, 1, 2, 0 };
    std::vector<uint16_t> expected_program_pids = { 0x0010, 0x0064, 0x00c8, 0 };
    uint32_t i = 0;
    for (const auto& program_entry : program_entries)
    {
        auto expected_program_number = expected_program_numbers[i];
        EXPECT_EQ(expected_program_number, program_entry.program_number());

        auto expected_program_pid = expected_program_pids[i];
        EXPECT_EQ(expected_program_pid, program_entry.pid());
        EXPECT_EQ(i == 0, program_entry.is_network_pid());
        i++;
    }

    EXPECT_EQ(2043760854U, pat->crc());
}
