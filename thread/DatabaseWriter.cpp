#include "DatabaseWriter.h"
#include "core/ParsedFrame.h"
#include <QTimer>

DatabaseWriter::DatabaseWriter(const QString &dbPath, QObject *parent)
    : QObject(parent)
{
    if (!m_db.open(dbPath)) {
        QTimer::singleShot(0, this, [this]() {
            emit errorOccurred("打开数据库失败");
        });
    }
}

DatabaseWriter::~DatabaseWriter()
{
    m_db.close();
}

void DatabaseWriter::saveRecord(const ParsedFrame &frame)
{
    m_db.insertRecord(frame);
}

void DatabaseWriter::queryRecent(int limit)
{
    auto records = m_db.queryRecent(limit);
    emit queryReady(records);
}