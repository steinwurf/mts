// Copyright (c) Steinwurf ApS 2017.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include <cstdint>
#include <cassert>
#include <vector>
#include <memory>

#include <endian/big_endian.hpp>
#include <bnb/stream_reader.hpp>

#include "helper.hpp"
#include "stream_type.hpp"

namespace mts
{

// packetized elementary stream
class pes
{
public:

    static std::shared_ptr<pes> parse(
        const uint8_t* data, uint32_t size, std::error_code& error)
    {
        bnb::stream_reader<endian::big_endian> reader(data, size, error);
        auto pes = std::make_shared<mts::pes>();

        reader.read<endian::u24>(pes->m_packet_start_code_prefix);
        reader.read<endian::u8>(pes->m_stream_id);

        uint16_t packet_length;
        reader.read<endian::u16>(packet_length);

        if (reader.error())
            return nullptr;

        if (packet_length == 0)
            packet_length = reader.remaining_size();

        auto packet_reader = reader.skip(packet_length);
        if (pes->m_stream_id != 0xbc && // program_stream_map
            pes->m_stream_id != 0xbe && // padding_stream
            pes->m_stream_id != 0xbf && // private_stream_2
            pes->m_stream_id != 0xf0 && // ECM
            pes->m_stream_id != 0xf1 && // EMM
            pes->m_stream_id != 0xff && // program_stream_directory
            pes->m_stream_id != 0xf2 && // DSMCC
            pes->m_stream_id != 0xf8) // H.222.1 type E
        {
            packet_reader
                .read_bits<endian::u8, bitter::msb0, 2, 2, 1, 1, 1, 1>()
                    .read<0>().expect_eq(0x02)
                    .read<1>(pes->m_scrambling_control)
                    .read<2>(pes->m_priority)
                    .read<3>(pes->m_data_alignment_indicator)
                    .read<4>(pes->m_copyright)
                    .read<5>(pes->m_original_or_copy);

            packet_reader
                .read_bits<endian::u8, bitter::msb0, 2, 1, 1, 1, 1, 1, 1>()
                    .read<0>(pes->m_pts_dts_flags).expect_ne(0x01)
                    .read<1>(pes->m_escr_flag)
                    .read<2>(pes->m_es_rate_flag)
                    .read<3>(pes->m_dsm_trick_mode_flag)
                    .read<4>(pes->m_additional_copy_info_flag)
                    .read<5>(pes->m_crc_flag)
                    .read<6>(pes->m_extension_flag);

            uint8_t header_data_length = 0;
            packet_reader.read<endian::u8>(header_data_length);
            auto header_reader = packet_reader.skip(header_data_length);

            if (pes->has_presentation_timestamp())
            {
                uint8_t ts_32_30 = 0;
                uint16_t ts_29_15 = 0;
                uint16_t ts_14_0 = 0;

                header_reader
                    .read_bits<endian::u40, bitter::msb0, 4, 3, 1, 15, 1, 15, 1>()
                    .read<1>(ts_32_30)
                    .read<3>(ts_29_15)
                    .read<5>(ts_14_0);

                pes->m_pts = helper::read_timestamp(ts_32_30, ts_29_15, ts_14_0);
            }
            if (pes->has_decoding_timestamp())
            {
                uint8_t ts_32_30 = 0;
                uint16_t ts_29_15 = 0;
                uint16_t ts_14_0 = 0;

                header_reader
                    .read_bits<endian::u40, bitter::msb0, 4, 3, 1, 15, 1, 15, 1>()
                    .read<1>(ts_32_30)
                    .read<3>(ts_29_15)
                    .read<5>(ts_14_0);

                pes->m_dts = helper::read_timestamp(ts_32_30, ts_29_15, ts_14_0);
            }

            if (pes->m_escr_flag)
            {
                uint8_t escr_32_30 = 0;
                uint16_t escr_29_15 = 0;
                uint16_t escr_14_0 = 0;
                uint16_t escr_base = 0;

                header_reader
                    .read_bits<endian::u48, bitter::msb0, 2, 3, 1, 15, 1, 15, 1, 9, 1>()
                    .read<1>(escr_32_30)
                    .read<3>(escr_29_15)
                    .read<5>(escr_14_0)
                    .read<7>(escr_base);

                pes->m_escr = helper::read_timestamp(
                    escr_32_30, escr_29_15, escr_14_0) * 300 + escr_base;
            }

            if (pes->m_es_rate_flag)
            {
                header_reader
                    .read_bits<endian::u24, bitter::msb0, 1, 22, 1>()
                    .read<1>(pes->m_es_rate);
            }

            if (pes->m_dsm_trick_mode_flag)
            {
                header_reader
                    .read_bits<endian::u8, bitter::msb0, 3, 5>()
                    .read<0>(pes->m_trick_mode_control)
                    .read<1>(pes->m_trick_mode_data);
            }

            if (pes->m_additional_copy_info_flag)
            {
                header_reader
                    .read_bits<endian::u8, bitter::msb0, 1, 7>()
                    .read<1>(pes->m_additional_copy_info);
            }

            if (pes->m_crc_flag)
            {
                header_reader.read<endian::u16>(pes->m_previous_crc);
            }
        }

        if (reader.error())
            return nullptr;

        pes->m_payload_data = packet_reader.remaining_data();
        pes->m_payload_size = packet_reader.remaining_size();

        return pes;
    }

public:

    bool has_presentation_timestamp() const
    {
        return (m_pts_dts_flags & 0x02) == 0x02;
    }

    bool has_decoding_timestamp() const
    {
        return m_pts_dts_flags == 0x03;
    }


    bool has_elementary_stream_clock_reference() const
    {
        return m_escr_flag;
    }

    uint32_t packet_start_code_prefix() const
    {
        return m_packet_start_code_prefix;
    }

    uint8_t stream_id() const
    {
        return m_stream_id;
    }

    uint8_t scrambling_control() const
    {
        return m_scrambling_control;
    }

    uint8_t priority() const
    {
        return m_priority;
    }

    bool data_alignment_indicator() const
    {
        return m_data_alignment_indicator;
    }

    bool copyright() const
    {
        return m_copyright;
    }

    bool original_or_copy() const
    {
        return m_original_or_copy;
    }

    bool has_es_rate() const
    {
        return m_es_rate_flag;
    }

    bool has_dsm_trick_mode() const
    {
        return m_dsm_trick_mode_flag;
    }

    bool has_additional_copy_info() const
    {
        return m_additional_copy_info_flag;
    }

    bool has_previous_crc() const
    {
        return m_crc_flag;
    }

    bool has_extension() const
    {
        return m_extension_flag;
    }

    uint64_t presentation_timestamp() const
    {
        assert(has_presentation_timestamp());
        return m_pts;
    }

    uint64_t decoding_timestamp() const
    {
        assert(has_decoding_timestamp());
        return m_dts;
    }

    uint64_t elementary_stream_clock_reference() const
    {
        assert(has_elementary_stream_clock_reference());
        return m_escr;
    }

    uint32_t es_rate() const
    {
        assert(has_es_rate());
        return m_es_rate;
    }

    uint8_t trick_mode_control() const
    {
        assert(has_dsm_trick_mode());
        return m_trick_mode_control;
    }

    uint8_t trick_mode_data() const
    {
        assert(has_dsm_trick_mode());
        return m_trick_mode_data;
    }

    uint8_t additional_copy_info() const
    {
        assert(has_additional_copy_info());
        return m_additional_copy_info;
    }

    uint16_t previous_crc() const
    {
        assert(has_previous_crc());
        return m_previous_crc;
    }

    const uint8_t* payload_data() const
    {
        return m_payload_data;
    }

    uint32_t payload_size() const
    {
        return m_payload_size;
    }

private:

    uint32_t m_packet_start_code_prefix;
    uint8_t m_stream_id;

    uint8_t m_scrambling_control;
    uint8_t m_priority;
    bool m_data_alignment_indicator;
    bool m_copyright;
    bool m_original_or_copy;
    uint8_t m_pts_dts_flags;
    bool m_escr_flag;
    bool m_es_rate_flag;
    bool m_dsm_trick_mode_flag;
    bool m_additional_copy_info_flag;
    bool m_crc_flag;
    bool m_extension_flag;

    uint64_t m_pts;
    uint64_t m_dts;
    uint64_t m_escr;

    uint32_t m_es_rate;

    uint8_t m_trick_mode_control;
    uint8_t m_trick_mode_data;

    uint8_t m_additional_copy_info;

    uint16_t m_previous_crc;

    const uint8_t* m_payload_data;
    uint32_t m_payload_size;
};
}