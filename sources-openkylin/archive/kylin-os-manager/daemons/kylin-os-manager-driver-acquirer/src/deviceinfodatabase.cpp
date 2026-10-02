#include "deviceinfodatabase.h"
#include <QDebug>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusPendingCallWatcher>
#include <QtDBus/QDBusPendingReply>
#include <QtDBus/QDBusReply>
#include <QJsonDocument>

#include "include/common.h"

static QString getDriverName(QString driverName);

static QString getDriverVersion(QString driverName);

DeviceInfoDatabase::DeviceInfoDatabase() : m_db(nullptr) {
    openDatabase("/usr/share/kylin-os-manager/data/kylin_driver_data.db", "1111111111111111111111111");
}

DeviceInfoDatabase::~DeviceInfoDatabase()
{
    closeDatabase();
}

DeviceInfoDatabase *DeviceInfoDatabase::instance()
{
    static DeviceInfoDatabase s_instance;
    return &s_instance;
}

bool DeviceInfoDatabase::openDatabase(const QString &dbPath, const QString &password) {
    // 打开数据库
    if (sqlite3_open(dbPath.toUtf8().constData(), &m_db) != SQLITE_OK) {
        qDebug() << "Failed to open database:" << sqlite3_errmsg(m_db);
        return false;
    }

    // 设置密钥和所有加密参数（必须一次性执行，不能分多次 PRAGMA）
    QString encryptionSql = QString(
                                "PRAGMA key = '%1'; "
                                "PRAGMA cipher_compatibility = 3;"
                                "PRAGMA kdf_iter = 64000; "
                                "PRAGMA cipher_page_size = 1024; "
                                "PRAGMA cipher_hmac_algorithm = HMAC_SHA1; "
                                "PRAGMA cipher_kdf_algorithm = PBKDF2_HMAC_SHA1; "
                                ).arg(password);

    if (sqlite3_exec(m_db, encryptionSql.toUtf8().constData(), nullptr, nullptr, nullptr) != SQLITE_OK) {
        qDebug() << "Failed to configure encryption:" << sqlite3_errmsg(m_db);
        sqlite3_close(m_db);
        m_db = nullptr;
        return false;
    }

    // 验证密钥（必须在新连接中执行，因为某些参数需要重新加载）
    const char *testSql = "SELECT count(*) FROM alias_name;";
    if (sqlite3_exec(m_db, testSql, nullptr, nullptr, nullptr) != SQLITE_OK) {
        qDebug() << "Invalid encryption key or parameters:" << sqlite3_errmsg(m_db);
        sqlite3_close(m_db);
        m_db = nullptr;
        return false;
    }

    is_open = true;
    return true;
}

void DeviceInfoDatabase::closeDatabase()
{
    if (m_db) {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
}

QList<QString> DeviceInfoDatabase::getAliasVendors(const QString &vendor)
{
    QList<QString> vendorList;
    vendorList << vendor;

    QString sql = "SELECT manufacter FROM alias_name WHERE alias_name = ? OR manufacter = ?;";
    sqlite3_stmt *stmt = nullptr;

    if (sqlite3_prepare_v2(m_db, sql.toUtf8().constData(), -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_text(stmt, 1, vendor.toUtf8().constData(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, vendor.toUtf8().constData(), -1, SQLITE_TRANSIENT);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            QString m = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
            if (!vendorList.contains(m))
                vendorList << m;
        }
    }
    sqlite3_finalize(stmt);
    return vendorList;
}

QVariantMap DeviceInfoDatabase::fetchFirstRow(sqlite3_stmt *stmt)
{
    QVariantMap result;
    int colCount = sqlite3_column_count(stmt);

    for (int i = 0; i < colCount; ++i) {
        QString colName = sqlite3_column_name(stmt, i);
        const char *val = reinterpret_cast<const char *>(sqlite3_column_text(stmt, i));
        result[colName] = val ? QString(val) : QVariant();
    }

    return result;
}

QStringList DeviceInfoDatabase::splitDriverNames(const QString &driverStr) {
    QStringList drivers = driverStr.split(QRegExp("[\\s\\n]+"), QString::SkipEmptyParts);
    QStringList uniqueSet;
    for (QString &d : drivers) {
        QString trimmed = d.trimmed();
        if (!trimmed.isEmpty())
            uniqueSet.append(trimmed);
    }
    return uniqueSet;
}

QString DeviceInfoDatabase::getDriverIds(const QStringList &driverNames, sqlite3_stmt *stmt) {
    QStringList idList;
    QString sql = "SELECT id FROM driver_list WHERE driver_name = ?;";

    for (const QString &name : driverNames) {
        if (sqlite3_prepare_v2(m_db, sql.toUtf8().constData(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, name.toUtf8().constData(), -1, SQLITE_TRANSIENT);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                int id = sqlite3_column_int(stmt, 0);
                idList << QString::number(id);
            }
        }
        sqlite3_finalize(stmt);
    }

    return idList.join(",");  // 多个驱动 ID 以逗号拼接
}

bool DeviceInfoDatabase::checkOnlineInstallable(QJsonObject &jsonObj){
    QJsonArray jsonArr = jsonObj["data"].toArray();
    for (const QJsonValue& value : jsonArr) {
        if (value.isObject()) {
            QJsonObject obj = value.toObject();
            if (obj.contains("packageName") && obj["packageName"].isString()) {
                QString driver = obj["packageName"].toString();
                QString packageName = getDriverName(driver);
                QString pkgVersion = getDriverVersion(driver);
                qDebug() << "PackageName: " << packageName << "   version: " << pkgVersion;
                QDBusConnection bus = QDBusConnection::systemBus();
                QDBusInterface manager("com.kylin.UpgradeStrategies", "/com/kylin/UpgradeStrategies", "com.kylin.UpgradeStrategies.interface", bus);
                QDBusMessage response = manager.call("PackageIsOnlineInstallable", packageName, pkgVersion);
                bool result = response.arguments().at(0).toBool();
                if (result)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

QJsonObject DeviceInfoDatabase::queryDevices(const QString &pid,
                                             const QString &vid,
                                             const QString &vendor,
                                             const QString &model,
                                             const QString &type)
{
    QJsonObject v11result = localQueryV11Driver(pid, vid, vendor, model, type);
    QJsonObject v10result = localQueryV10Driver(pid, vid, vendor, model, type);
    QJsonArray v11Array = v11result["data"].toArray();
    QJsonArray v10Array = v10result["data"].toArray();
    bool installable = false;
    if (!v11Array.isEmpty())
    {   
        qDebug() << "v11";
        installable = checkOnlineInstallable(v11result);
        if (installable)
        {
            return v11result;
        }
    }
    if (!v10Array.isEmpty())
    {
        qDebug() << "v10";
        installable = checkOnlineInstallable(v10result);
        // if (installable)
        // {
        return v10result;
        // }
        // else if(!v11Array.isEmpty()){
        //     return v11result;
        // }
    }
    QJsonObject onlineRes = onlineQueryV10Driver(pid, vid, vendor, model, type);
    QJsonArray onlineArray = onlineRes["data"].toArray();
    if (!onlineRes.isEmpty())
    {
        qDebug() << "online v10";
        installable = checkOnlineInstallable(onlineRes);
        if (installable)
        {
            return onlineRes;
        }else{
            if (!v11Array.isEmpty())
            {
                return v11result;
            }else if(!v10Array.isEmpty()){
                return v10result;
            }
        }
    }
    return v11result;
}

QJsonObject DeviceInfoDatabase::localQueryV11Driver(const QString &pid,
                                             const QString &vid,
                                             const QString &vendor,
                                             const QString &model,
                                             const QString &type)
{
    QJsonArray dataArray;
    QSet<QString> uniqueKeys;

    // 生成唯一 key 的 lambda
    auto generateKey = [](const QJsonObject &item) {
        return item.value("packageName").toString() + "|" +
                item.value("type").toString() + "|" +
                item.value("ppdName").toString();
    };

    if (!m_db) {
        return QJsonObject{
            {"code", 500},
            {"message", "数据库未打开"},
            {"data", QJsonArray()},
            {"timestamp", QDateTime::currentMSecsSinceEpoch()},
            {"lastRequestTime", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")}
        };
    }

    QString arch = getDebianArchitecture();
    if (arch.isEmpty()) arch = "amd64";
    QString system_version = "v11";

    QStringList tables;
    if (type == "printer") tables << "printer_list";
    else if (type == "scanner") tables << "scanner_list";
    else tables << "printer_list" << "scanner_list";

    QList<QString> vendorList = getAliasVendors(vendor);

    for (const QString &table : tables) {
        sqlite3_stmt *stmt = nullptr;

        // 精准 pid + vid 查
        if (!pid.isEmpty() && !vid.isEmpty()) {
            QString sql = QString("SELECT * FROM %1 WHERE pid = ? AND vid = ? AND arch = ? AND system_version = ?;").arg(table);
            if (sqlite3_prepare_v2(m_db, sql.toUtf8().constData(), -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1, pid.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 2, vid.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 3, arch.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 4, system_version.toUtf8().constData(), -1, SQLITE_TRANSIENT);

                while (sqlite3_step(stmt) == SQLITE_ROW) {
                    QVariantMap row = fetchFirstRow(stmt);
                    QJsonObject item;
                    item["packageName"]  = row.value("package_name").toString();
                    QStringList drivers = splitDriverNames(item["packageName"].toString());
                    item["digit_id"] = getDriverIds(drivers, stmt);
                    item["type"]         = row.value("type").toString();
                    item["others"]       = row.value("others").isValid() ? row.value("others").toString() : QJsonValue();
                    item["ppdName"]      = row.value("ppd_name").isValid() ? row.value("ppd_name").toString() : QJsonValue();

                    if (row.value("type").toString() == "printer" && !row.value("ppd_name").toString().isEmpty())
                        item["ppdScore"] = 2147483647;

                    item["packageLevel"] = row.value("package_level").isValid() ? row.value("package_level").toString() : QJsonValue();

                    QString key = generateKey(item);
                    if (!uniqueKeys.contains(key)) {
                        uniqueKeys.insert(key);
                        dataArray.append(item);
                    }
                }
            }
            sqlite3_finalize(stmt);
            if (!dataArray.isEmpty()) break;
        }

        if (dataArray.isEmpty()) {
            // 精准 vendor + model 查
            for (const QString &v : vendorList) {
                QString sql = QString("SELECT * FROM %1 WHERE manufacter = ? AND model = ? AND arch = ? AND system_version = ?;").arg(table);
                if (sqlite3_prepare_v2(m_db, sql.toUtf8().constData(), -1, &stmt, nullptr) == SQLITE_OK) {
                    sqlite3_bind_text(stmt, 1, v.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                    sqlite3_bind_text(stmt, 2, model.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                    sqlite3_bind_text(stmt, 3, arch.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                    sqlite3_bind_text(stmt, 4, system_version.toUtf8().constData(), -1, SQLITE_TRANSIENT);

                    while (sqlite3_step(stmt) == SQLITE_ROW) {
                        QVariantMap row = fetchFirstRow(stmt);
                        QJsonObject item;
                        item["packageName"]  = row.value("package_name").toString();
                        QStringList drivers = splitDriverNames(item["packageName"].toString());
                        item["digit_id"] = getDriverIds(drivers, stmt);
                        item["type"]         = row.value("type").toString();
                        item["others"]       = row.value("others").isValid() ? row.value("others").toString() : QJsonValue();
                        item["ppdName"]      = row.value("ppd_name").isValid() ? row.value("ppd_name").toString() : QJsonValue();

                        if (row.value("type").toString() == "printer" && !row.value("ppd_name").toString().isEmpty())
                            item["ppdScore"] = 2147483647;

                        item["packageLevel"] = row.value("package_level").isValid() ? row.value("package_level").toString() : QJsonValue();

                        QString key = generateKey(item);
                        if (!uniqueKeys.contains(key)) {
                            uniqueKeys.insert(key);
                            dataArray.append(item);
                        }
                    }
                }
                sqlite3_finalize(stmt);
                if (!dataArray.isEmpty()) break;
            }
        }

        if (dataArray.isEmpty()) {
            // 模糊 vendor + model 查
            for (const QString &v : vendorList) {
                QString sql = QString("SELECT * FROM %1 WHERE manufacter LIKE ? AND model LIKE ? AND arch = ? AND system_version = ?;").arg(table);
                if (sqlite3_prepare_v2(m_db, sql.toUtf8().constData(), -1, &stmt, nullptr) == SQLITE_OK) {
                    QString vLike = "%" + v + "%";
                    QString mLike = "%" + model + "%";
                    sqlite3_bind_text(stmt, 1, vLike.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                    sqlite3_bind_text(stmt, 2, mLike.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                    sqlite3_bind_text(stmt, 3, arch.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                    sqlite3_bind_text(stmt, 4, system_version.toUtf8().constData(), -1, SQLITE_TRANSIENT);

                    while (sqlite3_step(stmt) == SQLITE_ROW) {
                        QVariantMap row = fetchFirstRow(stmt);
                        QJsonObject item;
                        item["packageName"]  = row.value("package_name").toString();
                        QStringList drivers = splitDriverNames(item["packageName"].toString());
                        item["digit_id"] = getDriverIds(drivers, stmt);
                        item["type"]         = row.value("type").toString();
                        item["others"]       = row.value("others").isValid() ? row.value("others").toString() : QJsonValue();
                        item["ppdName"]      = row.value("ppd_name").isValid() ? row.value("ppd_name").toString() : QJsonValue();

                        if (row.value("type").toString() == "printer" && !row.value("ppd_name").toString().isEmpty())
                            item["ppdScore"] = 2147483647;

                        item["packageLevel"] = row.value("package_level").isValid() ? row.value("package_level").toString() : QJsonValue();

                        QString key = generateKey(item);
                        if (!uniqueKeys.contains(key)) {
                            uniqueKeys.insert(key);
                            dataArray.append(item);
                        }
                    }
                }
                sqlite3_finalize(stmt);
            }
        }
    }

    // 遍历数组中的所有对象
    for (int i = 0; i < dataArray.size(); ++i) {
        QJsonObject item = dataArray[i].toObject();
        QString packageNames = item["packageName"].toString();
        // 分割包名字符串（根据换行符或分号等分隔符）
        QStringList packageList = packageNames.split("\n");
        QStringList driverNames;
        QStringList versions;

        // 为每个包获取驱动名称和版本
        for (const QString &packageName : packageList) {
            QString trimmedPackageName = packageName.trimmed();
            if (!trimmedPackageName.isEmpty()) {
                driverNames.append(getDriverName(trimmedPackageName));
                versions.append(getDriverVersion(trimmedPackageName));
            }
        }
        QStringList driverList;
        for (int i = 0; i < driverNames.length(); i++)
        {   
            QString tmp = driverNames[i] + "=" + versions[i];
            driverList.append(tmp);
        }
        item["driver"] = driverList.join(",");

        // 替换数组中的当前元素
        dataArray[i] = item;
    }


    QJsonObject result;
    result["code"]             = 200;
    result["message"]          = QStringLiteral("请求成功！");
    result["data"]             = dataArray;
    result["timestamp"]        = QDateTime::currentMSecsSinceEpoch();
    result["lastRequestTime"]  = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    qDebug() << "V11 Result: " << result;
    return result;
}

QJsonObject DeviceInfoDatabase::localQueryV10Driver(const QString &pid,
                                             const QString &vid,
                                             const QString &vendor,
                                             const QString &model,
                                             const QString &type)
{
    QJsonArray dataArray;
    QSet<QString> uniqueKeys;

    // 生成唯一 key 的 lambda
    auto generateKey = [](const QJsonObject &item) {
        return item.value("packageName").toString() + "|" +
                item.value("type").toString() + "|" +
                item.value("ppdName").toString();
    };

    if (!m_db) {
        return QJsonObject{
            {"code", 500},
            {"message", "数据库未打开"},
            {"data", QJsonArray()},
            {"timestamp", QDateTime::currentMSecsSinceEpoch()},
            {"lastRequestTime", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss")}
        };
    }

    QString arch = getDebianArchitecture();
    if (arch.isEmpty()) arch = "amd64";
    if (arch.compare("loong64", Qt::CaseInsensitive) == 0) arch = "loongarch64";

    QString system_version = "v10sp1";

    QStringList tables;
    if (type == "printer") tables << "printer_list";
    else if (type == "scanner") tables << "scanner_list";
    else tables << "printer_list" << "scanner_list";

    QList<QString> vendorList = getAliasVendors(vendor);

    for (const QString &table : tables) {
        sqlite3_stmt *stmt = nullptr;

        // 精准 pid + vid 查
        if (!pid.isEmpty() && !vid.isEmpty()) {
            QString sql = QString("SELECT * FROM %1 WHERE pid = ? AND vid = ? AND arch = ? AND system_version = ?;").arg(table);
            if (sqlite3_prepare_v2(m_db, sql.toUtf8().constData(), -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_text(stmt, 1, pid.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 2, vid.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 3, arch.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                sqlite3_bind_text(stmt, 4, system_version.toUtf8().constData(), -1, SQLITE_TRANSIENT);

                while (sqlite3_step(stmt) == SQLITE_ROW) {
                    QVariantMap row = fetchFirstRow(stmt);
                    QJsonObject item;
                    item["packageName"]  = row.value("package_name").toString();
                    QStringList drivers = splitDriverNames(item["packageName"].toString());
                    item["digit_id"] = getDriverIds(drivers, stmt);
                    item["type"]         = row.value("type").toString();
                    item["others"]       = row.value("others").isValid() ? row.value("others").toString() : QJsonValue();
                    item["ppdName"]      = row.value("ppd_name").isValid() ? row.value("ppd_name").toString() : QJsonValue();

                    if (row.value("type").toString() == "printer" && !row.value("ppd_name").toString().isEmpty())
                        item["ppdScore"] = 2147483647;

                    item["packageLevel"] = row.value("package_level").isValid() ? row.value("package_level").toString() : QJsonValue();

                    QString key = generateKey(item);
                    if (!uniqueKeys.contains(key)) {
                        uniqueKeys.insert(key);
                        dataArray.append(item);
                    }
                }
            }
            sqlite3_finalize(stmt);
            if (!dataArray.isEmpty()) break;
        }

        if (dataArray.isEmpty()) {
            // 精准 vendor + model 查
            for (const QString &v : vendorList) {
                QString sql = QString("SELECT * FROM %1 WHERE manufacter = ? AND model = ? AND arch = ? AND system_version = ?;").arg(table);
                if (sqlite3_prepare_v2(m_db, sql.toUtf8().constData(), -1, &stmt, nullptr) == SQLITE_OK) {
                    sqlite3_bind_text(stmt, 1, v.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                    sqlite3_bind_text(stmt, 2, model.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                    sqlite3_bind_text(stmt, 3, arch.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                    sqlite3_bind_text(stmt, 4, system_version.toUtf8().constData(), -1, SQLITE_TRANSIENT);

                    while (sqlite3_step(stmt) == SQLITE_ROW) {
                        QVariantMap row = fetchFirstRow(stmt);
                        QJsonObject item;
                        item["packageName"]  = row.value("package_name").toString();
                        QStringList drivers = splitDriverNames(item["packageName"].toString());
                        item["digit_id"] = getDriverIds(drivers, stmt);
                        item["type"]         = row.value("type").toString();
                        item["others"]       = row.value("others").isValid() ? row.value("others").toString() : QJsonValue();
                        item["ppdName"]      = row.value("ppd_name").isValid() ? row.value("ppd_name").toString() : QJsonValue();

                        if (row.value("type").toString() == "printer" && !row.value("ppd_name").toString().isEmpty())
                            item["ppdScore"] = 2147483647;

                        item["packageLevel"] = row.value("package_level").isValid() ? row.value("package_level").toString() : QJsonValue();

                        QString key = generateKey(item);
                        if (!uniqueKeys.contains(key)) {
                            uniqueKeys.insert(key);
                            dataArray.append(item);
                        }
                    }
                }
                sqlite3_finalize(stmt);
                if (!dataArray.isEmpty()) break;
            }
        }

        if (dataArray.isEmpty()) {
            // 模糊 vendor + model 查
            for (const QString &v : vendorList) {
                QString sql = QString("SELECT * FROM %1 WHERE manufacter LIKE ? AND model LIKE ? AND arch = ? AND system_version = ?;").arg(table);
                if (sqlite3_prepare_v2(m_db, sql.toUtf8().constData(), -1, &stmt, nullptr) == SQLITE_OK) {
                    QString vLike = "%" + v + "%";
                    QString mLike = "%" + model + "%";
                    sqlite3_bind_text(stmt, 1, vLike.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                    sqlite3_bind_text(stmt, 2, mLike.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                    sqlite3_bind_text(stmt, 3, arch.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                    sqlite3_bind_text(stmt, 4, system_version.toUtf8().constData(), -1, SQLITE_TRANSIENT);

                    while (sqlite3_step(stmt) == SQLITE_ROW) {
                        QVariantMap row = fetchFirstRow(stmt);
                        QJsonObject item;
                        item["packageName"]  = row.value("package_name").toString();
                        QStringList drivers = splitDriverNames(item["packageName"].toString());
                        item["digit_id"] = getDriverIds(drivers, stmt);
                        item["type"]         = row.value("type").toString();
                        item["others"]       = row.value("others").isValid() ? row.value("others").toString() : QJsonValue();
                        item["ppdName"]      = row.value("ppd_name").isValid() ? row.value("ppd_name").toString() : QJsonValue();

                        if (row.value("type").toString() == "printer" && !row.value("ppd_name").toString().isEmpty())
                            item["ppdScore"] = 2147483647;

                        item["packageLevel"] = row.value("package_level").isValid() ? row.value("package_level").toString() : QJsonValue();

                        QString key = generateKey(item);
                        if (!uniqueKeys.contains(key)) {
                            uniqueKeys.insert(key);
                            dataArray.append(item);
                        }
                    }
                }
                sqlite3_finalize(stmt);
            }
        }
    }

    // 遍历数组中的所有对象
    for (int i = 0; i < dataArray.size(); ++i) {
        QJsonObject item = dataArray[i].toObject();
        QString packageNames = item["packageName"].toString();
        // 分割包名字符串（根据换行符或分号等分隔符）
        QStringList packageList = packageNames.split("\n");
        QStringList driverNames;
        QStringList versions;

        // 为每个包获取驱动名称和版本
        for (const QString &packageName : packageList) {
            QString trimmedPackageName = packageName.trimmed();
            if (!trimmedPackageName.isEmpty()) {
                driverNames.append(getDriverName(trimmedPackageName));
                versions.append(getDriverVersion(trimmedPackageName));
            }
        }
        
        QStringList driverList;
        for (int i = 0; i < driverNames.length(); i++)
        {   
            QString tmp = driverNames[i] + "=" + versions[i];
            driverList.append(tmp);
        }
        item["driver"] = driverList.join(",");

        // 替换数组中的当前元素
        dataArray[i] = item;
    }


    QJsonObject result;
    result["code"]             = 200;
    result["message"]          = QStringLiteral("请求成功！");
    result["data"]             = dataArray;
    result["timestamp"]        = QDateTime::currentMSecsSinceEpoch();
    result["lastRequestTime"]  = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    qDebug() << "V10 Result: " << result;

    return result;
}

QJsonObject DeviceInfoDatabase::onlineQueryV10Driver(const QString &pid,
                                             const QString &vid,
                                             const QString &vendor,
                                             const QString &model,
                                             const QString &type)
{
    QJsonObject jsonObject;
    QString scheme = "https";

    QString httpRequest = QString("%1://%2/api/v1/getprintertype?systemVersion=%3&arch=%4%5%6%7%8")
            .arg(scheme)
            .arg(QString("api.kylinos.cn"))
            .arg(QString("V10SP1"))
            .arg(getDebianArchitecture())
            .arg(pid.size() ? QString("&pid=" + pid) : "")
            .arg(vid.size() ? QString("&vid=" + vid) : "")
            .arg(vendor.size() ? QString("&manufacter=" + vendor) : "")
            .arg(model.size() ? QString("&model=" + model) : "");
    if (httpRequest.contains("+")) {
        httpRequest.replace("+", "%2B");
    }

    QNetworkAccessManager manager;
    QNetworkRequest netRequest;
    QNetworkReply *netReply;
    QEventLoop loop;
    qDebug() << "http link is :" << httpRequest;
    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    netRequest.setUrl(QUrl(httpRequest));
    netReply = manager.get(netRequest);
    QTimer timer;
    QObject::connect(netReply, SIGNAL(finished()), &loop, SLOT(quit()));
    QObject::connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    timer.start(10000);
    loop.exec();

    if (netReply->error() != QNetworkReply::NoError) {
        return jsonObject;
    }

    QByteArray strRateAll = netReply->readAll();
    qDebug() << "http result is" << strRateAll;

    QJsonDocument doc = QJsonDocument::fromJson(strRateAll);
    if (!doc.isNull() && doc.isObject()) {
        jsonObject = doc.object();
        
        // 访问所有字段，包括null值的data
        int code = jsonObject["code"].toInt();
        QString message = jsonObject["message"].toString();
        QJsonValue data = jsonObject["data"]; // 保持为QJsonValue
        qint64 timestamp = jsonObject["timestamp"].toVariant().toLongLong();
        QString lastRequestTime = jsonObject["lastRequestTime"].toString();
    }

    // 检查jsonObject中是否有data字段
    if (jsonObject.contains("data") && jsonObject["data"].isArray()) {
        QJsonArray dataArray = jsonObject["data"].toArray();

        // 遍历data数组中的所有对象
        for (int i = 0; i < dataArray.size(); ++i) {
            QJsonObject item = dataArray[i].toObject();
            QString packageNames = item["packageName"].toString();

            // 分割包名字符串（根据换行符分割）
            QStringList packageList = packageNames.split("\n");

            QStringList driverNames;
            QStringList versions;

            // 为每个包获取驱动名称和版本
            for (const QString &packageName : packageList) {
                QString trimmedPackageName = packageName.trimmed();
                if (!trimmedPackageName.isEmpty()) {
                    driverNames.append(getDriverName(trimmedPackageName));
                    versions.append(getDriverVersion(trimmedPackageName));
                }
            }

            QStringList driverList;
            for (int i = 0; i < driverNames.length(); ++i) {
                QString tmp = driverNames[i] + "=" + versions[i];
                driverList.append(tmp);
            }
            // 用逗号连接所有的驱动名称和版本
            item["driver"] = driverList.join(",");

            // 替换数组中的当前元素
            dataArray[i] = item;
        }

        // 更新jsonObject中的data数组
        jsonObject["data"] = dataArray;
    }
    qDebug() << "--------------------------";
    qDebug() << "在线获取结果：";
    qDebug() << jsonObject;
    qDebug() << "--------------------------";


    return jsonObject;
}


QString DeviceInfoDatabase::getVersionFromDeb(const QString &debName)
{
    QString deb(debName);
    if (!deb.contains("_")) {
        return "";
    }

    deb.remove(0, deb.indexOf("_") + 1);

    if (!deb.contains("_")) {
        return "";
    }

    QString debVersion = deb.left(deb.indexOf("_"));
    return debVersion;
}

static QString getDriverName(QString driverName){
    if (driverName.contains(".deb"))
    {
        QString res = QString(QString(driverName).left(QString(driverName).indexOf("_")));
        return res;
    }else{
        QString arch = getDebianArchitecture();
        if(arch.trimmed() == "amd64"){
        arch = "x86_64";
        }
        
        QString res = QString(driverName.split(arch)[0]).remove("_");
        return res;
    }
}

static QString getDriverVersion(QString driverName){
    if (driverName.contains(".deb"))
    {
        QString res = QString(QString(driverName).split("_")[1]);
        return res;
    }else{
        QString arch = getDebianArchitecture();
        if(arch.trimmed() == "amd64"){
            arch = "x86_64";
        }
        arch = "_" + arch + "_";
        QString tmp = driverName.split(arch)[1];
        QString res = QString(tmp.split("_stable")[0]);
        return res;
    }
}
