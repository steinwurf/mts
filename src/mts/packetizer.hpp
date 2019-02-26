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

    constexpr static uint64_t packet_size()
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

    void read(const uint8_t* data, uint32_t size)
    {
        assert(data != nullptr);
        assert(size > 0);

        m_buffer.insert(m_buffer.end(), data, data + size);

        while (m_buffer.size() > packet_size())
        {
            // check if the head of our buffer is a valid ts packet
            if (m_buffer.at(0) == sync_byte() &&
                m_buffer.at(packet_size()) == sync_byte())
            {
                m_on_data(m_buffer.data(), packet_size());

                m_buffer.erase(m_buffer.begin(), m_buffer.begin() + packet_size());
                continue;
            }

            // if not a valid ts packet, we look for the next sync byte
            auto it = std::find(m_buffer.begin() + 1, m_buffer.end(), sync_byte());
            m_buffer.erase(m_buffer.begin(), it);
        }
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

    const on_data_callback m_on_data;
    std::vector<uint8_t> m_buffer;
};
}
