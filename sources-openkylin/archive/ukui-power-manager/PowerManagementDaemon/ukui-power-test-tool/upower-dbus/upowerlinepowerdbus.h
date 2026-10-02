/*
 * Copyright 2023 KylinSoft Co., Ltd.
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UPOWERLINEPOWERDBUS_H
#define UPOWERLINEPOWERDBUS_H

#include <QObject>
#include <QtDBus>
#include <QDBusInterface>

class UpowerLinePowerDBus : public QObject
{
    Q_OBJECT
public:
    explicit UpowerLinePowerDBus(int linePowerId, const QString &linePowerDBusPath, QObject *parent = nullptr);
    ~UpowerLinePowerDBus();

private:
    int m_linePowerId;

    QDBusInterface *m_DBusInterface;

    bool m_acOnlineState;

Q_SIGNALS:
    void acOnlineStateChanged(int, bool);

private Q_SLOTS:
    void dealLinePowerPropertiesMessage(QDBusMessage);

public:
    int getAcOnlineState();
};

#endif // UPOWERLINEPOWERDBUS_H
