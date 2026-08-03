#include "DatabaseManager.h"
#include "core/ParsedFrame.h"
#include <QDateTime>
#include <QSqlError>
#include <QSqlRecord>


DatabaseManager::DatabaseManager() {}

DatabaseManager::~DatabaseManager()
{
    close();
}

bool DatabaseManager::open(const QString &dbPath)
{
    m_connName = "data_connection";
    
    // 如果连接已存在，先清理
    if (QSqlDatabase::contains(m_connName)) {
        QSqlDatabase::removeDatabase(m_connName);
    }
    
    m_db = QSqlDatabase::addDatabase("QSQLITE", m_connName);
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        return false;
    }

    // 与 UI 历史查询共用同一 SQLite 文件，设置忙等待避免瞬时锁冲突
    QSqlQuery pragma(m_db);
    pragma.exec("PRAGMA busy_timeout=3000");

    QSqlQuery query(m_db);
    query.exec(
        "CREATE TABLE IF NOT EXISTS data_records ("
        "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "  timestamp TEXT NOT NULL,"
        "  address INTEGER NOT NULL,"
        "  value INTEGER NOT NULL"
        ")"
        );
    return true;
}

void DatabaseManager::close()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
    // 先释放 QSqlDatabase 对连接的引用，再移除连接，避免 removeDatabase 告警
    m_db = QSqlDatabase();
    // 清理连接
    if (!m_connName.isEmpty()) {
        QSqlDatabase::removeDatabase(m_connName);
        m_connName.clear();
    }
}

bool DatabaseManager::insertRecord(const ParsedFrame &frame)
{
    QSqlQuery query(m_db);
    query.prepare("INSERT INTO data_records (timestamp, address, value) VALUES (?, ?, ?)");
    query.addBindValue(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz"));
    query.addBindValue(frame.address);
    query.addBindValue(frame.value);
    return query.exec();
}

QList<ParsedFrame> DatabaseManager::queryRecent(int limit)
{
    QList<ParsedFrame> records;
    QSqlQuery query(m_db);
    query.prepare("SELECT address, value FROM data_records ORDER BY id DESC LIMIT ?");
    query.addBindValue(limit);

    if (query.exec()) {
        while (query.next()) {
            ParsedFrame frame;
            frame.address = query.value(0).toUInt();
            frame.value = query.value(1).toUInt();
            records.append(frame);
        }
    }
    return records;
}
