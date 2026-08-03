#include "CommWorker.h"
#include <QSettings>

CommWorker::CommWorker(QObject *parent)
    : QObject(parent)
{
    // 定时器随对象一起 moveToThread，在 worker 线程内运行
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

void CommWorker::setMode(Mode mode)
{
    m_mode = mode;
}

void CommWorker::start()
{
    m_running = true;
    loadSettings();
    m_reconnectTimer->stop();

    // 清理旧的通信管理器，确保按当前模式重建
    if (m_serialComm) {
        m_serialComm->deleteLater();
        m_serialComm = nullptr;
    }
    if (m_tcpComm) {
        m_tcpComm->deleteLater();
        m_tcpComm = nullptr;
    }

    if (m_mode == Tcp) {
        ensureTcpComm();
    } else {
        ensureSerialComm();
    }
}

void CommWorker::ensureSerialComm()
{
    if (!m_serialComm) {
        m_serialComm = new CommunicationManager(this);
        setupConnections(m_serialComm);
    }
}

void CommWorker::ensureTcpComm()
{
    if (!m_tcpComm) {
        m_tcpComm = new TcpCommunicationManager(this);
        setupConnections(m_tcpComm);
    }
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
        connect(tcp, &TcpCommunicationManager::connectionEstablished, this, [this]() {
            emit tcpConnected();
        });
        connect(tcp, &TcpCommunicationManager::connectionFailed, this, [this](const QString &msg) {
            emit tcpConnectFailed(msg);
            // 首次连接失败也进入自动重试，与断线重连行为一致
            if (m_running) m_reconnectTimer->start();
        });
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
    ensureSerialComm();
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
    ensureTcpComm();
    m_lastHost = host;
    m_lastTcpPort = port;
    m_tcpComm->connectToHost(host, port);
    saveSettings();
}

void CommWorker::closePort()
{
    // 手动断开时停止自动重连；拔出设备触发的 disconnected 会另行启动定时器
    m_reconnectTimer->stop();
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
