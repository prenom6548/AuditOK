#include "defaultgetset.h"
#include <QVariant>
#include <QDebug>
#include <buried_point.hpp>
#include <gsettings.hpp>

#include <ksoundeffects.h>

DefaultGetSet::DefaultGetSet(QWidget *parent)
    : QWidget(parent)
{
    setting = new QGSettings("org.kylin-camera.settings");
    m_pGSettingThemeData = new QGSettings("org.ukui.style");
    ca_context_create(&caContext);
    m_pGsettingSound = g_settings_new (KEY_SOUNDS_SCHEMA);
    if(setting->isSchemaInstalled("org.kylin-camera.settings")
            && m_pGSettingThemeData->isSchemaInstalled("org.ukui.style")){
        qDebug() << "success";
    }
    else{
        qDebug() << "fail";
        exit(0);
    }
    photoFormat = 0;
    videoFormat = 0;
    connect(kdk::kabase::Gsettings::getPoint(), &kdk::kabase::Gsettings::systemThemeChange, this, [=]() {
        QString theme = getStyleName();
        Q_EMIT sigThemeChange(theme);
    });
}
DefaultGetSet::~DefaultGetSet(){
    delete setting;
    delete m_pGSettingThemeData;
}
//播放指定类型的提示音
void DefaultGetSet::playAlertSound(QString gsettingStr){
    if (gsettingStr == "photo")
        kdk::KSoundEffects::playSound(kdk::SCREEN_CAPTURE);
    else if (gsettingStr == "screen")
        kdk::KSoundEffects::playSound(kdk::SCREEN_RECORD);
    return;

    gint retval;
    const gchar *desc = "Alert Sound";
    QString filenameStr;
    QList<char *> existsPath = this->listExistsPath();
    for (char * path : existsPath) {
        char * prepath = QString(KEYBINDINGS_CUSTOM_DIR).toLatin1().data();
        char * allpath = strcat(prepath, path);
        const QByteArray ba(KEYBINDINGS_CUSTOM_SCHEMA);
        const QByteArray bba(allpath);
        if(QGSettings::isSchemaInstalled(ba)){
            QGSettings * settings = new QGSettings(ba, bba);
            filenameStr = settings->get(FILENAME_KEY).toString();
            QString nameStr = settings->get(NAME_KEY).toString();
            //此处匹配对应的gsetting接口字段
            if (nameStr == gsettingStr){
                break;
            }
        }
    }
    const QByteArray text = filenameStr.toLocal8Bit();
    const gchar *id = text.data();
    const gchar *eventId =id;
    if(desc){
        bool status = g_settings_get_boolean(m_pGsettingSound, EVENT_SOUNDS_KEY);
        if (status)
            retval = ca_context_play (this->caContext, 0,
                                      CA_PROP_EVENT_ID, eventId,
                                      CA_PROP_EVENT_DESCRIPTION, desc, NULL);

        qDebug() << "ca_context_play" << status << eventId << retval;
    }
}
QList<char *> DefaultGetSet::listExistsPath()
{
    char ** childs;
    int len;

    DConfClient * client = dconf_client_new();
    childs = dconf_client_list (client, KEYBINDINGS_CUSTOM_DIR, &len);
    g_object_unref (client);

    QList<char *> vals;

    for (int i = 0; childs[i] != NULL; i++){
        if (dconf_is_rel_dir (childs[i], NULL)){
            char * val = g_strdup (childs[i]);

            vals.append(val);
        }
    }

    g_strfreev (childs);
    return vals;
}
QString DefaultGetSet::getFileName(){
    fileName = setting->get("filename").toString();
    return fileName;
}
QString DefaultGetSet::getVideoCaptureFileName(){
    videoCaptureFileName = setting->get("video-capture").toString();
    return videoCaptureFileName;
}
bool DefaultGetSet::getIsVideo(){
    isVideo = setting->get("is-video").toBool();
    return isVideo;
}
QString DefaultGetSet::getSavePath(){
    savePath = setting->get("save-path").toString();
    return savePath;
}
QString DefaultGetSet::getVideoSavePath(){
    videoSavePath = setting->get("video-save-path").toString();
    return videoSavePath;
}
QString DefaultGetSet::getUser(){
    user = setting->get("user").toString();
    return user;
}
int DefaultGetSet::getDelayTime(){
    delayTime = setting->get("delay-time").toInt();
    return delayTime;
}
int DefaultGetSet::getScaleIndex(){
    scaleIndex = setting->get("scale-index").toInt();
    return scaleIndex;
}
QString DefaultGetSet::getScale()
{
    return setting->get("scale").toString();
}
int DefaultGetSet::getDeviceIndex(){
    deviceIndex = setting->get("device-index").toInt();
    return deviceIndex;
}
int DefaultGetSet::getPhotoFormat(){
    photoFormat = setting->get("photo-format").toInt();
    return photoFormat;
}
int DefaultGetSet::getVideoFormat(){
    videoFormat = setting->get("video-format").toInt();
    return videoFormat;
}

QString DefaultGetSet::getNoMirrorDevice()
{
    noMirrorDevice = setting->get("nomirror").toString();
    return noMirrorDevice;
}
QString DefaultGetSet::getStyleName(){
    QString styleName = kdk::kabase::Gsettings::getSystemTheme().toString();
    return styleName;
}
void DefaultGetSet::setFileName(QString res){
    fileName = res;
    setting->set("filename",fileName);
}
void DefaultGetSet::setVideoCaptureFileName(QString res){
    videoCaptureFileName = res;
    setting->set("video-capture",videoCaptureFileName);
}
void DefaultGetSet::setIsVideo(bool res){
    isVideo = res;
    setting->set("is-video",isVideo);
}
void DefaultGetSet::setSavePath(QString res){
    savePath = res;
    setting->set("save-path",savePath);
}
void DefaultGetSet::setVideoSavePath(QString res){
    videoSavePath = res;
    setting->set("video-save-path",videoSavePath);
}
void DefaultGetSet::setUser(QString res){
    user = res;
    setting->set("user",user);
}
void DefaultGetSet::setDelayTime(int res){
    delayTime = res;
    setting->set("delay-time",delayTime);
}
void DefaultGetSet::setScaleIndex(int res){
    scaleIndex = res;
    setting->set("scale-index",scaleIndex);
}
void DefaultGetSet::setScale(int width, int height)
{
    QString scale = QString::number(width) + "*" + QString::number(height);
    setting->set("scale", scale);
}
void DefaultGetSet::setDeviceIndex(int res){
    deviceIndex = res;
    setting->set("device-index",deviceIndex);
}


void DefaultGetSet::setPhotoFormat(int res){
    photoFormat = res;
    setting->set("photo-format",photoFormat);
}
void DefaultGetSet::setVideoFormat(int res){
    videoFormat = res;
    setting->set("video-format",videoFormat);
}
void DefaultGetSet::addPreviewMonitorSDKPoint(){
    kdk::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(kdk::kabase::AppName::KylinCamera , kdk::kabase::BuriedPoint::PT::KylinCameraPreviewMonitor)) {
        qCritical() << "Error : buried point fail !";
    };

    return;
}
void DefaultGetSet::addSingleShotSDKPoint(){
    kdk::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(kdk::kabase::AppName::KylinCamera , kdk::kabase::BuriedPoint::PT::KylinCameraSingleShot)) {
        qCritical() << "Error : buried point fail !";
    };

    return;
}
void DefaultGetSet::addContinuousShotSDKPoint(){
    kdk::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(kdk::kabase::AppName::KylinCamera , kdk::kabase::BuriedPoint::PT::KylinCameraContinuousShot)) {
        qCritical() << "Error : buried point fail !";
    };

    return;
}
void DefaultGetSet::addDelaySDKPoint(){
    kdk::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(kdk::kabase::AppName::KylinCamera , kdk::kabase::BuriedPoint::PT::KylinCameraDelay)) {
        qCritical() << "Error : buried point fail !";
    };

    return;
}
void DefaultGetSet::addVideotapeSDKPoint(){
    kdk::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(kdk::kabase::AppName::KylinCamera , kdk::kabase::BuriedPoint::PT::KylinCameraVideotape)) {
        qCritical() << "Error : buried point fail !";
    };

    return;
}
void DefaultGetSet::addCameraSelectionSDKPoint(){
    kdk::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(kdk::kabase::AppName::KylinCamera , kdk::kabase::BuriedPoint::PT::KylinCameraCameraSelection)) {
        qCritical() << "Error : buried point fail !";
    };

    return;
}
void DefaultGetSet::addResolutionSelectionSDKPoint(){
    kdk::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(kdk::kabase::AppName::KylinCamera , kdk::kabase::BuriedPoint::PT::KylinCameraResolutionSelection)) {
        qCritical() << "Error : buried point fail !";
    };

    return;
}
void DefaultGetSet::addVideoFormatSDKPoint(){
    kdk::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(kdk::kabase::AppName::KylinCamera , kdk::kabase::BuriedPoint::PT::KylinCameraVideoFormatSelection)) {
        qCritical() << "Error : buried point fail !";
    };

    return;
}
void DefaultGetSet::addPictureFormatSDKPoint(){
    kdk::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(kdk::kabase::AppName::KylinCamera , kdk::kabase::BuriedPoint::PT::KylinCameraPictureFormatSelection)) {
        qCritical() << "Error : buried point fail !";
    };

    return;
}
void DefaultGetSet::addChangeSavePathSDKPoint(){
    kdk::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(kdk::kabase::AppName::KylinCamera , kdk::kabase::BuriedPoint::PT::KylinCameraChangeSavePath)) {
        qCritical() << "Error : buried point fail !";
    };

    return;
}
void DefaultGetSet::addThumbnailSDKPoint(){
    kdk::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(kdk::kabase::AppName::KylinCamera , kdk::kabase::BuriedPoint::PT::KylinCameraThumbnail)) {
        qCritical() << "Error : buried point fail !";
    };

    return;
}
void DefaultGetSet::addGridLineSDKPoint(){
    kdk::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(kdk::kabase::AppName::KylinCamera , kdk::kabase::BuriedPoint::PT::KylinCameraGridLine)) {
        qCritical() << "Error : buried point fail !";
    };

    return;
}
void DefaultGetSet::addMirrorFunSDKPoint(){
    kdk::kabase::BuriedPoint buriedPointTest;

    if (buriedPointTest.functionBuriedPoint(kdk::kabase::AppName::KylinCamera , kdk::kabase::BuriedPoint::PT::KylinCameraMirrorFun)) {
        qCritical() << "Error : buried point fail !";
    };

    return;
}
