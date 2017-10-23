// Copyright (c) Steinwurf ApS 2017.
// All Rights Reserved
//
// Distributed under the "BSD License". See the accompanying LICENSE.rst file.

#pragma once

namespace mts
{
/// Enumeration of different nalu types, we use a bit of macro
/// uglyness to makes this easy. PHK says this is ok so if you have a
/// problem with it - take it up with him :)
///
/// http://phk.freebsd.dk/time/20141116.html
enum class stream_type
{
#define STREAM_TYPE_TAG(value,id,msg) id=value,
#include "stream_type_tags.hpp"
#undef STREAM_TYPE_TAG
};
}
