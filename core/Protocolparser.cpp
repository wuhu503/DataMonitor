#include "ProtocolParser.h"
#include "Crc16.h"

ProtocolParser::ProtocolParser(uint8_t address, uint8_t functionCode)
    : m_address(address), m_functionCode(functionCode)
{
}

void ProtocolParser::feed(const QByteArray &data)
{
    m_buffer.append(data);
}

bool ProtocolParser::hasFrame()
{
    int offset = 0;
    while (offset + kMinFrameSize <= m_buffer.size()) {
        // 检查地址字节
        if (static_cast<uint8_t>(m_buffer[offset]) != m_address) {
            offset++;
            continue;
        }

        // 检查功能码
        if (static_cast<uint8_t>(m_buffer[offset + 1]) != m_functionCode) {
            offset++;
            continue;
        }

        // 检查 CRC
        QByteArray frame = m_buffer.mid(offset, kMinFrameSize);
        if (!Crc16::verify(frame)) {
            offset++;
            continue;
        }

        // 一次性移除无效数据
        if (offset > 0) {
            m_buffer.remove(0, offset);
        }
        return true;  // 找到一帧有效数据
    }
    // 移除无效数据
    if (offset > 0) {
        m_buffer.remove(0, offset);
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
