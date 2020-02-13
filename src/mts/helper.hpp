// Copyright (c) Steinwurf ApS 2017.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include <cstdint>
#include <bitter/msb0_writer.hpp>

namespace mts
{
struct helper
{
    static uint64_t read_timestamp(
        uint8_t ts_32_30, uint16_t ts_29_15, uint16_t ts_14_0)
    {
        auto writer = bitter::msb0_writer<bitter::u40, 7, 3, 15, 15>();
        writer.field<1>(ts_32_30);
        writer.field<2>(ts_29_15);
        writer.field<3>(ts_14_0);
        return writer.data();
    }

    static uint8_t continuity_loss_calculation(uint8_t expected, uint8_t actual)
    {
        assert(actual < 16U);
        assert(expected < 16U);
        if (actual == expected) return 0U;
        if (actual < expected) actual += 16U;
        assert(actual > expected);
        return actual - expected;
    }
};
}