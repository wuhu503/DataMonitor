#include "ProtocolParser.h"
#include "Crc16.h"

void ProtocolParser::feed(const QByteArray &data)
{
    m_buffer.append(data);
}

bool ProtocolParser::hasFrame()
{
    // 从缓冲区里找完整帧
    while (m_buffer.size() >= kMinFrameSize) {
        // 检查地址字节
        if (static_cast<uint8_t>(m_buffer[0]) != kAddress) {
            m_buffer.remove(0, 1);  // 不是我们的设备，跳过
            continue;
        }

        // 检查功能码
        if (static_cast<uint8_t>(m_buffer[1]) != kFunctionCode) {
            m_buffer.remove(0, 1);
            continue;
        }

        // 检查 CRC
        QByteArray frame = m_buffer.left(kMinFrameSize);
        if (!Crc16::verify(frame)) {
            m_buffer.remove(0, 1);
            continue;
        }

        return true;  // 找到一帧有效数据
    }
    return false;
}

ParsedFrame ProtocolParser::takeFrame()
{
    ParsedFrame frame;
    frame.address = static_cast<uint8_t>(m_buffer[0]);
    frame.functionCode = static_cast<uint8_t>(m_buffer[1]);
    // 数据：小端序，低字节在前
    frame.value = static_cast<uint16_t>(static_cast<uint8_t>(m_buffer[2]))
                  | (static_cast<uint16_t>(static_cast<uint8_t>(m_buffer[3])) << 8);

    m_buffer.remove(0, kMinFrameSize);  // 移除已解析的帧
    return frame;
}