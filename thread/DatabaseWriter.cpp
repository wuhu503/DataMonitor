#include "DatabaseWriter.h"
#include "core/ParsedFrame.h"

DatabaseWriter::DatabaseWriter(const QString &dbPath, QObject *parent)
    : QObject(parent)
    , m_dbPath(dbPath)
{
}

DatabaseWriter::~DatabaseWriter()
{
    m_db.close();
}

void DatabaseWriter::init()
{
    // 在 moveToThread 之后调用，确保 QSqlDatabase 连接归属于数据库线程
    if (!m_db.open(m_dbPath)) {
        emit errorOccurred("打开数据库失败: " + m_dbPath);
    }
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
