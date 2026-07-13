#include "CommWorker.h"
#include <QSettings>

CommWorker::CommWorker(QObject *parent)
    : QObject(parent)
{
}

void CommWorker::setMode(Mode mode)
{
    m_mode = mode;
}

void CommWorker::start()
{
    m_running = true;
    loadSettings();

    // 清理旧的通信管理器
    if (m_serialComm) {
        m_serialComm->deleteLater();
        m_serialComm = nullptr;
    }
    if (m_tcpComm) {
        m_tcpComm->deleteLater();
        m_tcpComm = nullptr;
    }
    if (m_reconnectTimer) {
        m_reconnectTimer->deleteLater();
        m_reconnectTimer = nullptr;
    }

    if (m_mode == Tcp) {
        m_tcpComm = new TcpCommunicationManager(this);
        setupConnections(m_tcpComm);
    } else {
        m_serialComm = new CommunicationManager(this);
        setupConnections(m_serialComm);
    }

    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setInterval(3000);
    connect(m_reconnectTimer, &QTimer::timeout, this, [this]() {
        if (m_mode == Tcp && !m_lastHost.isEmpty()) {
            connectToHost(m_lastHost, m_lastTcpPort);
        } else if (!m_lastPort.isEmpty()) {
            openPort(m_lastPort, m_lastBaud);
        }
    });
}

void CommWorker::setupConnections(QObject *source)
{
    auto *serial = qobject_cast<CommunicationManager *>(source);
    auto *tcp = qobject_cast<TcpCommunicationManager *>(source);

    if (serial) {
        connect(serial, &CommunicationManager::frameReceived, this, &CommWorker::frameReceived);
        connect(serial, &CommunicationManager::errorOccurred, this, &CommWorker::errorOccurred);
        connect(serial, &CommunicationManager::disconnected,
                this, [this]() { m_reconnectTimer->start(); });
    } else if (tcp) {
        connect(tcp, &TcpCommunicationManager::frameReceived, this, &CommWorker::frameReceived);
        connect(tcp, &TcpCommunicationManager::errorOccurred, this, &CommWorker::errorOccurred);
        connect(tcp, &TcpCommunicationManager::disconnected,
                this, [this]() { m_reconnectTimer->start(); });
    }
}

void CommWorker::stop()
{
    m_running = false;
    if (m_reconnectTimer) m_reconnectTimer->stop();
    closePort();
    emit finished();
}

void CommWorker::openPort(const QString &portName, qint32 baudRate)
{
    if (!m_serialComm) return;
    m_lastPort = portName;
    m_lastBaud = baudRate;
    bool ok = m_serialComm->openSerialPort(portName, baudRate);
    if (ok) {
        m_reconnectTimer->stop();
        saveSettings();
    }
    emit portOpened(ok);
}

void CommWorker::connectToHost(const QString &host, quint16 port)
{
    if (!m_tcpComm) return;
    m_lastHost = host;
    m_lastTcpPort = port;
    m_tcpComm->connectToHost(host, port);
}

void CommWorker::closePort()
{
    if (m_serialComm) m_serialComm->closeSerialPort();
    if (m_tcpComm)  m_tcpComm->disconnectFromHost();
}

void CommWorker::loadSettings()
{
    QSettings settings("DataMonitor", "DataMonitor");
    m_lastPort = settings.value("port/name").toString();
    m_lastBaud = settings.value("port/baud", 115200).toInt();
    m_lastHost = settings.value("tcp/host").toString();
    m_lastTcpPort = settings.value("tcp/port", 502).toUInt();
}

void CommWorker::saveSettings()
{
    QSettings settings("DataMonitor", "DataMonitor");
    settings.setValue("port/name", m_lastPort);
    settings.setValue("port/baud", m_lastBaud);
    settings.setValue("tcp/host", m_lastHost);
    settings.setValue("tcp/port", m_lastTcpPort);
}
