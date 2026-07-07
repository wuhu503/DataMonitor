#ifndef PARSEDFRAME_H
#define PARSEDFRAME_H

#include <cstdint>

struct ParsedFrame
{
    uint8_t address      = 0;   // 设备地址
    uint8_t functionCode = 0;   // 功能码
    uint16_t value       = 0;   // 数据值
};

#endif // PARSEDFRAME_H