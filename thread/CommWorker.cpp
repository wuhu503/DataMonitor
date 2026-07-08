#include "CommWorker.h"

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
}

void CommWorker::stop()
{
    m_running = false;
    if (m_comm) {
        m_comm->closeSerialPort();
    }
    emit finished();
}

void CommWorker::openPort(const QString &portName, qint32 baudRate)
{
    if (!m_comm) return;
    bool ok = m_comm->openSerialPort(portName, baudRate);
    emit portOpened(ok);
}

void CommWorker::closePort()
{
    if (m_comm) {
        m_comm->closeSerialPort();
    }
}