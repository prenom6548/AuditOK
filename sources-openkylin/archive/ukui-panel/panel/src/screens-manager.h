/*
 *  * Copyright (C) 2024, KylinSoft Co., Ltd.
 *  *
 *  * This program is free software: you can redistribute it and/or modify
 *  * it under the terms of the GNU General Public License as published by
 *  * the Free Software Foundation, either version 3 of the License, or
 *  * (at your option) any later version.
 *  *
 *  * This program is distributed in the hope that it will be useful,
 *  * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  * GNU General Public License for more details.
 *  *
 *  * You should have received a copy of the GNU General Public License
 *  * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *  *
 *  * Authors: iaom <zhangpengfei@kylinos.cn>
 *
 */

#ifndef UKUI_PANEL_SCREENS_MANAGER_H
#define UKUI_PANEL_SCREENS_MANAGER_H
#include <QObject>
#include <QScreen>
namespace UkuiPanel {
class ScreensManager;
class Screen : public QObject
{
    Q_OBJECT
public:
    explicit Screen(QScreen *screen, QObject *parent = nullptr);
    Q_DISABLE_COPY(Screen);

public:
    QRect geometry() const;
    QScreen * internal();
    bool enable();
    void setEnable(bool enable);
    bool isValid();
Q_SIGNALS:
    void geometryChanged(const QRect &geometry);
    void enableChanged(bool enable);

private:
    QScreen *m_screen = nullptr;
    bool m_enable = false;
};
class ScreensManager : public QObject
{
    Q_OBJECT
public:
    explicit ScreensManager(QObject *parent = nullptr);
    ~ScreensManager();
    QList<Screen *> screens();
    Screen * primaryScreen();

Q_SIGNALS:
    void screenAdded(Screen *);
    void screenRemoved(Screen *);
    void primaryScreenChanged(Screen *);

private Q_SLOTS:
    void onScreenAdded(QScreen *screen);
    void onScreenRemoved(QScreen *screen);
    void onPrimaryScreenChanged(QScreen *screen);
    void onEnableChanged(Screen *screen, bool enable);
    void checkScreensGeometryStatus(Screen *screen);
private:
    Screen *m_primaryScreen = nullptr;
    QList<Screen *> m_screens;

};

} // UkuiPanel

#endif //UKUI_PANEL_SCREENS_MANAGER_H
