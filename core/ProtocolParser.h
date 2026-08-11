#ifndef PROTOCOLPARSER_H
#define PROTOCOLPARSER_H

#include <QByteArray>
#include <QList>
#include "ParsedFrame.h"

class ProtocolParser
{
public:
    // 默认接受 0x01-0x04 四个通道地址（与界面通道面板一致），功能码可配置
    explicit ProtocolParser(QList<uint8_t> addresses = {0x01, 0x02, 0x03, 0x04},
                            uint8_t functionCode = 0x41);
    void feed(const QByteArray &data);
    bool hasFrame();
    ParsedFrame takeFrame();

private:
    QByteArray m_buffer;
    static constexpr int kMinFrameSize = 6;
    // 帧布局：[0 地址][1 功能码][2 数值低][3 数值高][4 CRC低][5 CRC高]
    static constexpr int kIndexAddress  = 0;
    static constexpr int kIndexFunction = 1;
    static constexpr int kIndexDataLow  = 2;
    static constexpr int kIndexDataHigh = 3;
    static constexpr int kIndexCrcLow   = 4;
    static constexpr int kIndexCrcHigh  = 5;
    QList<uint8_t> m_addresses;
    uint8_t m_functionCode;
};

#endif // PROTOCOLPARSER_H
