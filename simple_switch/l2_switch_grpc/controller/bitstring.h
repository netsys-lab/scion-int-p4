#pragma once

#include "common.h"
#include <string>


/// \brief Convert a string of bytes in big-endian byte-order to a little-endian value.
/// \tparam bytes Maximum number of bytes to extract from the bitstring.
template <size_t bytes, typename T>
T fromBitstring(const std::string& bitstring)
{
    static_assert(sizeof(T) >= bytes,
        "Return value is too short for the requested number of bytes.");

    T value = 0;
    size_t i = 0;
    for (auto c = bitstring.rbegin(); c != bitstring.rend() && i < bytes; ++c)
        value |= static_cast<T>(static_cast<unsigned char>(*c)) << (8 * i++);

    return value;
}

/// \brief Write a little-endian value to a big-endian bitstring.
/// \tparam bytes Number of bytes to copy over.
template <size_t bytes, typename T>
void toBitstring(T value, std::string& bitstring)
{
    static_assert(sizeof(T) >= bytes,
        "Return value is too short for the requested number of bytes.");

    bitstring.resize(bytes);
    for (size_t i = 0; i < bytes; ++i)
        bitstring[i] = static_cast<char>((value >> (8 * (bytes - i - 1))) & 0xff);
}
