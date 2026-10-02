/*
* Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 *  This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: iaom <zhangpengfei@kylinos.cn>
 */

#ifndef UKUI_FRAMEWORK_BACKUP_SERVICE_PRIVATE_H
#define UKUI_FRAMEWORK_BACKUP_SERVICE_PRIVATE_H
#include <QObject>
namespace UkuiConfigBackup {
class BackupServicePrivate : public QObject
{
    Q_OBJECT

public:
    explicit BackupServicePrivate(QObject* parent = nullptr);
    ~BackupServicePrivate();

    QString GetBackupConfig(const QStringList &modules, const QString &name);
    QStringList SyncConfig(const QString &config);

    static QString backupServicePath() { return QStringLiteral("/org/ukui/ConfigBackupService"); }
    static QString backupServiceName() { return QStringLiteral("org.ukui.ConfigBackupService"); }

    QString saveCache(const QJsonObject &config, QString pathPrefix);
    QJsonObject loadConfig(const QString &config);

    QMetaType::Type propertyTypeOfList(const QString &path, const QString &interface, const QString &property);
    template <typename T>
    QList<T> convertVariantListToQList(QVariantList list, bool *ok = nullptr);

};

template<typename T>
QList<T> BackupServicePrivate::convertVariantListToQList(QVariantList variantList, bool* ok)
{
    QList<T> resultList;
    bool success = true;
    for (const QVariant& variant : variantList) {
        if (variant.canConvert<T>()) {
            T value = variant.value<T>();
            resultList.append(value);
        } else {
            qWarning() << "Cannot convert variant to target type. Variant type:" << variant.typeName();
            success = false;
            break;
        }
    }
    if (ok) {
        *ok = success;
    }
    return resultList;
}
}
#endif //UKUI_FRAMEWORK_BACKUP_SERVICE_PRIVATE_H
