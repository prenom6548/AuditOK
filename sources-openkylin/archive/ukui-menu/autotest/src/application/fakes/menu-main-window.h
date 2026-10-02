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

#ifndef AUTOTEST_APPLICATION_FAKE_MENU_MAIN_WINDOW_H
#define AUTOTEST_APPLICATION_FAKE_MENU_MAIN_WINDOW_H

#include <QQuickView>

namespace UkuiMenu {

class WindowModule
{
public:
    static void defineModule(const char *uri, int versionMajor, int versionMinor);
};

class MenuWindow : public QQuickView
{
    Q_OBJECT
public:
    explicit MenuWindow(QWindow *parent = nullptr);
    MenuWindow(QQmlEngine *engine, QWindow *parent);

    virtual void activeMenuWindow(bool active);
    bool windowVisible() const;
    QList<bool> history() const;

Q_SIGNALS:
    void windowVisibleChanged();

protected:
    bool m_windowVisible = false;
    QList<bool> m_history;
};

class MenuWindowWayland : public MenuWindow
{
    Q_OBJECT
public:
    using MenuWindow::MenuWindow;
};

class MenuWindowXcb : public MenuWindow
{
    Q_OBJECT
public:
    using MenuWindow::MenuWindow;
};

} // namespace UkuiMenu

#endif
