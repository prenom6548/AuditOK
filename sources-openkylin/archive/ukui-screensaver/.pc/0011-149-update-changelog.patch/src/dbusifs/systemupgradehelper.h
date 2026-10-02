/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
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
#ifndef SYSTEMUPGRADEHELPER_H
#define SYSTEMUPGRADEHELPER_H

#include <QDBusInterface>
#include <QDBusConnection>
#include <QVariantMap>
#include <QObject>

class SystemUpgradeHelper : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief 构造
     *
     * @param parent 父指针
     */
    explicit SystemUpgradeHelper(QObject *parent = nullptr);
    /**
     * @brief 析构
     *
     */
    virtual ~SystemUpgradeHelper();

    bool checkSystemUpgrade();
    bool doUpgradeThenRboot();
    bool doUpgradeThenShutdown();

private:
private:
    QDBusInterface *m_sysUpgradeInterface = nullptr;
    bool isDbusActive = false;
    bool isSystemUpgrade = false;
};

#endif // SYSTEMUPGRADEHELPER_H
