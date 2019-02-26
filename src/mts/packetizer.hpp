// Copyright (c) Steinwurf ApS 2017.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include <cassert>
#include <cstdint>
#include <vector>
#include <functional>

namespace mts
{
/// Reads packets of abitray size and then constructs packets of
/// 188 bytes - while ensuring the sync byte 0x47 is present.
class packetizer
{
public:

    using on_data_callback = std::function<void(const uint8_t*,uint32_t)>;

public:

    constexpr static uint32_t packet_size()
    {
        return 188;
    }

    constexpr static uint8_t sync_byte()
    {
        return 0x47;
    }
public:

    packetizer(on_data_callback on_data) :
        m_on_data(on_data)
    {
        assert(m_on_data);
    }

    void read(const uint8_t* data, uint64_t size)
    {
        assert(data != nullptr);
        assert(size != 0);

        uint64_t offset = 0;

        // Fill remaining buffer
        if (!m_buffer.empty())
        {
            auto missing = packet_size() - m_buffer.size();

            if (missing > size)
                missing = size;

            m_buffer.insert(m_buffer.end(), data, data + missing);

            if (m_buffer.size() < packet_size())
            {
                // Not enough data available
                return;
            }

            if (verify(m_buffer.data()))
            {
                offset = missing;
            }
            else
            {
                // Buffer invalid
                m_buffer.clear();
            }
        }

        // Find offset
        while ((size - offset) > 1)
        {
            assert(offset < size);
            if (verify(data + offset))
            {
                break;
            }

            offset++;
            if (!m_buffer.empty())
            {
                // buffer is probably corrupted.
                m_buffer.clear();
                offset = 0;
            }
        }

        // Read buffer
        if (!m_buffer.empty())
        {
            assert(verify(m_buffer.data()));
            assert(m_buffer.size() == packet_size());
            handle_data(m_buffer.data());
            m_buffer.clear();
        }

        // Read remaining
        auto remaining_ts_packets = (size - offset) / packet_size();
        for (uint32_t i = 0; i < remaining_ts_packets; i++)
        {
            if (verify(data + offset))
            {
                handle_data(data + offset);
            }
            else
            {
                // Corrupted package?
            }
            offset += packet_size();
        }

        // Buffer remaining
        assert(m_buffer.empty());
        if (offset == size)
            return;
        m_buffer.insert(m_buffer.begin(), data + offset, data + size);
    }

    void reset()
    {
        m_buffer.clear();
    }

    uint32_t buffered() const
    {
        return m_buffer.size();
    }

private:

    inline bool verify(const uint8_t* data) const
    {
        assert(data != nullptr);
        return data[0] == sync_byte();
    }

    void handle_data(const uint8_t* data) const
    {
        assert(verify(data));
        m_on_data(data, packet_size());
    }

private:

    const on_data_callback m_on_data;
    std::vector<uint8_t> m_buffer;
};
}
