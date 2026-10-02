/*
* Copyright (C) 2026, KylinSoft Co., Ltd.
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

#ifndef UKUI_SIDEBAR_SIDEBAR_MAIN_H
#define UKUI_SIDEBAR_SIDEBAR_MAIN_H

#include <QObject>
#include <QVariantMap>

class SidebarMain : public QObject
{
    Q_OBJECT
public:
    enum Request {
        Active,
        Show,
        Hide,
        DirectlyHide
    };

    explicit SidebarMain(QObject *parent = nullptr);

    void requestWidgetActive(const QString &id, bool showReturnButton);

    static void resetRecords();
    static QStringList calls();
    static QString lastWidgetId();
    static bool lastShowReturnButton();
    static QString lastParsedMessage();

public Q_SLOTS:
    void parseMessage(const QString &msg);
    void requestSidebar(SidebarMain::Request request);
    void requestShortcut(SidebarMain::Request request);
    void requestNotificationCenter(SidebarMain::Request request);

Q_SIGNALS:
    void stateChange(const QString &module, const QVariantMap &data);
};

#endif
