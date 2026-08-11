#include "Crc16.h"

namespace Crc16 {

// Modbus CRC-16 使用右移算法，多项式需用 0x8005 的位反转值 0xA001
static constexpr uint16_t kPoly = 0xA001;

uint16_t calculate(const QByteArray &data)
{
    uint16_t crc = 0xFFFF;
    for (int i = 0; i < data.size(); ++i) {
        crc ^= static_cast<uint8_t>(data[i]);
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ kPoly;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

bool verify(const QByteArray &data)
{
    if (data.size() < 2) return false;
    QByteArray payload = data.left(data.size() - 2);
    uint16_t expected = calculate(payload);
    // 帧尾按 [CRC低字节][CRC高字节] 存放（与 ProtocolParser 的 kIndexCrcLow/kIndexCrcHigh 约定一致）
    uint16_t actual = (static_cast<uint8_t>(data[data.size() - 1]) << 8)
                      | static_cast<uint8_t>(data[data.size() - 2]);
    return expected == actual;
}

} // namespace Crc16
