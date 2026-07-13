#include "DatabaseManager.h"
#include "core/ParsedFrame.h"
#include <QDateTime>
#include <QSqlError>
#include <QSqlRecord>
#include <QUuid>


DatabaseManager::DatabaseManager() {}

DatabaseManager::~DatabaseManager()
{
    close();
}

bool DatabaseManager::open(const QString &dbPath)
{
    QString connName = "data_connection_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
    m_db = QSqlDatabase::addDatabase("QSQLITE", connName);
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        return false;
    }

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