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

    static uint32_t packet_size()
    {
        return 188U;
    }

public:

    packetizer(on_data_callback on_data) :
        m_on_data(on_data)
    {
        assert(m_on_data);
    }

    void read(const uint8_t* data, uint32_t size)
    {
        assert(data != nullptr);
        assert(size > 0);

        // If leftover data and the incoming data is enough to release a packet
        if(!m_buffer.empty() && ((m_buffer.size() + size) > packet_size()))
        {
            // The amount missing in the buffer to constitute a complete packet
            auto delta = packet_size() - m_buffer.size();

            // Is this a valid packet?
            if (data[delta] == sync_byte())
            {
                // Add to buffer
                m_buffer.insert(m_buffer.end(), data, data + delta);
                data += delta;
                size -= delta;

                // Release packet
                m_on_data(m_buffer.data(), m_buffer.size());
            }
            // Either the buffer was released or invalid
            m_buffer.clear();
        }

        // Release as many packets as possible.
        while(size >= packet_size())
        {
            // Check that the data is valid
            if (data[0] == sync_byte())
            {
                // Release packet
                m_on_data(data, packet_size());
                data += packet_size();
                size -= packet_size();
            }
            else
            {
                // Didn't find sync byte, advance the buffer
                data += 1;
                size -= 1;
            }
        }

        // If the buffer already contains data, we don't need a sync byte.
        if (m_buffer.empty())
        {
            // Look a sync byte or end of data
            while(data[0] != sync_byte() && size > 0)
            {
                data += 1;
                size -= 1;
            }
        }

        // Store the data to be used for next call to read.
        m_buffer.insert(m_buffer.end(), data, data + size);
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

    const on_data_callback m_on_data;
    std::vector<uint8_t> m_buffer;
};
}
