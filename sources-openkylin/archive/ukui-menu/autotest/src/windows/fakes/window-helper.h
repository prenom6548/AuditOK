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

#ifndef AUTOTEST_WINDOWS_FAKE_WINDOW_HELPER_H
#define AUTOTEST_WINDOWS_FAKE_WINDOW_HELPER_H

#include <QObject>
#include <QPoint>
#include <QRect>
#include <QRegion>
#include <QScreen>
#include <QWindow>

namespace UkuiQuick {

namespace WindowType {
enum Type {
    SystemWindow = 0
};
}

namespace WindowState {
enum State {
    Maximizable = 0x1
};
Q_DECLARE_FLAGS(States, State)
}

class WindowProxy : public QObject
{
    Q_OBJECT
public:
    enum Operation {
        SkipTaskBar = 0x0001,
        SkipSwitcher = 0x0002,
        RemoveTitleBar = 0x0004
    };
    Q_DECLARE_FLAGS(Operations, Operation)

    enum SlideFromEdge {
        NoEdge = 0,
        TopEdge,
        RightEdge,
        BottomEdge,
        LeftEdge
    };
    Q_ENUM(SlideFromEdge)

    explicit WindowProxy(QWindow *window, Operations operations = {SkipTaskBar | SkipSwitcher | RemoveTitleBar});
    ~WindowProxy() override = default;

    void setPosition(const QPoint &point);
    void setGeometry(const QRect &rect);
    void setWindowType(WindowType::Type type);
    void slideWindow(SlideFromEdge fromEdge, int offset = -1);
    void setBlurRegion(bool enable = true, const QRegion &region = QRegion());
    static QScreen *currentScreen();
    static void setCurrentScreen(QScreen *screen);
    void setWindowStates(WindowState::States states, bool enable = true);
    void setRestoreGeometry(const QRect &rect);

    QPoint position() const;
    QRect geometry() const;
    QRect restoreGeometry() const;
    SlideFromEdge slideEdge() const;
    int slideOffset() const;
    bool blurEnabled() const;
    QRegion blurRegion() const;
    WindowType::Type windowType() const;
    WindowState::States windowStates() const;

private:
    static QScreen *s_currentScreen;

    QPoint m_position;
    QRect m_geometry;
    QRect m_restoreGeometry;
    SlideFromEdge m_slideEdge = NoEdge;
    int m_slideOffset = -1;
    bool m_blurEnabled = false;
    QRegion m_blurRegion;
    WindowType::Type m_windowType = WindowType::SystemWindow;
    WindowState::States m_windowStates;
};

} // namespace UkuiQuick

Q_DECLARE_OPERATORS_FOR_FLAGS(UkuiQuick::WindowProxy::Operations)
Q_DECLARE_OPERATORS_FOR_FLAGS(UkuiQuick::WindowState::States)

#endif
