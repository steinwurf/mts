// Copyright (c) Steinwurf ApS 2017.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include <cstdint>
#include <cassert>
#include <memory>
#include <bnb/stream_reader.hpp>

#include "helper.hpp"

namespace mts
{
class adaptation_field
{
public:

    static std::shared_ptr<adaptation_field> parse(
        bnb::stream_reader<endian::big_endian>& reader)
    {
        auto field = std::make_shared<mts::adaptation_field>();

        reader.read<endian::u8>(field->m_length);
        if (field->m_length == 0)
            return field;
        auto adaptation_field = reader.skip(field->m_length);

        adaptation_field
        .read_bits<endian::u8, bitter::msb0, 1, 1, 1, 1, 1, 1, 1, 1>()
        .get<0>(field->m_discontinuity_indicator)
        .get<1>(field->m_random_access_indicator)
        .get<2>(field->m_elementary_stream_priority_indicator)
        .get<3>(field->m_pcr_flag)
        .get<4>(field->m_opcr_flag)
        .get<5>(field->m_splicing_point_flag)
        .get<6>(field->m_transport_private_data_flag)
        .get<7>(field->m_adaptation_field_extension_flag);

        if (field->m_pcr_flag)
        {
            uint64_t pcr_base = 0;
            uint16_t pcr_extension = 0;
            adaptation_field.read_bits<endian::u48, bitter::msb0, 33, 6, 9>()
            .get<0>(pcr_base)
            .get<2>(pcr_extension);
            field->m_program_clock_reference = pcr_base * 300 + pcr_extension;
        }

        if (field->m_opcr_flag)
        {
            uint64_t opcr_base = 0;
            uint16_t opcr_extension = 0;

            adaptation_field.read_bits<endian::u48, bitter::msb0, 33, 6, 9>()
            .get<0>(opcr_base)
            .get<2>(opcr_extension);
            field->m_original_program_clock_reference =
                opcr_base * 300 + opcr_extension;
        }

        if (field->m_splicing_point_flag)
        {
            adaptation_field.read<endian::u8>(field->m_splice_countdown);
        }
        if (field->m_transport_private_data_flag)
        {
            adaptation_field.read<endian::u8>(
                field->m_transport_private_data_length);

            auto transport_private_data = adaptation_field.skip(
                field->m_transport_private_data_length);

            field->m_transport_private_data = transport_private_data.data();
        }
        if (field->m_adaptation_field_extension_flag)
        {
            uint8_t adaptation_field_extension_length = 0;
            adaptation_field.read<endian::u8>(
                adaptation_field_extension_length);

            auto adaptation_field_extension = adaptation_field.skip(
                adaptation_field_extension_length);

            adaptation_field_extension
            .read_bits<endian::u8, bitter::msb0, 1, 1, 1, 5>()
            .get<0>(field->m_ltw_flag)
            .get<1>(field->m_piecewise_rate_flag)
            .get<2>(field->m_seamless_splice_flag);

            if (field->m_ltw_flag)
            {
                adaptation_field_extension
                .read_bits<endian::u16, bitter::msb0, 1, 15>()
                .get<0>(field->m_ltw_valid_flag)
                .get<1>(field->m_ltw_offset);
            }

            if (field->m_piecewise_rate_flag)
            {
                adaptation_field_extension
                .read_bits<endian::u24, bitter::msb0, 2, 22>()
                .get<1>(field->m_piecewise_rate);
            }

            if (field->m_seamless_splice_flag)
            {

                uint8_t dts_next_au_32_30 = 0;
                uint16_t dts_next_au_29_15 = 0;
                uint16_t dts_next_au_14_0 = 0;
                adaptation_field_extension
                .read_bits<endian::u40, bitter::msb0, 4, 3, 1, 15, 1, 15, 1>()
                .get<0>(field->m_splice_type)
                .get<1>(dts_next_au_32_30)
                .get<3>(dts_next_au_29_15)
                .get<5>(dts_next_au_14_0);
                field->m_dts_next_au = helper::read_timestamp(
                    dts_next_au_32_30,
                    dts_next_au_29_15,
                    dts_next_au_14_0);
            }
        }

        if (reader.error())
        {
            return nullptr;
        }

        return field;
    }

public:

    uint8_t length() const
    {
        return m_length;
    }

    bool discontinuity_indicator() const
    {
        assert(m_length != 0);
        return m_discontinuity_indicator;
    }

    bool random_access_indicator() const
    {
        assert(m_length != 0);
        return m_random_access_indicator;
    }

    bool elementary_stream_priority_indicator() const
    {
        assert(m_length != 0);
        return m_elementary_stream_priority_indicator;
    }

    bool pcr_flag() const
    {
        assert(m_length != 0);
        return m_pcr_flag;
    }

    bool opcr_flag() const
    {
        assert(m_length != 0);
        return m_opcr_flag;
    }

    bool splicing_point_flag() const
    {
        assert(m_length != 0);
        return m_splicing_point_flag;
    }

    bool transport_private_data_flag() const
    {
        assert(m_length != 0);
        return m_transport_private_data_flag;
    }

    bool adaptation_field_extension_flag() const
    {
        assert(m_length != 0);
        return m_adaptation_field_extension_flag;
    }

    uint64_t program_clock_reference() const
    {
        assert(pcr_flag());
        return m_program_clock_reference;
    }

    uint64_t original_program_clock_reference() const
    {
        assert(opcr_flag());
        return m_original_program_clock_reference;
    }

    uint8_t splice_countdown() const
    {
        assert(splicing_point_flag());
        return m_splice_countdown;
    }

    const uint8_t* transport_private_data() const
    {
        assert(transport_private_data_flag());
        return m_transport_private_data;
    }

    uint8_t transport_private_data_length() const
    {
        assert(transport_private_data_flag());
        return m_transport_private_data_length;
    }

    bool ltw_flag() const
    {
        assert(adaptation_field_extension_flag());
        return m_ltw_flag;
    }

    bool piecewise_rate_flag() const
    {
        assert(adaptation_field_extension_flag());
        return m_piecewise_rate_flag;
    }

    bool seamless_splice_flag() const
    {
        assert(adaptation_field_extension_flag());
        return m_seamless_splice_flag;
    }

    bool ltw_valid_flag() const
    {
        assert(adaptation_field_extension_flag());
        assert(ltw_flag());
        return m_ltw_valid_flag;
    }

    uint16_t ltw_offset() const
    {
        assert(adaptation_field_extension_flag());
        assert(ltw_flag());
        return m_ltw_offset;
    }

    uint32_t piecewise_rate() const
    {
        assert(adaptation_field_extension_flag());
        assert(piecewise_rate_flag());
        return m_piecewise_rate;
    }

    uint8_t splice_type() const
    {
        assert(adaptation_field_extension_flag());
        assert(seamless_splice_flag());
        return m_splice_type;
    }

    uint64_t dts_next_au() const
    {
        assert(adaptation_field_extension_flag());
        assert(seamless_splice_flag());
        return m_dts_next_au;
    }

private:

    uint8_t m_length = 0;
    bool m_discontinuity_indicator = false;
    bool m_random_access_indicator = false;
    bool m_elementary_stream_priority_indicator = false;
    bool m_pcr_flag = false;
    bool m_opcr_flag = false;
    bool m_splicing_point_flag = false;
    bool m_transport_private_data_flag = false;
    bool m_adaptation_field_extension_flag = false;

    uint64_t m_program_clock_reference = 0;

    uint64_t m_original_program_clock_reference = 0;

    uint8_t m_splice_countdown = 0;

    uint8_t m_transport_private_data_length = 0;
    const uint8_t* m_transport_private_data = nullptr;

    bool m_ltw_flag = false;
    bool m_piecewise_rate_flag = false;
    bool m_seamless_splice_flag = false;

    bool m_ltw_valid_flag = false;
    uint16_t m_ltw_offset = 0;

    uint32_t m_piecewise_rate = 0;

    uint8_t m_splice_type = 0;
    uint64_t m_dts_next_au = 0;
};
}
