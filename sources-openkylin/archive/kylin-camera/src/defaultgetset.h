#ifndef DEFAULTGETSET_H
#define DEFAULTGETSET_H

#include <QGSettings>
#include <QString>
#include <QWidget>
extern "C" {
#include <dconf/dconf.h>
#include <canberra.h>
}

#define NAME_KEY "name"
#define FILENAME_KEY "filename"
#define EVENT_SOUNDS_KEY "event-sounds"
#define KEY_SOUNDS_SCHEMA "org.ukui.sound"
#define KEYBINDINGS_CUSTOM_DIR "/org/ukui/sound/keybindings/"
#define KEYBINDINGS_CUSTOM_SCHEMA "org.ukui.media.sound"

class DefaultGetSet: public QWidget
{
    Q_OBJECT
public:
    DefaultGetSet(QWidget *parent = 0);
    ~DefaultGetSet();
    //獲取gsetting值
    QString getFileName();
    QString getVideoCaptureFileName();
    bool getIsVideo();
    QString getSavePath();
    QString getVideoSavePath();
    QString getUser();
    int getDelayTime();
    int getScaleIndex();
    QString getScale();
    int getDeviceIndex();
    int getPhotoFormat();
    int getVideoFormat();
    QString getNoMirrorDevice();
    //获取主题gsetting值
    QString getStyleName();
    //設置gsetting值
    void setFileName(QString);
    void setVideoCaptureFileName(QString);
    void setIsVideo(bool);
    void setSavePath(QString);
    void setVideoSavePath(QString);
    void setUser(QString);
    void setDelayTime(int);
    void setScaleIndex(int);
    void setScale(int, int);
    void setDeviceIndex(int);
    void setPhotoFormat(int);
    void setVideoFormat(int);
    void playAlertSound(QString gsettingStr);
    void addPreviewMonitorSDKPoint();
    void addSingleShotSDKPoint();
    void addContinuousShotSDKPoint();
    void addDelaySDKPoint();
    void addVideotapeSDKPoint();
    void addCameraSelectionSDKPoint();
    void addResolutionSelectionSDKPoint();
    void addVideoFormatSDKPoint();
    void addPictureFormatSDKPoint();
    void addChangeSavePathSDKPoint();
    void addThumbnailSDKPoint();
    void addGridLineSDKPoint();
    void addMirrorFunSDKPoint();
    QList<char *> listExistsPath();
    QGSettings *setting = nullptr;
    QGSettings *m_pGSettingThemeData = nullptr;
    GSettings *m_pGsettingSound = nullptr;
    ca_context *caContext;
private:
    QString fileName;
    bool isVideo;
    QString savePath;
    QString videoSavePath;
    QString user;
    int delayTime;
    int scaleIndex;
    int deviceIndex;
    int photoFormat;
    int videoFormat;
    QString noMirrorDevice;
    QString styleName;
    QString videoCaptureFileName;
Q_SIGNALS:
    void sigThemeChange(QString);
};

#endif // DEFAULTGETSET_H
