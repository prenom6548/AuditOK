#ifndef HASDEVICEPAGE_H
#define HASDEVICEPAGE_H

#include <QWidget>
#include "button.h"
#include "cameracontrolviewpage.h"
#include "VideoDisplay.h"
#include "VideoDisplayGL.h"
#include "devicepull.h"
#include <QStackedLayout>
#include <QDBusInterface>
#include "KylinCamera.h"
#include "util/daemonipcdbus.h"
#include "util/login1_dbus.h"
#include "util/unix_signals.h"
#include <QMutex>
#include "cameracodestate.h"
#include "dirchange.h"
#include "gridlinepaint.h"
#include <QtMultimedia/QMediaPlayer>
#include <QGraphicsOpacityEffect>
#include <kylin_system/session_management.hpp>

class HasDevicePage : public QWidget
{
    Q_OBJECT

public:
    //核心类
    Button *btnPage = nullptr;//按钮控件组页面,待修改爲私有
    GridLinePaint *gridLinePaint = nullptr;
    DirChange *dirChange = nullptr;
    explicit HasDevicePage(QWidget *parent = nullptr);
    ~HasDevicePage();
    void startExistCamera(KylinCameraInfo cameraInfo);
    KylinCameraInfo getCameraInfo();//获取摄像头信息
    void stopCamera();
    KylinCamera *camera = nullptr;
    KylinCamera *cameraTmp = nullptr;
    static bool has_device;//静态全局变量保存摄像头的状态，当前摄像头状态和现有的设备列表
    CameraControlViewPage *widgetDisplay = nullptr;
    quint32 flag;
    VideoDisplayGL *videoDisplay = nullptr;
private:
    //辅助类
    kdk::kabase::SessionManagement sessionManagementTest;
    QDBusMessage serverReturnValue;
    DefaultGetSet *gsettingItem = nullptr;//gsetting读取设置的接口类
    DevicePull *devicePull = nullptr;//处理外接摄像头插拔的类
    CameraCodeState *coadState = nullptr;
    CameraCodeState *coadStateTest = nullptr;
    QString fileNameZ;
    QMediaPlayer *player = nullptr;
    bool exceptionFlag = false;
    bool changeUserFlag = false;
    bool isConnected = false;
    bool m_isHidden = false;
    bool m_captureBurst_isHidden = false;
    void burstCaptureInterrupt();
    void showNotify(QString message);
    void showConnect(bool connect);
    bool notNeedMirror(const QString& sys , const QString current);
    QString getProductValue(const QString& deviceNode);
    bool isSpecialCamera = false;
protected:
    QGraphicsOpacityEffect * m_pGraphicsOpacityEffect1 = nullptr;
    //核心类
    QVBoxLayout *controlView = nullptr;//封裝btn界面和窗口小控件界面
    QWidget *controlViewWidget = nullptr;//封裝btn界面和窗口小控件界面
    QStackedLayout *layout = nullptr;
    QTimer *suspendOn = nullptr;
    QTimer *suspendOff = nullptr;
    QString archName;  //架构名字
    KylinCameraInfo m_camInfo;
    int heartBeatsNum;
    QTimer *secTimer = nullptr;
    QTimer *burstTimer = nullptr;
    QTimer *recordTimer = nullptr;

    QString imagePath; //保存路径
    QString photoFormat;
    QString configPath; //储存中间文件的目录
    bool is_video = false;
    QString fileNameTemp;  //照片名字
    QString videoFormat;
    QString video_path_n; //新生成录像
    QTimer *timer = nullptr;
    QTimer *m_videoTimer = nullptr;
    QTimer *countDownStopCameraTimer = nullptr;

    int nPix = 1;
    int pixCount = 0;
    int deadTimeSec = 0;
    int deadTimeSecTmp = 0;
    int m_videoDeadTimeSec = 0;
    int m_videoDeadTimeSecTmp = 0;

    bool if_delay = false;
    int hour = -1;
    int min = -1;
    int sec = -1;
    login1dbusManager *m_dbus = nullptr;
    QMutex m_mutex;
    QMutex m_mutex1;
    DaemonIpcDbus *m_dbusHanler = nullptr;
    void initConnect();
    void initAudioDevice();
    bool check_save_path();
    void noPathPrompt();
    QString creatName();
    void takePhoto(QString name ,bool isvideo);
    void camera_set_param(KylinCameraInfo *device_info);
    void initDbus();
    void takePhotoPre();
    bool cameraInfoSetTmp(const char *in_devname);
    void startRecord();
public Q_SLOTS:
    void creatCameraPage(const char *in_devname);
    void noDeviceHandle();
    void change_device(const char * in_device);
    void change_resolution(QPair<uint, uint> resolution);
    void clickPhoto();
    void clickStopRecord();
    void exception_handing();
    void clearAlbumView();
    void videoDisplayResize();
    void suspendRecord(bool flag);
    void defaultFilterSet();
    KylinCameraInfo cameraInfoSet(const char *in_devname, QString scale);
private Q_SLOTS:
    void countDownStopCameraEvent();
    void onOpacityWindow();
    void gridOn(bool);
    void clickStartRecord();
    void timeEvent();
    void videoTimerEvent();
    void imageDisplay(QString filename);
    void clickVideoMode();
    void clickCaptureMode();
    void nPixOperation(int i);
    void delay_operation(int i);
    void mirror(bool mirror);
    void updateRecordTime();
    void recordingInterrupt(bool in);
    void recordInterrupt(bool flag);
    void recordInterruptDone();
    void onShutDown();
    void onSigTerm();
    void onSigHup();
    void onSigAbrt();
    void cameraFilter(int);
    void pullOut();
    void burstControl();
    void recordingStop(bool res);
    void clickDelay(bool flag);
    void clickNPix(bool flag);
    void clickFilter(bool flag);
Q_SIGNALS:
    void clickRecordSig();
    void flash();
    void swithToHasDevicePage();
    void swithToNoDevicePage(int);//1：无设备 2：编码格式不支持
    void lockSigForDevicePull(bool);
    void changeDeviceSuccess(bool);
    void recoredTimeChanged(int hourSig,int minSig,int secSig);
    void settingOptionsOn(bool);
};

#endif // HASDEVICEPAGE_H
