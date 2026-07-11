#include "CommWorker.h"
#include <QSettings>

CommWorker::CommWorker(QObject *parent)
    : QObject(parent)
{
}

void CommWorker::start()
{
    m_running = true;
    m_comm = new CommunicationManager(this);

    connect(m_comm, &CommunicationManager::frameReceived,
            this, &CommWorker::frameReceived);
    connect(m_comm, &CommunicationManager::errorOccurred,
            this, &CommWorker::errorOccurred);
    connect(m_comm, &CommunicationManager::disconnected,
            this, [this]() { m_reconnectTimer->start(); });

    loadSettings();
    m_reconnectTimer = new QTimer(this);
    m_reconnectTimer->setInterval(3000);
    connect(m_reconnectTimer, &QTimer::timeout, this, [this]() {
        if (!m_lastPort.isEmpty()) {
            openPort(m_lastPort, m_lastBaud);
        }
    });
}

void CommWorker::stop()
{
    m_running = false;
    if (m_reconnectTimer) m_reconnectTimer->stop();
    if (m_comm) {
        m_comm->closeSerialPort();
    }
    emit finished();
}

void CommWorker::openPort(const QString &portName, qint32 baudRate)
{
    if (!m_comm) return;
    m_lastPort = portName;
    m_lastBaud = baudRate;
    bool ok = m_comm->openSerialPort(portName, baudRate);
    if (ok) {
        m_reconnectTimer->stop();
        saveSettings();
    }
    emit portOpened(ok);
}

void CommWorker::closePort()
{
    if (m_comm) {
        m_comm->closeSerialPort();
    }
}

void CommWorker::loadSettings()
{
    QSettings settings("DataMonitor", "DataMonitor");
    m_lastPort = settings.value("port/name").toString();
    m_lastBaud = settings.value("port/baud", 115200).toInt();
}

void CommWorker::saveSettings()
{
    QSettings settings("DataMonitor", "DataMonitor");
    settings.setValue("port/name", m_lastPort);
    settings.setValue("port/baud", m_lastBaud);
}