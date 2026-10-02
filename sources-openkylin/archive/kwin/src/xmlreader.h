#ifndef XMLREADER_H
#define XMLREADER_H

#include "globalgestures.h"

#include <QDir>
#include <QFile>
#include <QMap>
#include <QPair>
#include <QXmlStreamReader>
#include <gestures.h>

enum class GestureType {
    EDGESWIPE,
    SWIPE,
    PINCH,
    LONGPRESS,
    TAP,
    SWIPESEQUENCE,
    NOT_SUPPORTED,
};
enum class TouchDevice {
    TouchScreen,
    TouchPad,
    NOT_SUPPORT
};
enum class ActionType {
    SEND_KEYS,
    RUN_COMMAND,
    SEND_BUTTON,
    DBUS_ACTION,
    NOT_SUPPORTED
};

// 手势类型和枚举类型之间的转换
GestureType strToGestureType(QString &gestureType);
QString gestureTypeToStr(GestureType gestureType);
// 手势方向和枚举类型之间的转换
QString gestureDirectionToStr(KWin::GlobalGesture::GestureDirection gestureDirection);
KWin::GlobalGesture::GestureDirection strToGestureDirection(QString &direction);
// 手势触发动作和枚举类型之间的转换
QString actionTypeToStr(ActionType actionType);
ActionType strToActionType(QString &actionType);

QString touchDeviceTostr(TouchDevice touchDevice);
TouchDevice strToTouchDevice(QString &touchdevice);

struct GestureAction
{
    GestureAction(ActionType actionType, QMap<QString, QString> actionSettings)
        : type(actionType)
        , settings(actionSettings)
    {
    }

    GestureAction()
        : type(ActionType::NOT_SUPPORTED)
        , settings(QMap<QString, QString>())
    {
    }

    ActionType type;
    QMap<QString, QString> settings;
};

struct GestureBlacklist
{
    GestureBlacklist(QStringList proclist = QStringList(), QStringList cmdlist = QStringList())
        : procNames(proclist)
        , cmdlines(cmdlist)
    {
    }

    QStringList procNames;
    QStringList cmdlines;
};

struct GestureConfig
{
    GestureConfig(GestureAction gestureAction,
                  GestureBlacklist gestureBlacklist = GestureBlacklist())
        : action(gestureAction)
        , blacklist(gestureBlacklist)
    {
    }

    GestureConfig()
        : action(GestureAction())
        , blacklist(QStringList())
    {
    }

    GestureAction action;
    GestureBlacklist blacklist;
};

struct GestureInfo
{
    GestureInfo(QString gestureType, QString fingerCount,
                QString gestureDirection, QString gestureDevice,
                QString timemore, QString timeless,
                QString swipemore, QString swipeless)
        : type(gestureType)
        , fingers(fingerCount)
        , direction(gestureDirection)
        , device(gestureDevice)
        , timemore(timemore)
        , timeless(timeless)
        , swipemore(swipemore)
        , swipeless(swipeless)
    {
    }

    inline bool operator<(const GestureInfo &rhs) const
    {
        QStringList llist = {this->type, this->fingers, this->direction, this->device,
                             this->timemore, this->timeless, this->swipemore, this->swipeless};
        QStringList rlist = {rhs.type, rhs.fingers, rhs.direction, rhs.device,
                             rhs.timemore, rhs.timeless, rhs.swipemore, rhs.swipeless};
        return llist < rlist;
    }

    QString type;
    QString fingers;
    QString direction;
    QString device;
    QString timemore;
    QString timeless;
    QString swipemore;
    QString swipeless;
};

class XMLReader
{
public:
    ~XMLReader()
    {
        if (file) {
            delete file;
            file = nullptr;
        }
    }
    static XMLReader *getInstance();

    // not used!
    bool read();
    // 解析手势的配置文件
    bool phraseXml();
    // 获取用户的家目录的绝对路径
    static QString getHomePath()
    {
        return QDir::homePath();
    }
    // 获取配置文件所在目录的绝对路径
    QString getConfigPath() const;
    // 获取配置文件的绝对路径
    static QString getConfigFile();

    // 如果家目录不存在配置文件，则从/usr/share下面拷贝一份到家目录
    bool copyConfigIfNotPresent();

    // 将配置文件中被指定为enable=true的手势注册到数据成员中去；
    void saveGestureConfig(QString gestureType,
                           QString &fingers,
                           QString gestureDirection,
                           QString touchDevice,
                           QString timemore, QString timeless, QString swipemore, QString swipeless,
                           ActionType actionType, QMap<QString, QString> &actionSettings,
                           GestureBlacklist blacklist);
    // 将配置文件中被指定为enable=true的手势注册到数据成员中去；
    QMap<GestureInfo, GestureConfig> getGestureConfig()
    {
        return gestureConfig;
    }

    // 清空缓存中的数据
    bool clearGestureConfig()
    {
        gestureConfig.clear();
        if (gestureConfig.size() == 0) {
            return true;
        } else {
            return false;
        }
    }

    // 将手势类型、手指数、手势方向的enum指拼接成字符串，便于存储；
    QString arrangeGestureConfigToEnumStr(GestureType gestureType,
                                          QString &fingers,
                                          KWin::GlobalGesture::GestureDirection gestureDirection);

    // 失败时获取错误信息
    QString errorString() const;

private:
    XMLReader();
    static XMLReader *s_self;
    QXmlStreamReader reader;
    QFile *file;
    QMap<GestureInfo, GestureConfig> gestureConfig;
};

#endif // XMLREADER_H
