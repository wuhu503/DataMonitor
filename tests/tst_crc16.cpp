#include <QtTest>
#include "Crc16.h"

class TestCrc16 : public QObject
{
    Q_OBJECT

private slots:
    void knownVector();
    void verifyValidFrame();
    void verifyCorruptedFrame();
    void verifyTooShort();
};

void TestCrc16::knownVector()
{
    // CRC-16/MODBUS 标准校验向量："123456789" -> 0x4B37
    QByteArray data("123456789");
    QCOMPARE(Crc16::calculate(data), quint16(0x4B37));
}

void TestCrc16::verifyValidFrame()
{
    QByteArray payload;
    payload.append(char(0x01));
    payload.append(char(0x41));
    payload.append(char(0x34));
    payload.append(char(0x12));

    quint16 crc = Crc16::calculate(payload);
    QByteArray frame = payload;
    frame.append(char(crc & 0xFF));        // 低字节在前
    frame.append(char((crc >> 8) & 0xFF)); // 高字节在后

    QVERIFY(Crc16::verify(frame));
}

void TestCrc16::verifyCorruptedFrame()
{
    QByteArray payload;
    payload.append(char(0x01));
    payload.append(char(0x41));
    payload.append(char(0x34));
    payload.append(char(0x12));

    quint16 crc = Crc16::calculate(payload);
    QByteArray frame = payload;
    frame.append(char((crc & 0xFF) ^ 0x01)); // 篡改 CRC 低字节
    frame.append(char((crc >> 8) & 0xFF));

    QVERIFY(!Crc16::verify(frame));
}

void TestCrc16::verifyTooShort()
{
    QVERIFY(!Crc16::verify(QByteArray()));
    QVERIFY(!Crc16::verify(QByteArray(1, char(0xAA))));
}

QTEST_APPLESS_MAIN(TestCrc16)

#include "tst_crc16.moc"
