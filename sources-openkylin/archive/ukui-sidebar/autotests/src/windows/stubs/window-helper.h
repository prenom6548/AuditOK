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

#ifndef AUTOTESTS_WINDOWS_STUB_WINDOW_HELPER_H
#define AUTOTESTS_WINDOWS_STUB_WINDOW_HELPER_H

#include <QObject>
#include <QPoint>
#include <QRegion>
#include <QScreen>
#include <QWindow>

namespace UkuiQuick {

namespace WindowType {
enum Type {
    SystemWindow = 0,
    ScreenLockNotification = 1,
    Notification = 2,
    Switcher = 3
};
}

namespace WindowState {
enum State {
    Activatable = 0x1
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
        BottomEdge,
        LeftEdge,
        RightEdge
    };

    explicit WindowProxy(QWindow *window, Operations operations = {SkipTaskBar | SkipSwitcher | RemoveTitleBar});
    ~WindowProxy() override = default;

    static QScreen *currentScreen();
    static void setCurrentScreen(QScreen *screen);

    void setPosition(const QPoint &point);
    QPoint position() const;

    void setWindowType(WindowType::Type type);
    WindowType::Type windowType() const;

    void setWindowStates(WindowState::States states, bool enable = true);
    bool windowStateEnabled(WindowState::State state) const;

    void setBlurRegion(bool enable = true, const QRegion &region = QRegion());
    bool blurEnabled() const;
    QRegion blurRegion() const;
    int blurCallCount() const;

    void setBlurOpacity(double opacity);
    double blurOpacity() const;

    void slideWindow(SlideFromEdge edge, int offset);
    SlideFromEdge lastSlideEdge() const;
    int lastSlideOffset() const;

private:
    QWindow *m_window = nullptr;
    QPoint m_position;
    WindowType::Type m_windowType = WindowType::SystemWindow;
    WindowState::States m_states;
    bool m_blurEnabled = false;
    QRegion m_blurRegion;
    int m_blurCallCount = 0;
    double m_blurOpacity = 0;
    SlideFromEdge m_lastSlideEdge = NoEdge;
    int m_lastSlideOffset = 0;
};

} // namespace UkuiQuick

Q_DECLARE_OPERATORS_FOR_FLAGS(UkuiQuick::WindowProxy::Operations)
Q_DECLARE_OPERATORS_FOR_FLAGS(UkuiQuick::WindowState::States)

#endif
