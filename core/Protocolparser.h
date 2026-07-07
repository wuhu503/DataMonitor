#ifndef PROTOCOLPARSER_H
#define PROTOCOLPARSER_H

#include <QByteArray>
#include "ParsedFrame.h"

class ProtocolParser
{
public:
    void feed(const QByteArray &data);
    bool hasFrame() ;
    ParsedFrame takeFrame();

private:
    QByteArray m_buffer;
    static constexpr int kMinFrameSize = 6;  // 地址 + 功能码 + 数据(2) + CRC(2)
    static constexpr uint8_t kAddress = 0x01;
    static constexpr uint8_t kFunctionCode = 0x41;
};

#endif // PROTOCOLPARSER_H