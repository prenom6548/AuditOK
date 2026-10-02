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
#ifndef USDBLOCKSHORTCUT_H
#define USDBLOCKSHORTCUT_H

#include <QObject>
#include <QDBusAbstractAdaptor>
#include <QDBusContext>
#include <QStringList>

class USDBlockShortCut : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.settingsDaemon.shortcut")
    Q_PROPERTY(QStringList blockShortcuts READ blockShortcuts)

public:
    USDBlockShortCut(QObject *parent = nullptr);
    virtual ~USDBlockShortCut();

public slots:
    /**
     * @brief unblockShortcuts 可用按键列表
     * @return 可用按键字符串列表
     */
    QStringList unblockShortcuts();
    /**
     * @brief blockShortcuts 禁用按键列表
     * @return 禁用按键字符串列表
     */
    QStringList blockShortcuts();

private:
    bool m_isRegistService = false; /**是否已注册服务*/
    bool m_isRegistObject = false;  /**是否已注册对象*/
    QString m_strDbusPath;          /**dbus服务路径*/
};

#endif // USDBLOCKSHORTCUT_H
