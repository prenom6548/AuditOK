#pragma once

#include <QObject>

class QWidget;
class QWindow;
class QScreen;

struct UkuiWindowHelperPrivate;

class UkuiWindowHelper : public QObject
{
    Q_OBJECT
public:
    explicit UkuiWindowHelper(QWidget *widget);
    explicit UkuiWindowHelper(QWindow *window);
    ~UkuiWindowHelper();

    enum class WindowRole {
        // UKUI window types which is supported on ukui wayland environment.
        Normal = 0,
        Desktop = 1,
        Panel = 2,
        OnScreenDisplay = 3,
        Notification = 4,
        Tooltip = 5,
        CriticalNotification = 6,
        AppletPopup = 7,
        ScreenLock = 8,
        Watermark = 9,
        SystemWindow = 10,
        InputPanel = 11,
        Logout = 12,
        ScreenLockNotification = 13,
        Switcher = 14,
        Authentication = 15,

        // Other upstream window types contains kde specific window types and standard window types
        // Warning: These types are not supported on ukui wayland environment.
        Override = 101,
        Dialog = 102,
        Menu = 103,
        TopMenu = 104,
        ToolBar = 105,
        Dock = 106,
        Utility = 107,
        Splash = 108,
        DropdownMenu = 109,
        PopupMenu = 110,
        ComboBox = 111,
        DNDIcon = 112,
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
        Left = 0,
        Top = 1,
        Right = 2,
        Bottom = 3
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

    /**
     * @brief requestGetCurrentScreen
     * Request to get the screen where the pointer is located.
     * This is useful when the screen information is not available immediately. You need to
     * connect to the `currentScreenChanged(QScreen *screen)` signal to get the current
     * screen after this request.
     * @note This function is only available on Wayland.
     */
    void requestGetCurrentScreen();

    /**
     * @brief currentScreen
     * Get the current screen where the window is located.
     * If the window has not been mapped to a screen yet, this function may return nullptr.
     * You should connect to the `currentScreenChanged(QScreen *screen)` signal to
     * get the current screen.
     * @return The current screen, or nullptr if not available.
     */
    QScreen *currentScreen() const;

    /**
     * Get the current screen where the window is located.
     * Warning: it's a block function
     */
    QScreen *getCurrentScreenBlocking();

    void setOpaqueRegion(QRegion region);
    void setRestoreGeometry(QRect geometry);

Q_SIGNALS:
    /**
     * @brief currentScreenChanged
     * This signal is emitted after requestedGetCurrentScreen() being called.
     * @param screen The current screen where the window is located.
     */
    void currentScreenChanged(QScreen *screen);

private:
    UkuiWindowHelperPrivate *p = nullptr;

    friend class UkuiWindowHelperPrivate;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(UkuiWindowHelper::States)
Q_DECLARE_OPERATORS_FOR_FLAGS(UkuiWindowHelper::DecorationComponents)
