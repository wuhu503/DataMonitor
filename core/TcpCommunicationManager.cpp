#include "TcpCommunicationManager.h"

TcpCommunicationManager::TcpCommunicationManager(QObject *parent)
    : QObject(parent)
{
}

TcpCommunicationManager::~TcpCommunicationManager()
{
    disconnectFromHost();
}

bool TcpCommunicationManager::connectToHost(const QString &host, quint16 port)
{
    if (m_socket) disconnectFromHost();

    m_socket = new QTcpSocket(this);
    m_everConnected = false;

    connect(m_socket, &QTcpSocket::connected, this, [this]() {
        m_everConnected = true;
        emit connectionEstablished();
    });
    connect(m_socket, &QTcpSocket::readyRead, this, &TcpCommunicationManager::onReadyRead);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &TcpCommunicationManager::onError);

    m_socket->connectToHost(host, port);
    return true;
}

void TcpCommunicationManager::disconnectFromHost()
{
    if (m_socket) {
        m_socket->disconnectFromHost();
        m_socket->deleteLater();
        m_socket = nullptr;
    }
}

bool TcpCommunicationManager::isConnected() const
{
    return m_socket && m_socket->state() == QAbstractSocket::ConnectedState;
}

void TcpCommunicationManager::onReadyRead()
{
    QByteArray data = m_socket->readAll();
    m_parser.feed(data);

    while (m_parser.hasFrame()) {
        ParsedFrame frame = m_parser.takeFrame();
        emit frameReceived(frame);
    }
}

void TcpCommunicationManager::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error);
    QString msg = m_socket ? m_socket->errorString() : QString();
    if (!m_everConnected) {
        emit connectionFailed(msg);
    } else {
        emit errorOccurred(msg);
        emit disconnected();
    }
}
