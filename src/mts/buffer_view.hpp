// Copyright (c) Steinwurf ApS 2017.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include <cstdint>
#include <cassert>
#include <memory>
#include <bnb/stream_reader.hpp>
#include <boost/optional.hpp>

#include "helper.hpp"

namespace mts
{
struct buffer_view
{
    buffer_view() : m_data(nullptr), m_size(0), m_in_sync(false)
    {
    }

    buffer_view(const uint8_t *data, uint32_t size) : m_data(data), m_size(size)
    {
        assert(m_data != nullptr);
        assert(m_size > 0);

        if (!sync_at(0))
        {
            m_in_sync = false;
            return;
        }

        for (uint32_t i = 1; i < packets(); ++i)
        {
            if (!sync_at(i * 188U))
            {
                m_in_sync = false;
                return;
            }
        }

        m_in_sync = true;
    }

    buffer_view(const uint8_t *data, uint32_t size, bool in_sync) : m_data(data), m_size(size), m_in_sync(in_sync)
    {
        assert(m_data != nullptr);
        assert(m_size > 0);
    }

    bool in_sync() const
    {
        return m_in_sync;
    }

    bool is_empty() const
    {
        return m_size == 0;
    }

    uint32_t packets() const
    {
        // assert(m_in_sync);
        return m_size / 188U;
    }

    uint32_t size() const
    {
        return m_size;
    }

    const uint8_t* data() const
    {
        return m_data;
    }

    const uint8_t* packet_at(uint32_t packet_index)
    {
        return m_data + (packet_index * 188);
    }

    bool has_partial_packet() const
    {
        return partial_packet_size() > 0;
    }

    const uint8_t *partial_packet_data() const
    {
        return m_data + (packets() * 188);
    }

    uint32_t partial_packet_size() const
    {
        return m_size - (packets() * 188U);
    }

    bool sync_at(uint32_t offset) const
    {
        return m_data[offset] == 0x47;
    }

private:

    const uint8_t *m_data;
    uint32_t m_size;
    bool m_in_sync;
};

}
