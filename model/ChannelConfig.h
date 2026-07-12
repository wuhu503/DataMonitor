#ifndef CHANNELCONFIG_H
#define CHANNELCONFIG_H

#include <cstdint>
#include <QString>

struct ChannelConfig
{
    uint8_t address = 0x01;
    QString name = "通道1";
    bool enabled = true;
    double alarmThreshold = 800.0;
};

#endif // CHANNELCONFIG_H
