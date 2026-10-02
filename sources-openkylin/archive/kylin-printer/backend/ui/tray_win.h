/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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
 */
#ifndef TRAY_WIN_H
#define TRAY_WIN_H

#include <QObject>
#include <QWidget>
#include <QMainWindow>
#include <QMainWindow>
#include <QString>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>

class TrayWin : public QWidget
{
    Q_OBJECT
public:
    explicit TrayWin(QWidget *parent = nullptr);
public slots:
    void showNotify(QStringList notify);

signals:

private:
    void createTrayIcon();
    void createActions();

    QSystemTrayIcon *m_trayIcon;
    QMenu *m_trayIconMenu;

    QAction *m_minimizeAction;
    QAction *m_maximizeAction;
    QAction *m_quitAction;
};

#endif // TRAY_WIN_H
