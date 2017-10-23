// Copyright (c) Steinwurf ApS 2017.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

#include "stream_type.hpp"

#include <cassert>
#include <string>

namespace mts
{
/// @return Human readable message corresponding to a stream type
inline std::string stream_type_to_string(mts::stream_type type)
{
    switch (type)
    {
#define STREAM_TYPE_TAG(value,id,msg)      \
                case mts::stream_type::id: return std::string(msg);
#include "stream_type_tags.hpp"
#undef STREAM_TYPE_TAG
    }
    assert(0 && "Invalid stream type received!");
    return "";
}
}
