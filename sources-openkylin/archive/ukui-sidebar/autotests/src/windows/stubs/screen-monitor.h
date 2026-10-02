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

#ifndef AUTOTESTS_WINDOWS_STUB_SCREEN_MONITOR_H
#define AUTOTESTS_WINDOWS_STUB_SCREEN_MONITOR_H

#include <QObject>
#include <QPointer>
#include <QScreen>

namespace Sidebar {

class ScreenMonitor : public QObject
{
    Q_OBJECT
public:
    static ScreenMonitor *getInstance();

    bool getScreenLockState() const;
    QScreen *getPrimaryScreen() const;
    QRect getGeometry() const;
    Qt::LayoutDirection getLayoutDirection() const;
    int getPanelPosition() const;
    int getPanelSize() const;

    void setScreenLockState(bool state);
    void setPrimaryScreen(QScreen *screen);
    void setLayoutDirection(Qt::LayoutDirection direction);
    void setPanelPosition(int position);
    void setPanelSize(int size);
    void emitPanelPropertyChanged();
    void reset();

Q_SIGNALS:
    void primaryScreenChanged();
    void geometryChanged(const QRect &geometry);
    void panelPropertyChanged();
    void layoutDirectionChanged();
    void screenLockStateChanged(bool state);
    void screenRemoved(QScreen *screen);

private:
    explicit ScreenMonitor(QObject *parent = nullptr);

    bool m_screenLockState = false;
    QPointer<QScreen> m_primaryScreen = nullptr;
    Qt::LayoutDirection m_direction = Qt::LeftToRight;
    int m_panelPosition = 0;
    int m_panelSize = 0;
};

} // namespace Sidebar

#endif
