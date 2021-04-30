#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

using std::size_t;
using std::uint32_t;
using std::uint64_t;

using DeviceId = uint64_t;
using ElectionId = uint64_t;
using DeviceConfig = std::vector<std::byte>;

constexpr size_t MAC_ADDR_BYTES = 6;
using MacAddr = uint64_t;

constexpr size_t PORT_BYTES = 2; // lower 9 bits are used
using Port = uint32_t;
