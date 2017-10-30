// Copyright (c) Steinwurf ApS 2017.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include <cstdint>
#include <cassert>
#include <memory>

#include <endian/big_endian.hpp>
#include <bnb/stream_reader.hpp>

#include "adaptation_field.hpp"

namespace mts
{
class ts_packet
{
public:

    static std::shared_ptr<ts_packet> parse(
        bnb::stream_reader<endian::big_endian>& reader)
    {
        auto ts_packet = std::make_shared<mts::ts_packet>();

        reader.read_bytes<1>().expect_eq(0x47); // sýnc byte

        reader.read_bits<bitter::u16, bitter::msb0, 1, 1, 1, 13>()
        .get<0>(ts_packet->m_transport_error_indicator).expect_eq(false)
        .get<1>(ts_packet->m_payload_unit_start_indicator)
        .get<2>(ts_packet->m_transport_priority)
        .get<3>(ts_packet->m_pid);
        reader.read_bits<bitter::u8, bitter::msb0, 2, 2, 4>()
        .get<0>(ts_packet->m_transport_scrambling_control)
        .get<1>(ts_packet->m_adaptation_field_control)
        .get<2>(ts_packet->m_continuity_counter);

        if (reader.error())
        {
            return nullptr;
        }

        if (ts_packet->has_adaptation_field())
        {
            auto adaptation_field = adaptation_field::parse(reader);
            ts_packet->m_adaptation_field = std::move(adaptation_field);
        }

        return ts_packet;
    }

public:

    bool is_null_packet() const
    {
        return m_pid == 0x1FFF;
    }

    bool has_adaptation_field() const
    {
        return m_adaptation_field_control == 2 ||
               m_adaptation_field_control == 3;
    }

    bool has_payload_field() const
    {
        return m_adaptation_field_control == 1 ||
               m_adaptation_field_control == 3;
    }

    bool transport_error_indicator() const
    {
        return m_transport_error_indicator;
    }

    bool payload_unit_start_indicator() const
    {
        return m_payload_unit_start_indicator;
    }

    bool transport_priority() const
    {
        return m_transport_priority;
    }

    uint16_t pid() const
    {
        return m_pid;
    }

    uint8_t transport_scrambling_control() const
    {
        return m_transport_scrambling_control;
    }

    uint8_t continuity_counter() const
    {
        return m_continuity_counter;
    }

    mts::adaptation_field adaptation_field() const
    {
        assert(has_adaptation_field());
        assert(m_adaptation_field != nullptr);
        return *m_adaptation_field;
    }

private:

    bool m_transport_error_indicator = false;
    bool m_payload_unit_start_indicator = false;
    bool m_transport_priority = false;
    uint16_t m_pid = 0;
    uint8_t m_transport_scrambling_control = 0;
    uint8_t m_adaptation_field_control = 0;
    uint8_t m_continuity_counter = 0;

    std::shared_ptr<mts::adaptation_field> m_adaptation_field;
};
}
