#ifndef COMMWORKER_H
#define COMMWORKER_H

#include <QObject>
#include "core/CommunicationManager.h"

class CommWorker : public QObject
{
    Q_OBJECT

public:
    explicit CommWorker(QObject *parent = nullptr);

public slots:
    void start();
    void stop();
    void openPort(const QString &portName, qint32 baudRate);
    void closePort();

signals:
    void frameReceived(const ParsedFrame &frame);
    void errorOccurred(const QString &message);
    void portOpened(bool success);
    void finished();

private:
    CommunicationManager *m_comm = nullptr;
    bool m_running = false;
};

#endif // COMMWORKER_H