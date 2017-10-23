// Copyright (c) Steinwurf ApS 2017.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include <cstdint>
#include <vector>
#include <system_error>
#include <map>

#include "pes.hpp"
#include "pat.hpp"
#include "program.hpp"

#include "ts_packet.hpp"

namespace mts
{
class parser
{
private:

    struct stream_state
    {
        std::vector<uint8_t> m_data;
        uint8_t m_last_continuity_counter;
    };

public:

    parser(uint32_t packet_size=188) :
        m_packet_size(packet_size)
    { }

    void read(const uint8_t* data, std::error_code& error)
    {
        if (has_pes())
        {
            m_pes_pid = 0;
            m_pes_data.resize(0);
        }

        bnb::stream_reader<endian::big_endian> reader(
            data, m_packet_size, error);

        auto ts_packet = mts::ts_packet::parse(reader);
        if (error)
            return;

        if (!ts_packet->has_payload_field())
            return;

        auto pid = ts_packet->pid();

        if (has_stream(pid))
        {
            assert(pid != 0);
            assert(m_programs.count(pid) == 0);

            // Verify data
            if (has_stream_state(pid))
            {
                auto& stream_state = m_stream_states.at(pid);
                auto expected_counter =
                    (stream_state.m_last_continuity_counter + 1) % 16;

                if (ts_packet->continuity_counter() != expected_counter)
                {
                    m_stream_states.erase(pid);
                    return;
                }
                stream_state.m_last_continuity_counter = expected_counter;
            }

            // extract data and create state
            if (ts_packet->payload_unit_start_indicator())
            {
                // extract if state exists
                if (has_stream_state(pid))
                {
                    m_pes_pid = pid;
                    m_pes_data = std::move(m_stream_states.at(pid).m_data);
                }
                // create new stream state
                m_stream_states[pid] =
                    { std::vector<uint8_t>(), ts_packet->continuity_counter() };
            }

            // insert data
            if (has_stream_state(pid))
            {
                auto& buffer = m_stream_states.at(pid).m_data;
                buffer.insert(
                    buffer.end(),
                    reader.remaining_data(),
                    reader.remaining_data() + reader.remaining_size());
            }
            return;
        }

        if (ts_packet->payload_unit_start_indicator())
        {
            uint8_t pointer_field = 0;
            reader.read<endian::u8>(pointer_field);
            if (pointer_field != 0)
            {
                reader.skip(pointer_field);
            }
        }

        if (pid == 0)
        {
            auto pat = mts::pat::parse(reader);
            if (error)
                return;

            for (const auto& program_entry : pat->program_entries())
            {
                if (program_entry.is_network_pid())
                {
                    continue;
                }
                auto program_pid = program_entry.pid();
                if (!m_programs.count(program_pid))
                {
                    m_programs.emplace(program_pid, nullptr);
                }
            }
        }
        else
        {
            auto result = m_programs.find(pid);
            // if this is a program pid and the program we have hasn't been
            // initialized.
            if (result != m_programs.end() && result->second == nullptr)
            {
                auto program = mts::program::parse(reader);
                if (error)
                    return;

                m_programs[pid] = program;
                return;
            }
        }
    }

    void reset()
    {
        m_programs.clear();
        m_stream_states.clear();

        m_pes_data.resize(0);
        m_pes_pid = 0;
    }

    bool has_pes() const
    {
        return m_pes_pid != 0;
    }

    const std::vector<uint8_t>& pes_data() const
    {
        assert(has_pes());
        return m_pes_data;
    }

    uint16_t pes_pid() const
    {
        assert(has_pes());
        return m_pes_pid;
    }

    bool has_stream(uint16_t pid) const
    {
        return find_stream(pid) != nullptr;
    }

    mts::stream_type stream_type(uint16_t pid) const
    {
        auto stream_entry = find_stream(pid);
        assert(stream_entry != nullptr);
        return static_cast<mts::stream_type>(stream_entry->type());
    }

    uint32_t packet_size() const
    {
        return m_packet_size;
    }

private:

    bool has_stream_state(uint16_t pid) const
    {
        return m_stream_states.count(pid) != 0;
    }

    std::shared_ptr<program::stream_entry> find_stream(uint16_t pid) const
    {
        for (const auto& item : m_programs)
        {
            auto program = item.second;
            if (program == nullptr)
                continue;
            for (const auto& stream_entry : program->stream_entries())
            {
                if (pid == stream_entry->pid())
                {
                    return stream_entry;
                }
            }
        }
        return nullptr;
    }

private:

    const uint32_t m_packet_size;

    std::map<uint16_t, std::shared_ptr<program>> m_programs;
    std::map<uint16_t, stream_state> m_stream_states;

    std::vector<uint8_t> m_pes_data;
    uint16_t m_pes_pid = 0;
};
}
