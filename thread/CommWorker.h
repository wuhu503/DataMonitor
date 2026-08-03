#ifndef COMMWORKER_H
#define COMMWORKER_H

#include <QObject>
#include <QTimer>
#include "core/CommunicationManager.h"
#include "core/TcpCommunicationManager.h"

class CommWorker : public QObject
{
    Q_OBJECT

public:
    enum Mode { Serial, Tcp };
    explicit CommWorker(QObject *parent = nullptr);

    void setMode(Mode mode);

public slots:
    void start();
    void stop();
    void openPort(const QString &portName, qint32 baudRate);
    void connectToHost(const QString &host, quint16 port);
    void closePort();
    void loadSettings();
    void saveSettings();

signals:
    void frameReceived(const ParsedFrame &frame);
    void errorOccurred(const QString &message);
    void portOpened(bool success);
    void tcpConnected();
    void tcpConnectFailed(const QString &message);
    void finished();

private:
    void ensureSerialComm();
    void ensureTcpComm();
    void setupConnections(QObject *source);

    Mode m_mode = Serial;
    CommunicationManager *m_serialComm = nullptr;
    TcpCommunicationManager *m_tcpComm = nullptr;
    bool m_running = false;
    QString m_lastPort;
    qint32 m_lastBaud = 0;
    QString m_lastHost;
    quint16 m_lastTcpPort = 502;
    QTimer *m_reconnectTimer = nullptr;
};

#endif // COMMWORKER_H
