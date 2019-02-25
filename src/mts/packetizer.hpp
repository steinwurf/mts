// Copyright (c) Steinwurf ApS 2017.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include <cassert>
#include <cstdint>
#include <vector>
#include <functional>
#include <algorithm>

namespace mts
{
/// Reads packets of abitray size and then constructs packets of
/// 188 bytes - while ensuring the sync byte 0x47 is present.
class packetizer
{
public:

    using on_data_callback = std::function<void(const uint8_t*,uint32_t)>;

public:

    static uint8_t sync_byte()
    {
        return 0x47;
    }

public:

    packetizer(on_data_callback on_data, uint32_t packet_size=188) :
        m_on_data(on_data),
        m_packet_size(packet_size)
    {
        assert(m_packet_size > 0);
    }

    void read(const uint8_t* data, uint32_t size)
    {
        assert(data != nullptr);
        assert(size > 0);

        m_buffer.insert(m_buffer.end(), data, data + size);

        while (m_buffer.size() > m_packet_size)
        {
            // check if the head of our buffer is a valid mts packet
            if (m_buffer[0] == sync_byte() && m_buffer[m_packet_size] == sync_byte())
            {
                handle_data(m_buffer.data());

                m_buffer.erase(m_buffer.begin(), m_buffer.begin() + m_packet_size);
                continue;
            }

            auto it = std::find(++m_buffer.begin(), m_buffer.end(), sync_byte());
            m_buffer.erase(m_buffer.begin(), it);
        }
    }

    void reset()
    {
        m_buffer.clear();
    }

    uint32_t buffered()
    {
        return m_buffer.size();
    }

private:

    void handle_data(const uint8_t* data) const
    {
        m_on_data(data, m_packet_size);
    }

private:

    const on_data_callback m_on_data;

    const uint32_t m_packet_size;

    std::vector<uint8_t> m_buffer;
};
}
