#ifndef CHANNELCONFIG_H
#define CHANNELCONFIG_H

#include <cstdint>
#include <QString>

// 通道配置：与 ProtocolParser 的地址列表、界面通道面板保持一致
struct ChannelConfig
{
    uint8_t address = 0x01;      // Modbus 设备地址
    QString name = "通道1";       // 显示名称
    bool enabled = true;         // 是否启用
    double alarmThreshold = 800.0; // 报警阈值
};

#endif // CHANNELCONFIG_H
