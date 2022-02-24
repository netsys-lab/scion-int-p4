#include "bitstring.h"

#include <doctest/doctest.h>

#include <algorithm>
#include <cstdint>
#include <string>


TEST_SUITE("Bitstring") {

TEST_CASE("fromBitstring (big endian)")
{
    std::string str = "\x01\x02\x03\x04\x05\x06\x07\x08";

    CHECK((fromBitstring<2, std::uint16_t>(str)) == 0x0708u);
    CHECK((fromBitstring<4, std::uint32_t>(str)) == 0x05060708u);
    CHECK((fromBitstring<8, std::uint64_t>(str)) == 0x0102030405060708ull);
    CHECK((fromBitstring<4, std::uint64_t>(str)) == 0x05060708u);

    str.resize(1);
    CHECK((fromBitstring<4, std::uint32_t>(str)) == 0x01u);
}

TEST_CASE("toBitstring (big endian)")
{
    using namespace std::string_literals;

    std::string str;
    toBitstring<8>(0x0102030405060708ull, str);
    CHECK(str == "\x01\x02\x03\x04\x05\x06\x07\x08"s);

    str.clear();
    toBitstring<4>(0x01020304u, str);
    CHECK(str == "\x01\x02\x03\x04"s);

    str.clear();
    toBitstring<2>(0x01020304u, str);
    CHECK(str == "\x03\x04"s);

    str.clear();
    toBitstring<8>(0x01020304ull, str);
    CHECK(str == "\x00\x00\x00\x00\x01\x02\x03\x04"s);
}

TEST_CASE("round trip conversion")
{
    std::uint64_t value = 0x0102030405060708;
    std::string str;

    toBitstring<8>(value, str);
    CHECK(fromBitstring<8, std::uint64_t>(str) == value);
}

} // TEST_SUITE
