#ifndef PARSEDFRAME_H
#define PARSEDFRAME_H

#include <cstdint>

// 协议帧固定 6 字节，布局约定：
//   [0 地址][1 功能码][2 数值低字节][3 数值高字节][4 CRC低字节][5 CRC高字节]
// 数值按小端解析；CRC 按“低字节在前”存放（详见 Crc16::verify / ProtocolParser）。
struct ParsedFrame
{
    uint8_t address      = 0;   // 设备地址
    uint8_t functionCode = 0;   // 功能码
    uint16_t value       = 0;   // 数据值
};

#endif // PARSEDFRAME_H
