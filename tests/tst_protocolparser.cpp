#include <QtTest>
#include "ProtocolParser.h"
#include "Crc16.h"

namespace {

QByteArray makeFrame(quint8 address, quint8 functionCode, quint16 value)
{
    QByteArray payload;
    payload.append(char(address));
    payload.append(char(functionCode));
    payload.append(char(value & 0xFF));
    payload.append(char((value >> 8) & 0xFF));

    quint16 crc = Crc16::calculate(payload);
    payload.append(char(crc & 0xFF));
    payload.append(char((crc >> 8) & 0xFF));
    return payload;
}

} // namespace

class TestProtocolParser : public QObject
{
    Q_OBJECT

private slots:
    void fullFrame();
    void splitFeed();
    void invalidAddressSkipped();
    void crcErrorSkipped();
    void multipleFrames();
};

void TestProtocolParser::fullFrame()
{
    ProtocolParser parser;
    parser.feed(makeFrame(0x01, 0x41, 0x1234));

    QVERIFY(parser.hasFrame());
    ParsedFrame frame = parser.takeFrame();
    QCOMPARE(frame.address, quint8(0x01));
    QCOMPARE(frame.functionCode, quint8(0x41));
    QCOMPARE(frame.value, quint16(0x1234));
}

void TestProtocolParser::splitFeed()
{
    ProtocolParser parser;
    QByteArray frame = makeFrame(0x02, 0x41, 0x00FF);

    parser.feed(frame.left(3));
    QVERIFY(!parser.hasFrame());

    parser.feed(frame.mid(3));
    QVERIFY(parser.hasFrame());
    ParsedFrame parsed = parser.takeFrame();
    QCOMPARE(parsed.address, quint8(0x02));
    QCOMPARE(parsed.value, quint16(0x00FF));
}

void TestProtocolParser::invalidAddressSkipped()
{
    ProtocolParser parser;
    QByteArray bad = makeFrame(0x99, 0x41, 0x0001);
    QByteArray good = makeFrame(0x03, 0x41, 0x0002);

    parser.feed(bad + good);
    QVERIFY(parser.hasFrame());
    ParsedFrame parsed = parser.takeFrame();
    QCOMPARE(parsed.address, quint8(0x03));
    QCOMPARE(parsed.value, quint16(0x0002));
}

void TestProtocolParser::crcErrorSkipped()
{
    ProtocolParser parser;
    QByteArray bad = makeFrame(0x01, 0x41, 0x0001);
    bad[5] = char(bad[5] ^ 0xFF); // 篡改 CRC 高字节
    QByteArray good = makeFrame(0x04, 0x41, 0x0003);

    parser.feed(bad + good);
    QVERIFY(parser.hasFrame());
    ParsedFrame parsed = parser.takeFrame();
    QCOMPARE(parsed.address, quint8(0x04));
    QCOMPARE(parsed.value, quint16(0x0003));
}

void TestProtocolParser::multipleFrames()
{
    ProtocolParser parser;
    parser.feed(makeFrame(0x01, 0x41, 0x0011));
    parser.feed(makeFrame(0x02, 0x41, 0x0022));

    QVERIFY(parser.hasFrame());
    ParsedFrame first = parser.takeFrame();
    QCOMPARE(first.address, quint8(0x01));
    QCOMPARE(first.value, quint16(0x0011));

    QVERIFY(parser.hasFrame());
    ParsedFrame second = parser.takeFrame();
    QCOMPARE(second.address, quint8(0x02));
    QCOMPARE(second.value, quint16(0x0022));
}

QTEST_APPLESS_MAIN(TestProtocolParser)

#include "tst_protocolparser.moc"
