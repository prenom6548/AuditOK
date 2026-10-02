/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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
 */

#ifndef MENU_DBUS_SERVICE_H
#define MENU_DBUS_SERVICE_H

#include <QObject>
#include <QDBusContext>

class MenuAdaptor;
class QDBusServiceWatcher;

namespace UkuiMenu {

class MenuDbusService : public QObject, public QDBusContext
{
    Q_OBJECT
    Q_PROPERTY(bool Visible READ visible)
public:
    explicit MenuDbusService(const QString& display, QObject *parent = nullptr);

public Q_SLOTS:
    void WinKeyResponse();
    void active(const QString &display);

    bool visible() const;
    void setVisible(bool visible);

    void sendPropertiesChangedSignal(const QString& propName, const QVariant& propValue);

Q_SIGNALS:
    void activeRequest(QString display);
    void menuActive();

private Q_SLOTS:
    void activeMenu(QString display);
    void onServiceOwnerChanged(const QString &service, const QString &oldOwner, const QString &newOwner);

private:
    static QString displayFromPid(uint pid);
    bool registerService();

    void initWatcher();
    void connectToActiveRequest();
    void disConnectActiveRequest();

private:
    QString m_display {};
    bool m_visible {false};
    QDBusServiceWatcher *m_watcher {nullptr};
};

}

#endif // MENU_DBUS_SERVICE_H
