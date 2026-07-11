#ifndef FILELOGGER_H
#define FILELOGGER_H

#include <QFile>
#include <QTextStream>
#include <QObject>
#include "ParsedFrame.h"

class FileLogger
    : public QObject
{
    Q_OBJECT

public:
    explicit FileLogger(const QString &filePath, QObject *parent = nullptr);
    ~FileLogger();

    void write(const ParsedFrame &frame);

private:
    QFile m_file;
    QTextStream m_stream;
};

#endif
