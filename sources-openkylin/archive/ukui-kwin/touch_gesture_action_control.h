#ifndef GESTUREBASEACTION_H
#define GESTUREBASEACTION_H

#include "xmlreader.h"
#include <QDebug>
//#include <ukui-kwin_export.h>

#include <QObject>
#include "X11/Xlib.h"
#include <QDBusAbstractInterface>
#include <memory>
#include <QString>
namespace KWin
{

class KylinStatusInterface : public QDBusAbstractInterface
{
    Q_OBJECT
public:
    ~KylinStatusInterface();
    static KylinStatusInterface* getInstance();
    bool isTabletMode();

public slots:
    bool setTabletMode(bool isTablet);

private:
    static KylinStatusInterface* s_self;
    static bool isInterfaceInstalled;
    KylinStatusInterface(const QDBusConnection &connection, QObject *parent);
};

class UKUI_KWIN_EXPORT GestureAndActionBase : public QObject
{
    Q_OBJECT
public:
    GestureAndActionBase(QList<QString> gesture, QPair<ActionType, QMap<QString, QString>> gestureAction);

    ~GestureAndActionBase();
    friend class KylinStatusInterface;

    bool operator ==(GestureAndActionBase& value);
    GestureType getGestureType()
    {
        return m_gestureType;
    }

    TouchDevice getDevice()
    {
        return m_touchDevice;
    }

    KWin::Gesture::GestureDirection getGestureDirection(){
        return m_gestureDirection;
    }
    int getFingers() const {
        return m_fingers;
    }
    ActionType getGestureActionType(){
        return m_actionType;
    }
    QMap<QString, QString>& getGestureActionSetting(){
        return m_gestureAction;
    }

    quint32 getTimeOut();
    quint32 getMinSwipeDistance();
    bool getEventStage(QString str);
    bool isRepeatable();
    bool isOnlyTablet();


    // 模拟键盘Key事件
    void sendKeyEvent(const QStringList &keycodes, bool is_press) const;
    // 模拟鼠标Button事件
    void sendButtonEvent(quint32 button, bool is_press) const;

private:
    bool isNeedConnectX11();
    bool isNeedEmulateKeyOrButton();
    static void setTabletMode(bool isTablet);

public slots:
    void onGestureActionStart();
    void onGestureActionReach();
    void onGestureActionTriggered();
    void getSignal(bool mode);
    void onAllDirectionKey(Gesture::GestureDirection direction);

private:
    GestureType m_gestureType;
    TouchDevice m_touchDevice;
    quint32 m_fingers;
    KWin::Gesture::GestureDirection m_gestureDirection;
    ActionType m_actionType;
    QMap<QString, QString> m_gestureAction;
    bool m_isNeedX;
    /**
     * X11 connection.
     */
    Display* m_display;
    static KylinStatusInterface* s_interface;
    static bool s_isTabletMode;
};

}

#endif // GESTUREBASEACTION_H
