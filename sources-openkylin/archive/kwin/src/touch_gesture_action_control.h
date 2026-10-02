#ifndef GESTUREBASEACTION_H
#define GESTUREBASEACTION_H

#include "xmlreader.h"

#include <QDBusAbstractInterface>
#include <QObject>

// class Display;
class QString;
namespace KWin
{

class KylinStatusInterface : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    ~KylinStatusInterface();
    static KylinStatusInterface *getInstance();
    bool isTabletMode();

public Q_SLOTS:
    bool setTabletMode(bool isTablet);

private:
    static KylinStatusInterface *s_self;
    static bool isInterfaceInstalled;
    KylinStatusInterface(const QDBusConnection &connection, QObject *parent);
};

class KWIN_EXPORT GestureAndActionBase : public QObject
{
    Q_OBJECT
public:
    GestureAndActionBase(GestureInfo gestureInfo, GestureConfig gestureConfig);

    ~GestureAndActionBase();
    friend class KylinStatusInterface;

    bool operator==(GestureAndActionBase &value);
    GestureType getGestureType()
    {
        return m_gestureType;
    }

    TouchDevice getDevice()
    {
        return m_touchDevice;
    }

    KWin::GlobalGesture::GestureDirection getGestureDirection()
    {
        return m_gestureDirection;
    }
    int getFingers() const
    {
        return m_fingers;
    }
    ActionType getGestureActionType()
    {
        return m_actionType;
    }
    QMap<QString, QString> &getGestureActionSetting()
    {
        return m_gestureAction;
    }
    GestureBlacklist getBlacklist()
    {
        return m_blacklist;
    }
    int getTimeMore()
    {
        return m_timemore;
    }
    int getTimeLess()
    {
        return m_timeless;
    }
    int getSwipeMore()
    {
        return m_swipemore;
    }
    int getSwileLess()
    {
        return m_swipeless;
    }

    quint32 getTimeOut();
    quint32 getMinSwipeDistance();
    bool getEventStage(QString str);
    bool isRepeatable();
    bool isOnlyTablet();

private:
    bool isNeedConnectX11();
    bool isNeedEmulateKeyOrButton();
    static void setTabletMode(bool isTablet);

    // 模拟键盘Key事件
    void sendKeyEvent(const QStringList &keycodes, bool is_press) const;
    // 模拟鼠标Button事件
    void sendButtonEvent(quint32 button, bool is_press) const;
    void runCommand(QString cmdline);

public Q_SLOTS:
    void onGestureActionStart();
    void onGestureActionReach();
    void onGestureActionTriggered();
    void getSignal(bool mode);
    void onAllDirectionKey(GlobalGesture::GestureDirection direction);

private:
    void burriedTouchpadData(GestureType gestureType, quint32 finger, const QString &triggerTime) const;
    void burriedData(const QList<QPair<QString, QString>> &properties) const;

    GestureType m_gestureType;
    TouchDevice m_touchDevice;
    quint32 m_fingers;
    KWin::GlobalGesture::GestureDirection m_gestureDirection;
    ActionType m_actionType;
    QMap<QString, QString> m_gestureAction;
    GestureBlacklist m_blacklist;
    quint32 m_timemore;
    quint32 m_timeless;
    quint32 m_swipemore;
    quint32 m_swipeless;
    bool m_isNeedX;
    /**
     * X11 connection.
     */
    // Display* m_display;
    static KylinStatusInterface *s_interface;
    static bool s_isTabletMode;
};

}

#endif // GESTUREBASEACTION_H
