#ifndef DEVICEINFODATABASE_H
#define DEVICEINFODATABASE_H

#include <QString>
#include <QVariantMap>
#include <QList>
#include <sqlite3.h>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>

class DeviceInfoDatabase
{
public:
    static DeviceInfoDatabase* instance();

    bool openDatabase(const QString &dbPath, const QString &password);
    void closeDatabase();

    QJsonObject queryDevices(const QString &pid,
                             const QString &vid,
                             const QString &vendor,
                             const QString &model,
                             const QString &type);
    bool is_open = false;

private:
    DeviceInfoDatabase();
    ~DeviceInfoDatabase();

    DeviceInfoDatabase(const DeviceInfoDatabase&) = delete;
    DeviceInfoDatabase& operator=(const DeviceInfoDatabase&) = delete;

    sqlite3 *m_db;

    QList<QString> getAliasVendors(const QString &vendor);
    QVariantMap fetchFirstRow(sqlite3_stmt *stmt);
    QStringList splitDriverNames(const QString &driverStr);
    QString getDriverIds(const QStringList &driverNames, sqlite3_stmt *stmt);

    QJsonObject localQueryV11Driver(const QString &pid,
                                const QString &vid,
                                const QString &vendor,
                                const QString &model,
                                const QString &type);
    QJsonObject localQueryV10Driver(const QString &pid,
                            const QString &vid,
                            const QString &vendor,
                            const QString &model,
                            const QString &type);    
    QJsonObject onlineQueryV10Driver(const QString &pid,
                            const QString &vid,
                            const QString &vendor,
                            const QString &model,
                            const QString &type);    
    
    bool checkOnlineInstallable(QJsonObject &jsonObj);
    QString getVersionFromDeb(const QString &debName);
};

#endif // DEVICEINFODATABASE_H
