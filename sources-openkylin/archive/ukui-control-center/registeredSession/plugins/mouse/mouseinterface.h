#ifndef MOUSEINTERFACE_H
#define MOUSEINTERFACE_H
#include <QDBusConnection>
#include <QDBusReply>
#include <QDBusArgument>
#include <QString>
#include <QDebug>

#include <ukui-service-interface/serviceobject.h>
#include "../../universalinterface.h"

// 定义结构体来匹配D-Bus结构
struct EffectStruct {
    QString name;
    quint32 value;
    bool flag;

    // 必须添加序列化和反序列化操作符
    friend QDBusArgument &operator<<(QDBusArgument &argument, const EffectStruct &myStruct) {
        argument.beginStructure();
        argument << myStruct.name << myStruct.value << myStruct.flag;
        argument.endStructure();
        return argument;
    }

    friend const QDBusArgument &operator>>(const QDBusArgument &argument, EffectStruct &myStruct) {
        argument.beginStructure();
        argument >> myStruct.name >> myStruct.value >> myStruct.flag;
        argument.endStructure();
        return argument;
    }
};

// 需要在qRegisterMetaType之前声明
Q_DECLARE_METATYPE(EffectStruct)

class MouseInterface : public UKUI::ServiceObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.ukui.ukcc.session.Mouse")
    Q_PROPERTY(bool dominantHand READ getDominantHand)
    Q_PROPERTY(bool scrollDirection READ getScrollDirection)
    Q_PROPERTY(int wheelSpeed READ getWheelSpeed)
    Q_PROPERTY(int doubleClick READ getDoubleClick)
    Q_PROPERTY(double pointerSpeed READ getPointerSpeed)
    Q_PROPERTY(bool mouseAcceleration READ getMouseAcceleration)
    Q_PROPERTY(bool tailLightEffect READ getTailLightEffect)
    Q_PROPERTY(bool pointerPosition READ getPointerPosition)
    Q_PROPERTY(bool shakeCursor READ getShakeCursor)
    Q_PROPERTY(int pointerSize READ getPointerSize)
    Q_PROPERTY(bool cursorBlink READ getCursorBlink)
    Q_PROPERTY(int cursorSpeed READ getCursorSpeed)

public:
    MouseInterface();
    ~MouseInterface();

    void initConnect();

private:
    QString DOMINANT_HAND_KEY      = QString("leftHanded");
    QString SCROLL_DIRECTION_KEY   = QString("naturalScroll");
    QString WHEEL_SPEED_KEY        = QString("wheelSpeed");
    QString DOUBLECLICK_KEY        = QString("doubleClick");
    QString POINTER_SPEED_KEY      = QString("motionAcceleration");
    QString MOUSE_ACCELERATION_KEY = QString("mouseAccel");
    QString POINTER_POSITION_KEY   = QString("locatePointer");
    QString SHAKE_CURSOR          = QString("shakeCursor");
    QString POINTER_SIZE_KEY       = QString("cursorSize");

    QString CURSOR_BLINK_KEY       = QString("cursorBlink");
    QString CURSOR_SPEED_KEY       = QString("cursorBlinkTime");

    QGSettings         *ukuiMouseGsettings    = nullptr;
    QGSettings         *mateInterfaceGsettings= nullptr;
    QGSettings         *ukuiStyleGsettings    = nullptr;

    QDBusInterface *m_pKwinInterface = nullptr;

protected:
    // 对属性的访问函数
    const bool   getDominantHand();
    const bool   getScrollDirection();
    const int    getWheelSpeed();
    const int    getDoubleClick();
    const double getPointerSpeed();
    const bool   getMouseAcceleration();
    const bool   getTailLightEffect();
    const bool   getPointerPosition();
    const bool   getShakeCursor();
    const int    getPointerSize();
    const bool   getCursorBlink();
    const int    getCursorSpeed();

public Q_SLOTS:
    bool isEffectSupported();
    // 对属性的设置函数
    void setDominantHand(const bool rightHand);
    void setScrollDirection(const bool reverse);
    void setWheelSpeed(const int value);
    void setDoubleClick(const int value);
    void setPointerSpeed(const double value);
    void setMouseAcceleration(const bool checked);
    void setTailLightEffect(const bool checked);
    void setPointerPosition(const bool checked);
    void setShakeCursor(const bool checked);
    void setPointerSize(const int value);
    void setCursorBlink(const bool checked);
    void setCursorSpeed(const int value);
    void setStyleCursorSpeed(const int value);

    void resetKey(QString key);
Q_SIGNALS:
    void changed(QString keyName);

};

#endif // MOUSEINTERFACE_H
