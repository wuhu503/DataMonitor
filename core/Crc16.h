#ifndef CRC16_H
#define CRC16_H

#include <cstdint>
#include <QByteArray>

namespace Crc16 {

// 计算单块数据的 CRC-16/Modbus
uint16_t calculate(const QByteArray &data);

// 验证数据（最后两字节为 CRC）
bool verify(const QByteArray &data);

} // namespace Crc16

#endif // CRC16_H
