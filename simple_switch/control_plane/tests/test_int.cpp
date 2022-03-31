#include "controllers/int/addressConversion.h"
#include "controllers/int/readIntTable.h"
#include "controllers/int/takeUint.h"

#include <doctest/doctest.h>

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>


TEST_SUITE("Int") {

TEST_CASE("splitScionAddress")
{
    std::string str = "";
    uint16_t isdAddress = 0;
    uint64_t asAddress = 0;
    
    splitScionAddress(str, isdAddress, asAddress);
    CHECK(isdAddress == 0);
    CHECK(asAddress == 0);
    
    str = "fa-ff00:0:100";
    splitScionAddress(str, isdAddress, asAddress);
    CHECK(isdAddress == 0xfau);
    CHECK(asAddress == 0xff0000000100ull);
}

TEST_CASE("splitIpAddress")
{
    std::string str = "";
    std::string ipAddress = "";
    uint16_t port = 0;
    
    splitIpAddress(str, ipAddress, port);
    CHECK(ipAddress == "");
    CHECK(port == 0);
    
    str = "255.127.63.31:54321";
    splitIpAddress(str, ipAddress, port);
    CHECK(ipAddress == "255.127.63.31");
    CHECK(port == 54321);
}

TEST_CASE("ReadTable")
{
    std::vector<uint64_t> asList;
    std::vector<uint16_t> bitmaskIntList;
    std::vector<uint16_t> bitmaskScionList;
    
    std::vector<uint64_t> asList2;
    std::vector<uint16_t> bitmaskIntList2;
    std::vector<uint16_t> bitmaskScionList2;
    
    std::string tablePath = "int_table1.txt";
    readIntTable(tablePath, asList, bitmaskIntList, bitmaskScionList);
    
    CHECK(asList == asList2);
    CHECK(bitmaskIntList == bitmaskIntList2);
    CHECK(bitmaskScionList == bitmaskScionList2);
    
    tablePath = "int_table2.txt";
    readIntTable(tablePath, asList, bitmaskIntList, bitmaskScionList);
    
    asList2 = {0x0001ff0000000001ull, 0x0001ff0000000020ull, 0x0001ff00ff000300ull, 0x000fff0000000004ull};
    bitmaskIntList2 = {0, 0, 0x8d00, 0xffff};
    bitmaskScionList2 = {0, 0, 1, 0xffff};
    
    for (int i = 0; i < 4; i++)
    {
        CHECK(asList[i] == asList2[i]);
        CHECK(bitmaskIntList[i] == bitmaskIntList2[i]);
        CHECK(bitmaskScionList[i] == bitmaskScionList2[i]);
    }
}

TEST_CASE("TakeUint")
{
    std::string str;
    str.assign({'\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x01', '\x01', '\x02', '\x03', '\x05', '\x08', '\x0d', '\x15'});
    auto strChar = str.c_str();
    CHECK(takeUint64(strChar, 0) == 0);
    CHECK(takeUint64(strChar, 8) == 0x0101020305080d15ull);
    CHECK(takeUint32(strChar, 6) == 0x00000101u);
    CHECK(takeUint16(strChar, 14) == 0x0d15);
    CHECK(takeUint8(strChar, 15) == 0x15);
}

} // TEST_SUITE
