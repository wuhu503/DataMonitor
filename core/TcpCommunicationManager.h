#ifndef TCPCOMMUNICATIONMANAGER_H
#define TCPCOMMUNICATIONMANAGER_H

#include <QObject>
#include <QTcpSocket>
#include "ProtocolParser.h"

class TcpCommunicationManager : public QObject
{
    Q_OBJECT
public:
    explicit TcpCommunicationManager(QObject *parent = nullptr);
    ~TcpCommunicationManager() override;

    bool connectToHost(const QString &host, quint16 port);
    void disconnectFromHost();
    bool isConnected() const;

signals:
    void frameReceived(const ParsedFrame &frame);
    void errorOccurred(const QString &message);
    void disconnected();
    void connectionEstablished();
    void connectionFailed(const QString &message);

private slots:
    void onReadyRead();
    void onError(QAbstractSocket::SocketError error);

private:
    QTcpSocket *m_socket = nullptr;
    ProtocolParser m_parser;
    bool m_everConnected = false;
};

#endif
