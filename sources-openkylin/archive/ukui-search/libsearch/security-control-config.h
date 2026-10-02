/*
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
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
 * Authors: baijunjie <baijunjie@kylinos.cn>
 *
 */

#ifndef UKUI_SEARCH_SECURITYCONTROLCONFIG_H
#define UKUI_SEARCH_SECURITYCONTROLCONFIG_H

#include <QObject>
#include <QJsonObject>
#include <QDBusInterface>
#include <QMutex>

namespace UkuiSearch {

class SecurityControlConfig : public QObject
{
    Q_OBJECT
public:
    static SecurityControlConfig& instance();
    bool isUnderSecurityControl() const;
    bool isWhiteListEnable() const;
    bool isBlackListEnable() const;
    QStringList appWhiteList() const;
    QStringList appBlackList() const;
    static QMutex s_mutex;

private Q_SLOTS:
    void onConfigFileChanged(QString userName, QStringList appConfigNames);
    void onSecurityControlExited();

private:
    SecurityControlConfig(QObject *parent = nullptr);
    void parseData(const QString& path);
    void getConfigPath();
    QJsonObject m_dataObject;
    QDBusInterface* m_interface;
    QDBusInterface* m_signalInterface;
    QString m_configPath;
};

} // UkuiSearch

#endif //UKUI_SEARCH_SECURITYCONTROLCONFIG_H
