// Copyright (c) Steinwurf ApS 2017.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include <cstdint>
#include <vector>
#include <cassert>
#include <memory>

#include <endian/stream_reader.hpp>
#include <endian/big_endian.hpp>

namespace mts
{
class program
{
public:

    class stream_entry
    {
    public:

        struct es_info_entry
        {
            uint8_t m_tag = 0;
            const uint8_t* m_description_data = nullptr;
            uint8_t m_description_length = 0;
        };

    public:

        static std::shared_ptr<stream_entry> parse(
            bnb::stream_reader<endian::big_endian>& reader)
        {
            auto stream_entry = std::make_shared<mts::program::stream_entry>();
            reader.read<endian::u8>(stream_entry->m_type);

            reader.read_bits<endian::u16, bitter::msb0, 3, 13>()
                .read<1>(stream_entry->m_pid);

            uint16_t es_info_length = 0;
            reader.read_bits<endian::u16, bitter::msb0, 4, 2, 10>()
                .read<1>().expect_eq(0x00)
                .read<2>(es_info_length);

            auto es_info_reader = reader.skip(es_info_length);

            while (
                !es_info_reader.error() &&
                es_info_reader.remaining_size() > 0)
            {
                mts::program::stream_entry::es_info_entry es_info_entry;
                es_info_reader.read<endian::u8>(es_info_entry.m_tag);
                es_info_reader.read<endian::u8>(
                    es_info_entry.m_description_length);

                auto description = es_info_reader.skip(
                    es_info_entry.m_description_length);

                es_info_entry.m_description_data = description.data();
                stream_entry->m_es_info_entries.push_back(es_info_entry);
            }

            if (reader.error())
                return nullptr;

            return stream_entry;
        }

    public:

        uint8_t type() const
        {
            return m_type;
        }

        uint16_t pid() const
        {
            return m_pid;
        }

        const std::vector<es_info_entry>& es_info_entries() const
        {
            return m_es_info_entries;
        }

    private:

        uint8_t m_type = 0;
        uint16_t m_pid = 0;
        std::vector<es_info_entry> m_es_info_entries;
    };

public:

    static std::shared_ptr<program> parse(
        bnb::stream_reader<endian::big_endian>& reader)
    {
        auto program = std::make_shared<mts::program>();

        reader.read<endian::u8>(program->m_table_id);

        uint16_t section_length = 0;
        reader.read_bits<endian::u16, bitter::msb0, 1, 1, 2, 2, 10>()
            .read<0>(program->m_section_syntax_indicator)
            .read<3>().expect_eq(0x00)
            .read<4>(section_length);

        auto section_reader = reader.skip(section_length);

        section_reader.read<endian::u16>(program->m_program_number);

        section_reader.read_bits<endian::u8, bitter::msb0, 2, 5, 1>()
            .read<1>(program->m_version_number)
            .read<2>(program->m_current_next_indicator);

        section_reader.read<endian::u8>(program->m_section_number);
        section_reader.read<endian::u8>(program->m_last_section_number);

        section_reader.read_bits<endian::u16, bitter::msb0, 3, 13>()
            .read<1>(program->m_pcr_pid);

        section_reader.read_bits<endian::u16, bitter::msb0, 4, 2, 10>()
            .read<1>().expect_eq(0x00)
            .read<2>(program->m_program_info_length);

        auto program_info = section_reader.skip(program->m_program_info_length);
        program->m_program_info_data = program_info.data();

        while (
            !section_reader.error() &&
            section_reader.remaining_size() > sizeof(program->m_crc))
        {
            auto stream = stream_entry::parse(section_reader);
            if (section_reader.error())
                return nullptr;

            program->m_stream_entries.push_back(stream);
        }

        section_reader.read<endian::u32>(program->m_crc);

        if (reader.error())
            return nullptr;
        return program;
    }

public:

    const std::vector<std::shared_ptr<stream_entry>>& stream_entries() const
    {
        return m_stream_entries;
    }

     uint8_t table_id() const
     {
        return m_table_id;
     }

     bool section_syntax_indicator() const
     {
        return m_section_syntax_indicator;
     }

     uint16_t program_number() const
     {
        return m_program_number;
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

     uint16_t pcr_pid() const
     {
        return m_pcr_pid;
     }

     uint16_t program_info_length() const
     {
        return m_program_info_length;
     }

     const uint8_t* program_info_data() const
     {
        return m_program_info_data;
     }

private:

     uint8_t m_table_id = 0;
     bool m_section_syntax_indicator = false;
     uint16_t m_program_number = 0;
     uint8_t m_version_number = 0;
     bool m_current_next_indicator = false;
     uint8_t m_section_number = 0;
     uint8_t m_last_section_number = 0;
     uint16_t m_pcr_pid = 0;
     uint16_t m_program_info_length = 0;
     const uint8_t* m_program_info_data = nullptr;

    std::vector<std::shared_ptr<stream_entry>> m_stream_entries;

    uint32_t m_crc = 0;
};
}