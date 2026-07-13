#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariantList>

struct ParsedFrame;

class DatabaseManager
{
public:
    DatabaseManager();
    ~DatabaseManager();

    bool open(const QString &dbPath);
    void close();
    bool insertRecord(const ParsedFrame &frame);
    QList<ParsedFrame> queryRecent(int limit = 100);

private:
    QSqlDatabase m_db;
    QString m_connName;
};

#endif // DATABASEMANAGER_H

