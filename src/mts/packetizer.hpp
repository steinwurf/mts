// Copyright (c) Steinwurf ApS 2017.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include <cassert>
#include <cstdint>
#include <vector>
#include <functional>

#include "buffer_view.hpp"

namespace mts
{
/// Reads packets of abitray size and then constructs packets of
/// 188 bytes - while ensuring the sync byte 0x47 is present.

struct packetizer
{

public:

    static uint8_t sync_byte()
    {
        return 0x47;
    }

    static uint64_t packet_size()
    {
        return 188U;
    }

public:

    packetizer(std::function<void(const uint8_t *, uint32_t)> on_data) :
        m_handle_data(on_data)
    {
        assert(m_handle_data);
    }

    void reset()
    {
        m_incomplete.clear();
    }

    uint64_t buffered() const
    {
        return m_incomplete.size();
    }

    void read(const uint8_t *data, uint32_t size)
    {
        buffer_view buffer{data, size};

        if (has_incomplete())
        {
            // If the buffer is currently not empty we check if the incoming
            // data can complete a previously incomplete packet
            buffer = handle_incomplete(buffer);
        }
        buffer = sync(buffer);

        if (buffer.is_empty())
        {
            return;
        }

        assert(buffer.in_sync());

        // Deliver all the complete packets
        for (uint32_t i = 0; i < buffer.packets(); ++i)
        {
            m_handle_data(buffer.packet_at(i), 188U);
        }

        // If we have some remaining data add it to the incomplete
        if (buffer.has_partial_packet())
        {
            const uint8_t *partial_data = buffer.partial_packet_data();
            uint32_t partial_size = buffer.partial_packet_size();

            m_incomplete.insert(m_incomplete.end(),
                                partial_data,
                                partial_data + partial_size);
        }
    }

    bool has_incomplete() const
    {
        return !m_incomplete.empty();
    }

    uint32_t missing_bytes() const
    {
        assert(m_incomplete.size() > 0);
        assert(m_incomplete.size() < 188U);
        return 188U - m_incomplete.size();
    }

    /// @return An empty or in sync buffer
    buffer_view handle_incomplete(buffer_view buffer)
    {
        uint32_t missing = missing_bytes();

        if (buffer.size() > missing)
        {
            // We know that the after consuming "missing" bytes from the data
            // buffer we should see a sync byte - if not we cannot merge the
            // buffer and have to give up and start over
            buffer_view tmp = slide(buffer, missing);

            if (tmp.in_sync())
            {
                // The sync bytes line up taking into account both the incomplete
                // packet and the incoming data
                m_incomplete.insert(m_incomplete.end(), buffer.data(), buffer.data() + missing);
                m_handle_data(m_incomplete.data(), 188U);
                m_incomplete.clear();

                return tmp;
            }
            else
            {
                // The sync bytes were not found at the correct offsets. We have
                // to assume something was lost. We restart the read with the
                // incomplete buffer cleared.
                m_incomplete.clear();
                return sync(buffer);
            }
        }
        else if (buffer.size() == missing)
        {
            // We release the buffer
            m_incomplete.insert(m_incomplete.end(), buffer.data(), buffer.data() + missing);
            m_handle_data(m_incomplete.data(), 188U);
            m_incomplete.clear();

            return buffer_view{};
        }
        else
        {
            // We did not have enough data to fill the incomplete packet
            m_incomplete.insert(m_incomplete.end(), buffer.data(), buffer.data() + missing);
            return buffer_view{};
        }
    }

    buffer_view slide(buffer_view buffer, uint32_t offset) const
    {
        const uint8_t *data = buffer.data() + offset;
        uint32_t size = buffer.size() - offset;

        return buffer_view{data, size};
    }

    /// @return An empty or in sync buffer
    buffer_view sync(buffer_view buffer) const
    {
        if (buffer.in_sync())
        {
            return buffer;
        }

        // Two cases
        // 1. We have a partial or just one ts packet
        // 2. We have data for multiple ts packets

        // Case 1:
        if (buffer.size() < 188U)
        {
            return buffer_view{};
        }

        // Case 2:
        uint32_t max_offset = buffer.size() - 188U;

        for (uint32_t offset = 0; offset <= max_offset; ++offset)
        {
            buffer_view tmp = slide(buffer, offset);

            // Look for the intial sync byte
            if (!tmp.in_sync())
            {
                continue;
            }

            return tmp;
        }

        return buffer_view{};
    }

    std::vector<uint8_t> m_incomplete;
    std::function<void(const uint8_t *, uint32_t)> m_handle_data;
};
}
