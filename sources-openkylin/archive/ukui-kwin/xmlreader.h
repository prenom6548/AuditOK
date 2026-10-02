#ifndef XMLREADER_H
#define XMLREADER_H

#include <QFile>
#include <QDir>
#include <QXmlStreamReader>
#include <QMap>
#include <QPair>
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
enum class TouchDevice
{
    TouchScreen,
    TouchPad,
    NOT_SUPPORT
};
enum class ActionType {
    SEND_KEYS,
    RUN_COMMAND,
    SEND_BUTTON,
    DBUS_ACTION,
    DBUS_SIG,
    NOT_SUPPORTED
};

// 手势类型和枚举类型之间的转换
GestureType strToGestureType(QString& gestureType);
QString gestureTypeToStr(GestureType gestureType);
// 手势方向和枚举类型之间的转换
QString gestureDirectionToStr(KWin::Gesture::GestureDirection gestureDirection);
KWin::Gesture::GestureDirection strToGestureDirection(QString& direction);
// 手势触发动作和枚举类型之间的转换
QString actionTypeToStr(ActionType actionType);
ActionType strToActionType(QString& actionType);

QString touchDeviceTostr(TouchDevice touchDevice);
TouchDevice strToTouchDevice(QString& touchdevice);




class XMLReader
{
public:
    ~XMLReader(){
        if(file)
        {
            delete file;
            file = nullptr;
        }
    }
    static XMLReader* getInstance();

    // not used!
    bool read();
    // 解析手势的配置文件
    bool phraseXml();
    // 获取用户的家目录的绝对路径
    static QString getHomePath() {
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
                           ActionType actionType, QMap<QString, QString> &actionSettings);

    // 将配置文件中被指定为enable=true的手势注册到数据成员中去；
    QMap<QList<QString>, QPair<ActionType, QMap<QString, QString>>> getGestureConfig(){
        return gestureConfig;
    }

    // 清空缓存中的数据
    bool clearGestureConfig() {
        gestureConfig.clear();
        if(gestureConfig.size() == 0) {
            return true;
        }
        else {
            return false;
        }
    }

    // 将手势类型、手指数、手势方向的enum指拼接成字符串，便于存储；
    QString arrangeGestureConfigToEnumStr(GestureType gestureType,
                                          QString &fingers,
                                          KWin::Gesture::GestureDirection gestureDirection);

    // 失败时获取错误信息
    QString errorString() const;

private:
    XMLReader();
    static XMLReader* s_self;
    QXmlStreamReader reader;
    QFile* file;
    QMap<QList<QString>, QPair<ActionType, QMap<QString, QString>>> gestureConfig;
};

#endif // XMLREADER_H
