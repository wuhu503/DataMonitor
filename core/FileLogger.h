#ifndef FILELOGGER_H
#define FILELOGGER_H

#include <QFile>
#include <QTextStream>
#include <QObject>
#include <QMutex>
#include "ParsedFrame.h"

class FileLogger
    : public QObject
{
    Q_OBJECT

public:
    explicit FileLogger(const QString &filePath, QObject *parent = nullptr);
    ~FileLogger();

public slots:
    void write(const ParsedFrame &frame);

private:
    QMutex m_mutex;
    QFile m_file;
    QTextStream m_stream;
};

#endif
