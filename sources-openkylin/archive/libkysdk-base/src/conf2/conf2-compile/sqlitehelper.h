#ifndef SQLITEHELPER_H
#define SQLITEHELPER_H
#include <QMap>
#include <QString>
#include <QVariant>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

class SqliteHelper
{
public:
    SqliteHelper();

public:
    QMap<QString, QVariant> database2Map(const QString &dbFile);
    bool map2Database(const QMap<QString, QVariant> &map, const QString &databaseFile);
    bool createDbFile(const QString &databaseFile);
    bool updateTableStructure(const QString &databaseFile);

private:
    void writeGroupInDatabase(const QMap<QString, QVariant> &versionData, const QMap<QString, QVariant> &data, QSqlQuery &query, int versionId, int parentId);
    void writeKeyInDatabase(const QMap<QString, QVariant> &versionData, const QString &key, const QMap<QString, QVariant> &value, QSqlQuery &query, int versionId, int parentId, const QString &parentPermission);
    QString calculateDefaultVersion(const QMap<QString, QVariant> &data);
    QVariant recursiveSearch(const QMap<QString, QVariant> &dictionary, const QString &key);
};

#endif // SQLITEHELPER_H
