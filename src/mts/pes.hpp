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
#include <boost/optional.hpp>

#include "helper.hpp"
#include "stream_type.hpp"

namespace mts
{

// packetized elementary stream
class pes
{
public:

    static boost::optional<pes> parse(
        const uint8_t* data, uint64_t size, std::error_code& error)
    {
        bnb::stream_reader<endian::big_endian> reader(data, size, error);
        return parse(reader);
    }

    static boost::optional<pes> parse(
        bnb::stream_reader<endian::big_endian>& reader)
    {
        mts::pes pes;

        reader.read_bytes<3>(pes.m_packet_start_code_prefix);
        reader.read_bytes<1>(pes.m_stream_id);

        uint16_t read_packet_length = 0;
        reader.read_bytes<2>(read_packet_length);

        if (reader.error())
            return boost::none;

        uint64_t bytes_to_skip = read_packet_length;
        // A value of 0 indicates that the PES packet length is neither
        // specified nor bounded and is allowed only in PES packets whose
        // payload consists of bytes from a video elementary stream contained
        // in transport stream packets.
        //
        // From ISO/IEC 13818-1:2013 p. 35
        if (bytes_to_skip == 0)
        {
            bytes_to_skip = reader.remaining_size();
        }

        auto packet_reader = reader.skip(bytes_to_skip);
        if (pes.m_stream_id != 0xbc && // program_stream_map
            pes.m_stream_id != 0xbe && // padding_stream
            pes.m_stream_id != 0xbf && // private_stream_2
            pes.m_stream_id != 0xf0 && // ECM
            pes.m_stream_id != 0xf1 && // EMM
            pes.m_stream_id != 0xff && // program_stream_directory
            pes.m_stream_id != 0xf2 && // DSMCC
            pes.m_stream_id != 0xf8) // H.222.1 type E
        {
            packet_reader
            .read_bits<bitter::u8, bitter::msb0, 2, 2, 1, 1, 1, 1>()
            .get<0>().expect_eq(0x02)
            .get<1>(pes.m_scrambling_control)
            .get<2>(pes.m_priority)
            .get<3>(pes.m_data_alignment_indicator)
            .get<4>(pes.m_copyright)
            .get<5>(pes.m_original_or_copy);

            packet_reader
            .read_bits<bitter::u8, bitter::msb0, 2, 1, 1, 1, 1, 1, 1>()
            .get<0>(pes.m_pts_dts_flags).expect_ne(0x01)
            .get<1>(pes.m_escr_flag)
            .get<2>(pes.m_es_rate_flag)
            .get<3>(pes.m_dsm_trick_mode_flag)
            .get<4>(pes.m_additional_copy_info_flag)
            .get<5>(pes.m_crc_flag)
            .get<6>(pes.m_extension_flag);

            uint8_t header_data_length = 0;
            packet_reader.read_bytes<1>(header_data_length);
            auto header_reader = packet_reader.skip(header_data_length);

            if (pes.has_presentation_timestamp())
            {
                uint8_t ts_32_30 = 0;
                uint16_t ts_29_15 = 0;
                uint16_t ts_14_0 = 0;

                header_reader
                .read_bits<bitter::u40, bitter::msb0, 4, 3, 1, 15, 1, 15, 1>()
                .get<1>(ts_32_30)
                .get<3>(ts_29_15)
                .get<5>(ts_14_0);

                pes.m_pts = helper::read_timestamp(ts_32_30, ts_29_15, ts_14_0);
            }
            if (pes.has_decoding_timestamp())
            {
                uint8_t ts_32_30 = 0;
                uint16_t ts_29_15 = 0;
                uint16_t ts_14_0 = 0;

                header_reader
                .read_bits<bitter::u40, bitter::msb0, 4, 3, 1, 15, 1, 15, 1>()
                .get<1>(ts_32_30)
                .get<3>(ts_29_15)
                .get<5>(ts_14_0);

                pes.m_dts = helper::read_timestamp(ts_32_30, ts_29_15, ts_14_0);
            }

            if (pes.m_escr_flag)
            {
                uint8_t escr_32_30 = 0;
                uint16_t escr_29_15 = 0;
                uint16_t escr_14_0 = 0;
                uint16_t escr_base = 0;

                header_reader
                .read_bits<bitter::u48, bitter::msb0, 2, 3, 1, 15, 1, 15, 1, 9, 1>()
                .get<1>(escr_32_30)
                .get<3>(escr_29_15)
                .get<5>(escr_14_0)
                .get<7>(escr_base);

                pes.m_escr = helper::read_timestamp(
                    escr_32_30, escr_29_15, escr_14_0) * 300 + escr_base;
            }

            if (pes.m_es_rate_flag)
            {
                header_reader
                .read_bits<bitter::u24, bitter::msb0, 1, 22, 1>()
                .get<1>(pes.m_es_rate);
            }

            if (pes.m_dsm_trick_mode_flag)
            {
                header_reader
                .read_bits<bitter::u8, bitter::msb0, 3, 5>()
                .get<0>(pes.m_trick_mode_control)
                .get<1>(pes.m_trick_mode_data);
            }

            if (pes.m_additional_copy_info_flag)
            {
                header_reader
                .read_bits<bitter::u8, bitter::msb0, 1, 7>()
                .get<1>(pes.m_additional_copy_info);
            }

            if (pes.m_crc_flag)
            {
                header_reader.read_bytes<2>(pes.m_previous_crc);
            }
        }

        if (reader.error())
            return boost::none;

        pes.m_payload_data = packet_reader.remaining_data();
        pes.m_payload_size = packet_reader.remaining_size();

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

    uint32_t m_packet_start_code_prefix = 0;
    uint8_t m_stream_id = 0;

    uint8_t m_scrambling_control = 0;
    uint8_t m_priority = 0;
    bool m_data_alignment_indicator = false;
    bool m_copyright = false;
    bool m_original_or_copy = false;
    uint8_t m_pts_dts_flags = 0;
    bool m_escr_flag = false;
    bool m_es_rate_flag = false;
    bool m_dsm_trick_mode_flag = false;
    bool m_additional_copy_info_flag = false;
    bool m_crc_flag = false;
    bool m_extension_flag = false;

    uint64_t m_pts = 0;
    uint64_t m_dts = 0;
    uint64_t m_escr = 0;

    uint32_t m_es_rate = 0;

    uint8_t m_trick_mode_control = 0;
    uint8_t m_trick_mode_data = 0;

    uint8_t m_additional_copy_info = 0;

    uint16_t m_previous_crc = 0;

    const uint8_t* m_payload_data = nullptr;
    uint32_t m_payload_size = 0;
};
}