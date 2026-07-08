#include "CommunicationManager.h"
#include "Crc16.h"

CommunicationManager::CommunicationManager(QObject *parent)
    : QObject(parent)
{
}

CommunicationManager::~CommunicationManager()
{
    closeSerialPort();
}

bool CommunicationManager::openSerialPort(const QString &portName, qint32 baudRate)
{
    if (m_serial) {
        closeSerialPort();
    }

    m_serial = new QSerialPort(this);
    m_serial->setPortName(portName);
    m_serial->setBaudRate(baudRate);
    m_serial->setDataBits(QSerialPort::Data8);
    m_serial->setParity(QSerialPort::NoParity);
    m_serial->setStopBits(QSerialPort::OneStop);

    if (!m_serial->open(QIODevice::ReadWrite)) {
        emit errorOccurred(m_serial->errorString());
        delete m_serial;
        m_serial = nullptr;
        return false;
    }

    connect(m_serial, &QSerialPort::readyRead, this, &CommunicationManager::onReadyRead);
    return true;
}

void CommunicationManager::closeSerialPort()
{
    if (m_serial) {
        m_serial->close();
        m_serial->deleteLater();
        m_serial = nullptr;
    }
}

bool CommunicationManager::isOpen() const
{
    return m_serial && m_serial->isOpen();
}

void CommunicationManager::onReadyRead()
{
    QByteArray data = m_serial->readAll();
    m_parser.feed(data);

    while (m_parser.hasFrame()) {
        ParsedFrame frame = m_parser.takeFrame();
        emit frameReceived(frame);
    }
}