#ifndef DATABASEWRITER_H
#define DATABASEWRITER_H

#include <QObject>
#include "db/DatabaseManager.h"

struct ParsedFrame;

class DatabaseWriter : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseWriter(const QString &dbPath, QObject *parent = nullptr);
    ~DatabaseWriter() override;

public slots:
    void saveRecord(const ParsedFrame &frame);
    void queryRecent(int limit);

signals:
    void errorOccurred(const QString &message);
    void queryReady(const QList<ParsedFrame> &records);

private:
    DatabaseManager m_db;
};

#endif // DATABASEWRITER_H