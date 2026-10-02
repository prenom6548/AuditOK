#include <QDebug>
#include "xmlreader.h"
const char *USR_SHARE_CONFIG_FILE = "/usr/share/touchgesture/touchgesture.xml";
const char *HOME_CONFIG_DIR      = ".config/touchgesture";
const char *CONFIG_FILE          = "touchgesture.xml";

GestureType strToGestureType(QString& gestureType) {
    if (gestureType == "EDGESWIPE") {
        return GestureType::EDGESWIPE;
    }
    else if (gestureType == "SWIPE") {
        return GestureType::SWIPE;
    }
    else if (gestureType == "PINCH") {
        return GestureType::PINCH;
    }
    else if (gestureType == "LONGPRESS") {
        return GestureType::LONGPRESS;
    }
    else if(gestureType == "TAP"){
        return GestureType::TAP;
    }
    else if(gestureType == "SWIPESEQUENCE")
    {
        return GestureType::SWIPESEQUENCE;
    }
    return GestureType::NOT_SUPPORTED;
}
QString gestureTypeToStr(GestureType gestureType) {
    switch (gestureType) {
        case GestureType::EDGESWIPE:
            return "EDGESWIPE";
        case GestureType::SWIPE:
            return "SWIPE";
        case GestureType::PINCH:
            return "PINCH";
        case GestureType::LONGPRESS:
            return "LONGPRESS";
        case GestureType::SWIPESEQUENCE:
            return "SWIPESEQUENCE";
        case GestureType::TAP:
            return "TAP";
        default:
            return "NOT_SUPPORTED";
    }
}
QString gestureDirectionToStr(KWin::Gesture::GestureDirection gestureDirection) {
    switch (gestureDirection) {
        case KWin::Gesture::GestureDirection::Up:
            return "Up";
        case KWin::Gesture::GestureDirection::Down:
            return "Down";
        case KWin::Gesture::GestureDirection::Left:
            return "Left";
        case KWin::Gesture::GestureDirection::Right:
            return "Right";
        case KWin::Gesture::GestureDirection::In:
            return "In";
        case KWin::Gesture::GestureDirection::Out:
            return "Out";
        case KWin::Gesture::GestureDirection::NoDirection:
            return "NoDirection";
        default:
            return "NoDirection";
    }
}
KWin::Gesture::GestureDirection strToGestureDirection(QString& direction) {
    if (direction == "Up") {
        return KWin::Gesture::GestureDirection::Up;
    }
    if (direction == "Down") {
        return KWin::Gesture::GestureDirection::Down;
    }
    if (direction == "Left") {
        return KWin::Gesture::GestureDirection::Left;
    }
    if (direction == "Right") {
        return KWin::Gesture::GestureDirection::Right;
    }
    if (direction == "In") {
        return KWin::Gesture::GestureDirection::In;
    }
    if (direction == "Out") {
        return KWin::Gesture::GestureDirection::Out;
    }
    if (direction == "NoDirection") {
        return KWin::Gesture::GestureDirection::NoDirection;
    }
    return KWin::Gesture::GestureDirection::NoDirection;
}
QString actionTypeToStr(ActionType actionType) {
    switch (actionType) {
        case ActionType::SEND_KEYS:
            return "SEND_KEYS";
        case ActionType::RUN_COMMAND:
            return "RUN_COMMAND";
        case ActionType::SEND_BUTTON:
            return "SEND_BUTTON";
        case ActionType::DBUS_ACTION:
            return "DBUS_ACTION";
        case ActionType::DBUS_SIG:
            return "DBUS_SIG";
        default:
            return "NOT_SUPPORTED";
    }
}
ActionType strToActionType(QString& actionType) {
    if (actionType == "SEND_KEYS") {
        return ActionType::SEND_KEYS;
    }
    if (actionType == "RUN_COMMAND") {
        return ActionType::RUN_COMMAND;
    }
    if (actionType == "SEND_BUTTON") {
        return ActionType::SEND_BUTTON;
    }
    if (actionType == "DBUS_ACTION") {
        return ActionType::DBUS_ACTION;
    }
    if (actionType == "DBUS_SIG") {
        return ActionType::DBUS_SIG;
    }
    return ActionType::NOT_SUPPORTED;
}

QString touchDeviceTostr(TouchDevice touchDevice){
    switch (touchDevice) {
    case TouchDevice::TouchPad:
        return "TOUCHPAD";
    case TouchDevice::TouchScreen:
        return "TOUCHSCREEN";
    default:
        return "NOT_SUPPORT";
    }
}

TouchDevice strToTouchDevice(QString& touchdevice){

    if(touchdevice == "TouchScreen"){
        return TouchDevice::TouchScreen;
    }
    if(touchdevice == "TouchPad"){
        return TouchDevice::TouchPad;
    }
    return TouchDevice::NOT_SUPPORT;
}

XMLReader* XMLReader::s_self = nullptr;

XMLReader::XMLReader()
{
    if(!copyConfigIfNotPresent())
        qDebug() << "config file create error!";

    phraseXml();
}

QString XMLReader::errorString() const{
    return QString("Error:%1  Line:%2  Column:%3")
            .arg(reader.errorString())
            .arg(reader.lineNumber())
            .arg(reader.columnNumber());
}

XMLReader* XMLReader::getInstance()
{
    if(!s_self){
        s_self = new XMLReader();
        return s_self;
    }
    else
        return s_self;
}

QString XMLReader::getConfigPath() const
{
    return getHomePath() + QString("/%1").arg(HOME_CONFIG_DIR);
}

QString XMLReader::getConfigFile()
{
    return getHomePath() + QString("/%1/%2").arg(HOME_CONFIG_DIR).arg(CONFIG_FILE);
}

bool XMLReader::copyConfigIfNotPresent()
{
    QFile configFile(getConfigFile());
    if(!configFile.exists()){
        QDir configDir;
        configDir.mkpath(getConfigPath());
        if(!QFile::exists(QString(USR_SHARE_CONFIG_FILE))){
            qDebug() << USR_SHARE_CONFIG_FILE << "is not exists";
            return false;
        }
        return QFile::copy(QString(USR_SHARE_CONFIG_FILE), getConfigFile());
    }
    return true;
}

bool XMLReader::read()
{
    if (reader.readNextStartElement()) {
        QString strName = reader.name().toString();    // 获取根元素
        qDebug() << "根元素名字为：" << strName;

        QXmlStreamAttributes attributes = reader.attributes();
        if (attributes.hasAttribute("version")) {  // 存在属性 version
            QString strVersion = attributes.value("version").toString();
            if (strVersion == "1.0") {  // 可以作为版本兼容性判断
                qDebug() << "Version : " << strVersion << endl
                         << "_____________________________";
            } else {
                reader.raiseError("The file is not an XBEL version 1.0 file.");
            }
        }
    }

    return !reader.error();
}

bool XMLReader::phraseXml()
{
    QString strFile(getConfigFile());
    file = new QFile(strFile);
    if (!file->open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug() << QString("Cannot read file %1(%2).").arg(strFile).arg(file->errorString());
        return false;
    }
    reader.setDevice(file);

    QString gestureType;
    QString fingers;
    QString direction;
    QString actionType;
    QString isEnable;
    QString touchdevice;
    QMap<QString, QString> actionSettings;
    QString setName;
    while(!reader.atEnd())
    {
        reader.readNext();
        if(reader.isStartElement() && reader.name() != "touchgesture")
        {
            // 将属性读出
            QXmlStreamAttributes attributes = reader.attributes();
            if(attributes.hasAttribute("type")){
                gestureType = attributes.value("type").toString();
            }
            if(attributes.hasAttribute("fingers")){
                fingers = attributes.value("fingers").toString();
            }
            if(attributes.hasAttribute("direction")){
                direction = attributes.value("direction").toString();
            }
            if(attributes.hasAttribute("touchdevice")){
                touchdevice = attributes.value("touchdevice").toString();
            }

            // 将action的设置读出
            if(reader.name() == "action" && reader.attributes().hasAttribute("actiontype")){
//                qDebug() << "action actiontype =" << reader.attributes().value("actiontype").toString();
                actionType = attributes.value("actiontype").toString();
                continue;
            }
            else if(reader.name() == "enable"){
                setName = "enable";
                isEnable = reader.readElementText();
                actionSettings[setName] = isEnable;
                continue;
            }
            else if(reader.name() == "onlytablet"){
                setName = "onlytablet";
                actionSettings[setName] = reader.readElementText();
                continue;
            }
            else if(reader.name() == "distance"){
                setName = "distance";
                actionSettings[setName] = reader.readElementText();
                continue;
            }
            else if(reader.name() == "modifiers"){
                setName = "modifiers";
                actionSettings[setName] = reader.readElementText();
                continue;
            }
            else if(reader.name() == "keys"){
                setName = "keys";
                actionSettings[setName] = reader.readElementText();
                continue;
            }
            else if(reader.name() == "command"){
                setName = "command";
                actionSettings[setName] = reader.readElementText();
                continue;
            }
            else if(reader.name() == "button"){
                setName = "button";
                actionSettings[setName] = reader.readElementText();
                continue;
            }
            else if(reader.name() == "dbus"){
                setName = "dbus";
                actionSettings[setName] = reader.readElementText();
                continue;
            }
            else if(reader.name() == "arg"){
                setName = "arg";
                actionSettings[setName] = reader.readElementText();
                continue;
            }
            else if(reader.name() == "repeat"){
                setName = "repeat";
                actionSettings[setName] = reader.readElementText();
                continue;
            }
            else if(reader.name() == "timeout"){
                setName = "timeout";
                actionSettings[setName] = reader.readElementText();
                continue;
            }
            else if(reader.name() == "eventstage"){
                setName = "eventstage";
                actionSettings[setName] = reader.readElementText();
                continue;
            }
            else{
//                qDebug() << "Unkown node ! ! !";
            }
        }
        else if(reader.isEndElement() && reader.name() == "gesture"){
            // 只有配置文件中被指定为enable=true的手势才会被注册到配置中去；
            if(isEnable == "true"){
                saveGestureConfig(gestureType, fingers, direction,touchdevice,
                                  strToActionType(actionType), actionSettings);
            }
            actionSettings.clear();
            isEnable = "false";
        }
    }

    file->close();
    qDebug() << "The number of registed fingers is :" << gestureConfig.size();
    return true;
}

QString XMLReader::arrangeGestureConfigToEnumStr(GestureType gestureType, QString &fingers, KWin::Gesture::GestureDirection gestureDirection)
{
    return QString::number(static_cast<int>(gestureType)) + "_" + fingers + "_" +
           QString::number(static_cast<int>(gestureDirection));
}

void XMLReader::saveGestureConfig(QString gestureType, QString &fingers,
                                       QString gestureDirection, QString touchDevice, ActionType actionType,
                                       QMap<QString, QString> &actionSettings)
{
    QList<QString> key;
    key << gestureType << fingers << gestureDirection << touchDevice;
//    = arrangeGestureConfigToEnumStr(gestureType, fingers, gestureDirection);
//    GestureSpecific key(gestureType, fingers.toInt(), gestureDirection);
    gestureConfig[key] = qMakePair(actionType, actionSettings);
}
