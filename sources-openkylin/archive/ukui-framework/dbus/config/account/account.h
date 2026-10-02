/*
 * UKUI Framework
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Sisheng He <hesisheng@kylinos.cn>
 *
 */

#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QObject>
#include <QDBusContext>
#include <QDBusReply>
#include <QDBusInterface>

#define FACEPATH "/usr/share/ukui/faces/"
#define CUSTOMFACEPATH "/var/lib/AccountsService/icons/"

class Account : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.Framework.CONFIG.Account")
public:
    explicit Account(QObject *parent = nullptr);
    ~Account();

public slots:
    /**
     * @brief GetIconFile
     * 获取用户头像
     * @param username
     * @return QString
     */
    Q_INVOKABLE const QString GetIconFile(const int uid) const;
    /**
     * @brief SetIconFile
     * 设置用户头像
     * @param username
     * @return
     */
    Q_INVOKABLE void SetIconFile(const int uid, const QString &filename);

signals:
    void IconFileChanged();

private:
    QString renameFaceName(QString fullFilename, QString username);
};

#endif // ACCOUNT_H
