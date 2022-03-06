#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

using std::size_t;
using std::uint32_t;
using std::uint64_t;

constexpr size_t ISD_BYTES = 2;
using isdAddr = uint64_t;

constexpr size_t AS_BYTES = 6;
using asAddr = uint64_t;

constexpr size_t FLAG_BYTES = 1;
using flag = uint8_t;

constexpr size_t NODE_ID_BYTES = 4;
using nodeID_t = uint32_t;

constexpr size_t LINK_UTIL_BYTES = 4;
using LinkUtil = uint32_t;
