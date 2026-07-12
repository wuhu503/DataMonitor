#include "FileLogger.h"
#include <QDateTime>
#include <QMutexLocker>

FileLogger::FileLogger(const QString &filePath, QObject *parent)
    : QObject(parent), m_file(filePath)
{
    if (m_file.open(QIODevice::Append | QIODevice::Text)) {
        m_stream.setDevice(&m_file);
        m_stream << "=== DataMonitor Log Started ===" << "\n";
    }
}

FileLogger::~FileLogger()
{
    if (m_file.isOpen()) {
        m_stream << "=== DataMonitor Log Ended ===" << "\n";
        m_file.close();
    }
}

void FileLogger::write(const ParsedFrame &frame)
{
    QMutexLocker lock(&m_mutex);
    if (!m_file.isOpen()) return;
    m_stream << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz")
             << " | ADDR: 0x" << QString::number(frame.address, 16)
             << " | VAL: " << frame.value << "\n";
    m_stream.flush();
}
