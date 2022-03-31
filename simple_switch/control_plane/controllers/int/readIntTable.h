#pragma once

#include "addressConversion.h"

#include <fstream>
#include <vector>

static void readIntTable(std::string& intTablePath,
                  std::vector<uint64_t>& asList,
                  std::vector<uint16_t>& bitmaskIntList,
                  std::vector<uint16_t>& bitmaskScionList)
{
    // Get table with bitmasks from file
    std::ifstream intTable;
    
    intTable.open(intTablePath, std::ios::in);
    if (!intTable.is_open())
        throw std::runtime_error(
            std::string("ERROR: Failed to open int_table.txt"));
    
    // Write configuration defined in the file into lists
    std::string line;
    while (std::getline(intTable, line))
    {
        // Check that line is no comment
        if (line[0] != '#')
        {
            std::istringstream lineStr(line);
            std::string asName;
            uint16_t bitmaskInt = 0;
            uint16_t bitmaskScion = 0;
            lineStr >> asName >> std::hex >> bitmaskInt >> bitmaskScion;
            uint16_t isdAddr = 0;
            uint64_t asAddr = 0;
            splitScionAddress(asName, isdAddr, asAddr);
            asAddr = ((uint64_t)isdAddr << 48) + asAddr;
            asList.push_back(asAddr);
            bitmaskIntList.push_back(bitmaskInt);
            bitmaskScionList.push_back(bitmaskScion);
        }
    }
   
    intTable.close();
}
