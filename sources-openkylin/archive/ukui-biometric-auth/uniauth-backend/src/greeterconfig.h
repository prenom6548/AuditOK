/*
 * Copyright (C) 2024 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#ifndef GREETERCONFIG_H
#define GREETERCONFIG_H

#include <QSettings>

class GreeterConfig : public QObject
{
    Q_OBJECT
public:
    explicit GreeterConfig(QObject *parent = nullptr);

    void setValue(const QString &group, const QString &key, const QVariant &value, QString strUserName = "");

    QVariant getValue(const QString &group, const QString &key, QVariant varDefault, QString strUserName = "");

    bool resetValue(const QString &group, const QString &key, QString strUserName = "");

    QString printConfig(int nType, QString strUserName = "");

Q_SIGNALS:
    void changed(const QString &group, const QString &key);

    void changedForUser(const QString &group, const QString &key, const QString &userName);

private:
    void initData();

    void loadConfigDirectory(QString strPath);

    void loadConfigFromFile(QString strPath);

    QString getUserHomePath(const QString &strUserName);

private:
    QSettings *m_setttingsDefault;
    QSettings *m_settingsCustom;
    QString m_strHomePath;
};

#endif // LOGINCONFIG_H
