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

        reader.read<endian::u8>().expect_eq(0x47); // sýnc byte

        reader.read_bits<endian::u16, bitter::msb0, 1, 1, 1, 13>()
            .read<0>(ts_packet->m_transport_error_indicator).expect_eq(false)
            .read<1>(ts_packet->m_payload_unit_start_indicator)
            .read<2>(ts_packet->m_transport_priority)
            .read<3>(ts_packet->m_pid);
        reader.read_bits<endian::u8, bitter::msb0, 2, 2, 4>()
            .read<0>(ts_packet->m_transport_scrambling_control)
            .read<1>(ts_packet->m_adaptation_field_control)
            .read<2>(ts_packet->m_continuity_counter);

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

    bool m_transport_error_indicator;
    bool m_payload_unit_start_indicator;
    bool m_transport_priority;
    uint16_t m_pid;
    uint8_t m_transport_scrambling_control;
    uint8_t m_adaptation_field_control;
    uint8_t m_continuity_counter;

    std::shared_ptr<mts::adaptation_field> m_adaptation_field;
};
}
