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

namespace mts
{
/// program association table
class pat
{
private:

    class program_entry
    {
    public:

        friend class pat;

        uint16_t program_number() const
        {
            return m_program_number;
        }

        uint16_t pid() const
        {
            return m_pid;
        }

        bool is_network_pid() const
        {
            return m_program_number == 0;
        }

    private:

        uint16_t m_program_number = 0;
        uint16_t m_pid = 0;
    };

public:

    static std::shared_ptr<pat> parse(
        const uint8_t* data, uint64_t size, std::error_code& error)
    {
        bnb::stream_reader<endian::big_endian> reader(
            data, size, error);
        return parse(reader);
    }

    static std::shared_ptr<pat> parse(
        bnb::stream_reader<endian::big_endian>& reader)
    {
        auto pat = std::make_shared<mts::pat>();

        reader.read_bytes<1>(pat->m_table_id);

        uint16_t section_length = 0;
        reader.read_bits<bitter::u16, bitter::msb0, 1, 1, 2, 12>()
        .get<0>(pat->m_section_syntax_indicator)
        .get<3>(section_length);

        auto section_reader = reader.skip(section_length);

        section_reader.read_bytes<2>(pat->m_transport_stream_id);
        section_reader.read_bits<bitter::u8, bitter::msb0, 2, 5, 1>()
        .get<1>(pat->m_version_number)
        .get<2>(pat->m_current_next_indicator);

        section_reader.read_bytes<1>(pat->m_section_number);
        section_reader.read_bytes<1>(pat->m_last_section_number);

        while (
            !section_reader.error() &&
            section_reader.remaining_size() > sizeof(pat->m_crc))
        {
            program_entry program;
            auto program_reader = section_reader.skip(4);
            program_reader.read_bytes<2>(program.m_program_number);
            program_reader.read_bits<bitter::u16, bitter::msb0, 3, 13>()
            .get<1>(program.m_pid);
            pat->m_program_entries.push_back(program);
        }

        section_reader.read_bytes<4>(pat->m_crc);

        if (section_reader.error())
        {
            return nullptr;
        }
        return pat;
    }

public:

    uint8_t table_id() const
    {
        return m_table_id;
    }

    bool section_syntax_indicator() const
    {
        return m_section_syntax_indicator;
    }

    uint16_t transport_stream_id() const
    {
        return m_transport_stream_id;
    }

    uint8_t version_number() const
    {
        return m_version_number;
    }

    bool current_next_indicator() const
    {
        return m_current_next_indicator;
    }

    uint8_t section_number() const
    {
        return m_section_number;
    }

    uint8_t last_section_number() const
    {
        return m_last_section_number;
    }

    const std::vector<program_entry>& program_entries() const
    {
        return m_program_entries;
    }

    uint32_t crc() const
    {
        return m_crc;
    }

private:

    uint8_t m_table_id = 0;
    bool m_section_syntax_indicator = false;
    uint16_t m_transport_stream_id = 0;
    uint8_t m_version_number = 0;
    bool m_current_next_indicator = false;
    uint8_t m_section_number = 0;
    uint8_t m_last_section_number = 0;
    std::vector<program_entry> m_program_entries;
    uint32_t m_crc = 0;
};
}