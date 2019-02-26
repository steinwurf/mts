// Copyright (c) Steinwurf ApS 2017.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include <cstdint>
#include <map>
#include <system_error>
#include <vector>

#include <recycle/unique_pool.hpp>

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

    using pool_type = recycle::unique_pool<stream_state>;

    constexpr static uint32_t packet_size = 188;

public:

    parser() :
        m_stream_state_pool(
            pool_type::allocate_function(std::make_unique<stream_state>),
            [](auto& o) { o->m_data.resize(0); })
    { }

    void read(const uint8_t* data, std::error_code& error)
    {
        if (has_pes())
        {
            // Assume previous pes has been read and start the next one.
            m_pes_pid = 0;
            m_pes = nullptr;
        }

        bnb::stream_reader<endian::big_endian> reader(data, packet_size, error);
        auto res = mts::ts_packet::parse(reader);
        if (error)
            return;
        auto& ts_packet = *res;

        if (!ts_packet.has_payload_field())
            return;

        auto pid = ts_packet.pid();

        if (has_stream(pid))
        {
            assert(pid != 0);
            assert(m_programs.count(pid) == 0);

            // Verify data
            if (has_stream_state(pid))
            {
                auto& stream_state = m_stream_states.at(pid);
                auto expected_counter =
                    (stream_state->m_last_continuity_counter + 1) % 16;

                if (ts_packet.continuity_counter() != expected_counter)
                {
                    m_continuity_errors += 1;
                    m_stream_states.erase(pid);
                    return;
                }
                stream_state->m_last_continuity_counter = expected_counter;
            }

            // extract data and create state
            if (ts_packet.payload_unit_start_indicator())
            {
                // extract if state exists
                if (has_stream_state(pid))
                {
                    m_pes_pid = pid;
                    m_pes = std::move(m_stream_states.at(pid));
                }
                // create new stream state
                auto stream_state = m_stream_state_pool.allocate();
                stream_state->m_last_continuity_counter = ts_packet.continuity_counter();
                m_stream_states[pid] = std::move(stream_state);
            }

            // insert data
            if (has_stream_state(pid))
            {
                auto& buffer = m_stream_states.at(pid)->m_data;
                buffer.insert(
                    buffer.end(),
                    reader.remaining_data(),
                    reader.remaining_data() + reader.remaining_size());
            }
            return;
        }

        if (ts_packet.payload_unit_start_indicator())
        {
            uint8_t pointer_field = 0;
            reader.read_bytes<1>(pointer_field);
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
                    m_programs.emplace(program_pid, boost::none);
                }
            }
        }
        else
        {
            auto result = m_programs.find(pid);
            // if this is a program pid and the program we have hasn't been
            // initialized.
            if (result != m_programs.end() && result->second == boost::none)
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

        m_pes_pid = 0;
        m_pes.reset();
    }

    bool has_pes() const
    {
        return m_pes != nullptr;
    }

    const std::vector<uint8_t>& pes_data() const
    {
        assert(has_pes());
        return m_pes->m_data;
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

    uint32_t continuity_errors() const
    {
        return m_continuity_errors;
    }

private:

    bool has_stream_state(uint16_t pid) const
    {
        return m_stream_states.count(pid) != 0;
    }

    const program::stream_entry* find_stream(uint16_t pid) const
    {
        for (const auto& item : m_programs)
        {
            const auto& program = item.second;
            if (program == boost::none)
                continue;
            for (const auto& stream_entry : program->stream_entries())
            {
                if (pid == stream_entry.pid())
                {
                    return &stream_entry;
                }
            }
        }
        return nullptr;
    }

private:

    pool_type m_stream_state_pool;

    std::map<uint16_t, boost::optional<program>> m_programs;
    std::map<uint16_t, pool_type::pool_ptr> m_stream_states;
    pool_type::pool_ptr m_pes;
    uint16_t m_pes_pid = 0;
    uint32_t m_continuity_errors = 0;
};
}
