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

    static uint8_t sync_byte()
    {
        return 0x47;
    }

public:

    packetizer(uint32_t packet_size=188) :
        m_packet_size(packet_size)
    { }

    void read(const uint8_t* data, uint32_t size)
    {
        uint32_t offset = 0;

        // Fill remaining buffer
        if (!m_buffer.empty())
        {
            auto missing = m_packet_size - m_buffer.size();

            if (missing > size)
                missing = size;

            m_buffer.insert(m_buffer.end(), data, data + missing);

            if (m_buffer.size() < m_packet_size)
            {
                // Not enough data available
                return;
            }

            if (verify(m_buffer.data(), m_buffer.size()))
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
            if (verify(data + offset, size - offset))
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
            assert(verify(m_buffer.data(), m_packet_size));
            assert(m_buffer.size() == m_packet_size);
            handle_data(m_buffer.data());
            m_buffer.clear();
        }

        // Read remaining
        auto remaining_ts_packets = (size - offset) / m_packet_size;
        for (uint32_t i = 0; i < remaining_ts_packets; i++)
        {
            if (verify(data + offset, m_packet_size))
            {
                handle_data(data + offset);
            }
            else
            {
                // Corrupted package?
            }
            offset += m_packet_size;
        }

        // Buffer remaining
        assert(m_buffer.empty());
        if (offset == size)
            return;
        m_buffer.insert(m_buffer.begin(), data + offset, data + size);
    }

    bool verify(const uint8_t* data, uint32_t size) const
    {
        assert(data != nullptr);
        assert(size != 0);
        if (m_verify)
            return m_verify(data, size);
        return data[0] == sync_byte();
    }

    void handle_data(const uint8_t* data) const
    {
        assert(verify(data, m_packet_size));
        if (m_on_data)
            m_on_data(data, m_packet_size);
    }

    void reset()
    {
        m_buffer.clear();
    }

    uint32_t buffered()
    {
        return m_buffer.size();
    }

    void set_on_data(std::function<void(const uint8_t*,uint32_t)> on_data)
    {
        m_on_data = on_data;
    }

    void set_verify(std::function<bool(const uint8_t*,uint32_t)> verify)
    {
        m_verify = verify;
    }

private:

    const uint32_t m_packet_size;
    std::vector<uint8_t> m_buffer;
    std::function<void(const uint8_t*,uint32_t)> m_on_data;
    std::function<bool(const uint8_t*,uint32_t)> m_verify;
};
}
