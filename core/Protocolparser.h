#ifndef PROTOCOLPARSER_H
#define PROTOCOLPARSER_H

#include <QByteArray>
#include "ParsedFrame.h"

class ProtocolParser
{
public:
    explicit ProtocolParser(uint8_t address = 0x01, uint8_t functionCode = 0x41);
    void feed(const QByteArray &data);
    bool hasFrame();
    ParsedFrame takeFrame();

private:
    QByteArray m_buffer;
    static constexpr int kMinFrameSize = 6;
    uint8_t m_address;
    uint8_t m_functionCode;
};

#endif // PROTOCOLPARSER_H
