#pragma once

#include <QObject>

class QWidget;
class QWindow;

struct ukui_surface_v1;

class UkuiWindowHelper : public QObject
{
    Q_OBJECT
public:
    explicit UkuiWindowHelper(QWidget *widget);
    explicit UkuiWindowHelper(QWindow *window);
    ~UkuiWindowHelper();

    enum class WindowRole {
        Normal,
        Desktop,
        Panel,
        OnScreenDisplay,
        Notification,
        Tooltip,
        CriticalNotification,
        AppletPopup,
        ScreenLock,
        Watermark,
        SystemWindow,
        InputPanel,
        Logout,
        ScreenLockNotification,
        Switcher,
        Authentication
    };
    Q_ENUM(WindowRole)

    enum class State : uint32_t {
        Minimizable = 0x1,
        Maximizable = 0x2,
        Closeable = 0x4,
        Fullscreenable = 0x8,
        Movable = 0x10,
        Resizable = 0x20,
        Focusable = 0x40,
        Activatable = 0x80,
        KeepAbove = 0x100,
        KeepBelow = 0x200,
    };
    Q_DECLARE_FLAGS(States, State)

    enum class DecorationState : uint32_t {
        RoundCorner = 0x1,
        Border = 0x2,
        Shadow = 0x4,
    };
    Q_DECLARE_FLAGS(DecorationStates, DecorationState)

    enum class Position {
        Top,
        Bottom,
        Left,
        Right
    };
    Q_ENUM(Position)

    enum class DecorationComponent : uint32_t {
        RoundCorner = 0x1,
        Border = 0x2,
        Shadow = 0x4,
    };
    Q_DECLARE_FLAGS(DecorationComponents, DecorationComponent)

    void removeTitleBar();
    void setWindowRole(WindowRole role);
    void setWindowState(States state);
    void setWindowIcon(const QString &iconName);
    void setSkipTaskBar(bool skip);
    void setSkipSwitcher(bool skip);
    void setPanelAutoHide(bool autoHide);
    void grabKeyboard();
    void openUnderCursor(QPoint pos);
    void setBlurEffect(QRegion region, int level = 0, bool enable = true);
    void setSlideEffect(Position position, int offset = 0, bool enable = true);
    States defaultState() const;

    friend inline State operator&(State lhs, State rhs)
    {
        return static_cast<State>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
    }

    friend inline State operator~(State rhs)
    {
        return static_cast<State>(~static_cast<uint32_t>(rhs));
    }

    Q_DECL_DEPRECATED_X("Use UkuiWindowHelper::showTileFlyout(QRect rect) instead")
    void showTileFlyout();
    Q_DECL_DEPRECATED_X("Use UkuiWindowHelper::setDecorationCompoents(DecorationComponents components) instead")
    void setDecorationState(DecorationStates state);

    /**
     * @brief showTileFlyout
     * Show the tile flyout, the flyout will be closed when the poiner leave the given rect.
     * @param rect The rect to show the flyout.
     * @note The rect is in the coordinate of the window.
     */
    void showTileFlyout(QRect rect);
    void setDecorationCompoents(DecorationComponents components);

private:
    QWidget *m_widget = nullptr;
    QWindow *m_window = nullptr;
    struct ukui_surface_v1 *m_ukuiSurfacev1 = nullptr;
};
