/* -*- Mode: C++; indent-tabs-mode: nil; tab-width: 4 -*-
 * -*- coding: utf-8 -*-
 *
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MEDIAKEYCANCEL_H
#define MEDIAKEYCANCEL_H

#include <QObject>
#include <QDBusReply>
#include <QDBusInterface>
#include <QDBusConnection>

class MediaKeyCancel : public QObject
{
    Q_OBJECT
public:
    explicit MediaKeyCancel(QObject *parent = nullptr);
    /**
     * @brief 注销当前组合所有kglobalaccel绑定
     * @param 组合
     */
    void unRegisterAll(const QString& component);
private:
    /**
     * @brief 获取所有actionid
     * @return
     */
    const QStringList getActionIds();
    /**
     * @brief 获取组合路径
     * @return
     */
    const QString getComponentPath();
    /**
     * @brief 注销actionId 绑定的键
     * @param actionId
     */
    void unRegisterShortcut(const QString& actionId);

private:
    QDBusInterface *m_kglobalaccel = nullptr;
    QString m_component;
};

#endif // MEDIAKEYCANCEL_H
