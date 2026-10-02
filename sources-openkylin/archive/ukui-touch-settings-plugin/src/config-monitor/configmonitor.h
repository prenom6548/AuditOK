/*
 * Copyright (C) 2024 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU  Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#ifndef CONFIGMONITOR_H
#define CONFIGMONITOR_H

#include <QJsonObject>
#include <QMap>
#include <QObject>
#include <QString>

class ConfigMonitor : public QObject
{
    Q_OBJECT

public:
    explicit ConfigMonitor(const QString& configFileName);
    bool getModuleControlledByName(const QString& moduleName) const;
    QJsonValue getModuleConfigByName(const QString& moduleName) const;

private:
    void loadModuleConfig();
    void connectUsdDBusSignal();

    QString getModuleJsonFilePath() const;
    void updateModuleConfig();

    static QString getUserName();
    bool shouldUpdateModuleConfig(const QString& user, const QStringList& moduleNameList) const;

private:

    QJsonObject m_moduleConfigObject;

    const QString m_configFileBaseName;

private Q_SLOTS:
    void onConfigFilesChanged(const QString& user, const QStringList& moduleNameList);

    void clearModuleConfig();

Q_SIGNALS:
    void moduleConfigChanged();
};

#endif
