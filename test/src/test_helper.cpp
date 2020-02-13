// Copyright (c) Steinwurf ApS 2017.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#include <mts/helper.hpp>
#include <gtest/gtest.h>

TEST(test_helper, read_timestamp)
{
    uint8_t ts_32_30 = 1;
    uint16_t ts_29_15 = 2;
    uint16_t ts_14_0 = 3;
    auto ts = mts::helper::read_timestamp(ts_32_30, ts_29_15, ts_14_0);
    EXPECT_EQ(1073807363U, ts);
}

TEST(test_helper, continuity_loss_calculation)
{
    EXPECT_EQ(0U, mts::helper::continuity_loss_calculation(0, 0));
    EXPECT_EQ(0U, mts::helper::continuity_loss_calculation(1, 1));
    EXPECT_EQ(0U, mts::helper::continuity_loss_calculation(2, 2));
    EXPECT_EQ(0U, mts::helper::continuity_loss_calculation(3, 3));
    EXPECT_EQ(0U, mts::helper::continuity_loss_calculation(4, 4));
    EXPECT_EQ(0U, mts::helper::continuity_loss_calculation(5, 5));
    EXPECT_EQ(0U, mts::helper::continuity_loss_calculation(6, 6));
    EXPECT_EQ(0U, mts::helper::continuity_loss_calculation(7, 7));
    EXPECT_EQ(0U, mts::helper::continuity_loss_calculation(8, 8));
    EXPECT_EQ(0U, mts::helper::continuity_loss_calculation(9, 9));
    EXPECT_EQ(0U, mts::helper::continuity_loss_calculation(10, 10));
    EXPECT_EQ(0U, mts::helper::continuity_loss_calculation(11, 11));
    EXPECT_EQ(0U, mts::helper::continuity_loss_calculation(12, 12));
    EXPECT_EQ(0U, mts::helper::continuity_loss_calculation(13, 13));
    EXPECT_EQ(0U, mts::helper::continuity_loss_calculation(14, 14));
    EXPECT_EQ(0U, mts::helper::continuity_loss_calculation(15, 15));

    EXPECT_EQ(1U, mts::helper::continuity_loss_calculation(0, 1));
    EXPECT_EQ(1U, mts::helper::continuity_loss_calculation(1, 2));
    EXPECT_EQ(1U, mts::helper::continuity_loss_calculation(2, 3));
    EXPECT_EQ(1U, mts::helper::continuity_loss_calculation(3, 4));
    EXPECT_EQ(1U, mts::helper::continuity_loss_calculation(4, 5));
    EXPECT_EQ(1U, mts::helper::continuity_loss_calculation(5, 6));
    EXPECT_EQ(1U, mts::helper::continuity_loss_calculation(6, 7));
    EXPECT_EQ(1U, mts::helper::continuity_loss_calculation(7, 8));
    EXPECT_EQ(1U, mts::helper::continuity_loss_calculation(8, 9));
    EXPECT_EQ(1U, mts::helper::continuity_loss_calculation(9, 10));
    EXPECT_EQ(1U, mts::helper::continuity_loss_calculation(10, 11));
    EXPECT_EQ(1U, mts::helper::continuity_loss_calculation(11, 12));
    EXPECT_EQ(1U, mts::helper::continuity_loss_calculation(12, 13));
    EXPECT_EQ(1U, mts::helper::continuity_loss_calculation(13, 14));
    EXPECT_EQ(1U, mts::helper::continuity_loss_calculation(14, 15));
    EXPECT_EQ(1U, mts::helper::continuity_loss_calculation(15, 0));

    EXPECT_EQ(2U, mts::helper::continuity_loss_calculation(15, 1));
    EXPECT_EQ(3U, mts::helper::continuity_loss_calculation(15, 2));
    EXPECT_EQ(4U, mts::helper::continuity_loss_calculation(15, 3));
}
