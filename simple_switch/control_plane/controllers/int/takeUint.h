#pragma once

static uint64_t takeUint64(const char* payload, uint32_t pos)
{
        // Data is transferred in big-endian format, but saved in little-endian format
        uint64_t number;
        for (int i = 0; i < 8; i ++)
        {
            *(reinterpret_cast<char*>(&number) + i) = *(payload + pos + 7 - i);
        }
        return number;
}

static uint32_t takeUint32(const char* payload, uint32_t pos)
{
        // Data is transferred in big-endian format, but saved in little-endian format
        uint32_t number;
        for (int i = 0; i < 4; i ++)
        {
            *(reinterpret_cast<char*>(&number) + i) = *(payload + pos + 3 - i);
        }
        return number;
}

static uint16_t takeUint16(const char* payload, uint32_t pos)
{
        // Data is transferred in big-endian format, but saved in little-endian format
        uint16_t number;
        for (int i = 0; i < 2; i ++)
        {
            *(reinterpret_cast<char*>(&number) + i) = *(payload + pos + 1 - i);
        }
        return number;
}

static uint8_t takeUint8(const char* payload, uint32_t pos)
{
        // Data is transferred in big-endian format, but saved in little-endian format
        uint8_t number;
        *(reinterpret_cast<char*>(&number)) = *(payload + pos);
        return number;
}
