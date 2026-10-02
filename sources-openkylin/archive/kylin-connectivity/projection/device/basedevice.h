#ifndef BASEDEVICE_H
#define BASEDEVICE_H

#include <QElapsedTimer>
#include <QPointer>
#include <QTime>
#include <QtDBus/QDBusInterface>
#include "baseserver.h"

#include "controlmsg.h"


class QMouseEvent;

class QWheelEvent;

class QKeyEvent;

// class Recorder;

class BaseServer;

class VideoBuffer;

class Decoder;

// class FileHandler;

class Stream;

class VideoForm;

class Controller;

struct AVFrame;

class BaseDevice : public QObject
{
    Q_OBJECT
public:
    struct DeviceParams
    {
        QString recordFileName = ""; // 视频录制文件名
        QString recordPath = "";     // 视频保存路径
        QString serial = "";         // 设备序列号
        quint16 localPort = 27183;   // reverse时本地监听端口
        quint16 maxSize = 0;         // 视频分辨率
        quint32 bitRate = 8000000;   // 视频比特率
        quint32 maxFps = 60;         // 视频最大帧率
        bool closeScreen = false;    // 启动时自动息屏
        bool useReverse = true; // true:先使用adb reverse，失败后自动使用adb forward；false:直接使用adb forward
        bool display = true;              // 是否显示画面（或者仅仅后台录制）
        QString gameScript = "";          // 游戏映射脚本
        bool renderExpiredFrames = false; // 是否渲染延迟视频帧
        int lockVideoOrientation = -1;    // 是否锁定视频方向
        bool stayAwake = false;           // 是否保持唤醒
        bool framelessWindow = false;     // 是否无边框窗口
        int screenIndex = 0;              // 扩展屏索引
    };
    enum GroupControlState {
        GCS_FREE = 0,
        GCS_HOST,
        GCS_CLIENT,
    };

    BaseDevice(DeviceParams params, QObject *parent = nullptr);

    virtual ~BaseDevice();

    virtual VideoForm *getVideoForm();

    virtual const QString &getSerial();

    virtual const QSize frameSize();

    BaseDevice::GroupControlState controlState();

    virtual void updateScript(QString script);

    virtual bool isCurrentCustomKeymap();

    bool getDeviceStatus();

Q_SIGNALS:
    void deviceDisconnect(QString serial);

    // tool bar
    void switchFullScreen();
    void postGoBack();
    void postGoHome();
    void postGoMenu();
    void postAppSwitch();
    void postPower();
    void postVolumeUp();
    void postVolumeDown();
    void postCopy();

    void postCut();

    void setScreenPowerMode(ControlMsg::ScreenPowerMode mode);

    void expandNotificationPanel();
    void collapsePanel();
    void postBackOrScreenOn(bool down);
    void postTextInput(QString &text);
    void requestDeviceClipboard();

    void setDeviceClipboard(bool pause = true);

    void clipboardPaste();

    void pushFileRequest(const QString &file, const QString &devicePath = "");

    void installApkRequest(const QString &apkFile);

    // key map
    void mouseEvent(const QMouseEvent *from, const QSize &frameSize, const QSize &showSize);

    void wheelEvent(const QWheelEvent *from, const QSize &frameSize, const QSize &showSize);

    void keyEvent(const QKeyEvent *from, const QSize &frameSize, const QSize &showSize);

    // self connect signal and slots
    void screenshot();

    void showTouch(bool show);

    void setControlState(BaseDevice *device, BaseDevice::GroupControlState state);

    void grabCursor(bool grab);

    // for notify
    void controlStateChange(BaseDevice *device, BaseDevice::GroupControlState oldState,
                            BaseDevice::GroupControlState newState);

    // receive from ui
    void sigSocketDisconnect();

    // send to ui
    void sigVideoClose();
    void sigScreenConnected();

    void sigInputChinese(const QString &input);

protected:
    virtual void initSignals();

    virtual void startServer();

    bool saveFrame(const AVFrame *frame);

    void startAudio();

    void startVideo();

    void showSupportControl();

public Q_SLOTS:
    void onScreenshot();

    void onShowTouch(bool show);

    void onSetControlState(BaseDevice *device, BaseDevice::GroupControlState state);

    void onGrabCursor(bool grab);

    void onConnectToResult(bool success, const QString &deviceName, const QSize &size, const QString &clientIP);


protected:
    // server relevant
    QPointer<BaseServer> m_server;
    QPointer<Decoder> m_decoder;
    QPointer<Controller> m_controller;
    //    QPointer<FileHandler> m_fileHandler;
    QPointer<Stream> m_stream;
    VideoBuffer *m_vb = Q_NULLPTR;
    //    Recorder *m_recorder = Q_NULLPTR;

    // ui
    QPointer<VideoForm> m_videoForm;

    QElapsedTimer m_startTimeCount;

    DeviceParams m_params;

    GroupControlState m_controlState = GCS_FREE;

    QSize m_size;
    QString m_clientIP = "";

    bool isUsbDevice = false;
    // QDBusInterface *m_interface = nullptr;
    // quint32 m_inhibitValue = 0;
};

#endif // BASEDEVICE_H
