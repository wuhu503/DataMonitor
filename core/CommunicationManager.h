#ifndef COMMUNICATIONMANAGER_H
#define COMMUNICATIONMANAGER_H

#include <QObject>
#include <QSerialPort>
#include "ProtocolParser.h"

class CommunicationManager : public QObject
{
    Q_OBJECT
public:
    explicit CommunicationManager(QObject *parent = nullptr);
    ~CommunicationManager() override;
    bool openSerialPort(const QString &portName, qint32 baudRate);
    void closeSerialPort();
    bool isOpen() const;

signals:
    void frameReceived(const ParsedFrame &frame);
    void errorOccurred(const QString &message);
    void disconnected();

private slots:
    void onReadyRead();

private:
    QSerialPort *m_serial = nullptr;
    ProtocolParser m_parser;
};

#endif // COMMUNICATIONMANAGER_H
