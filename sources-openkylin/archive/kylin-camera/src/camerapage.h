#ifndef CAMERAPAGE_H
#define CAMERAPAGE_H
#include <QWidget>
#include <QDebug>
#include <QPushButton>
#include <QLabel>
#include <QBoxLayout>
#include <QLayout>
#include <QCamera>
#include <QCameraInfo>
#include <QAudioDeviceInfo>
#include <QPixmap>
#include <QStackedLayout>
#include <QMediaServiceCameraInfoInterface>
#include <QMediaService>
#include <QImage>
#include <QMessageBox>
#include <QTimer>
#include <QTime>
#include <QCameraCaptureBufferFormatControl>
#include <QCameraImageCapture>
#include <QStatusBar>
#include <QMediaRecorder>
#include <QFileSystemWatcher>
#include <QStackedWidget>
#include <QApplication>
#include <QFileInfoList>
#include <QDBusInterface>
#include <QThread>
#include "currentdeviceinfo.h"
#include "button.h"
#include "VideoDisplay.h"
#include "kylincamera_global.h"
#include "KylinCamera.h"
#include "delaywdiget.h"
#include "aboutwidget.h"
#include "camerakeepalive.h"

class CameraPage : public QStackedWidget
{
    Q_OBJECT
public:
    CameraPage(QWidget *parent = 0);
    ~CameraPage();
    QImage image;
    //图像右上角按钮

    QHBoxLayout *hlayout = nullptr;
    QVBoxLayout *vlayout = nullptr;
    QVBoxLayout *vlayout_out = nullptr;
    QStackedLayout *layout = nullptr;
    QCameraInfo *camerainfo = nullptr;
    QHBoxLayout *recordLayout = nullptr;
    QHBoxLayout *buttomLayout = nullptr;
    QHBoxLayout *recorderWidgetLayout = nullptr;
    //摄像头信息
    KylinCamera *camera = nullptr;
    VideoCapture *video = nullptr;
    VideoDisplay *videoDisplay = nullptr;
    QFileInfoList video_info;

    QList<QCameraInfo> camerainfo_list;
    QCameraInfo camera_info;
    Button *btnPage = nullptr;

    //倒计时
    QLabel *dead_time = nullptr;
    QTimer *time = nullptr;

    QWidget *widgetDisplay;
    QWidget *timeLabel;
    QLabel *recorder_time_label;
    QTimer * recorder_time;
    DelayWdiget *delayWidget;
    QWidget *recorder;
    QWidget *recorderWidget;
    QWidget *recorderWidgetWin;
    QStackedWidget *buttom;
    QWidget *buttomWidget;
    int hour = 0;
    int min = 0;
    int sec = 0;
    //未发现摄像头
    QLabel *No_device_lab = nullptr;
    QLabel *No_device_lab_s = nullptr;
    QLabel *No_device_icon = nullptr;
    //摄像头显示界面
    QWidget *cameraViewPage = nullptr;

    //静态全局变量保存摄像头的状态，当前摄像头状态和现有的设备列表
    static bool has_device;
    static char* current_indevice;
    static QList<QCameraInfo> *cameras;
    static QList<QAudioDeviceInfo> *audios;
    static QGSettings *m_GsettingThemeData;
    QDBusInterface *server = nullptr;
    quint32 arg_0; //用来保存返回值
    void resetCamera(KylinCameraInfo);

protected:
//    virtual bool eventFilter(QObject *watched, QEvent *event);
    virtual void paintEvent(QPaintEvent *);
signals:
    //设备被拔出
    void device_pull_out();

public slots:
    //未发现设备
    void displayCameraError();
    //时间信号
    void timeEvent();
    //打开摄像头
    void creatCameraPage(const char *);
    //设置设备参数
    void camera_set_param(KylinCameraInfo *);
    //更改摄像头设备
    void change_device(const char *);
    //设置分辨率
    void change_resolution(QPair<uint,uint>);
//    //改变分辨率后，预览页面更改窗口比例
//    void changeWindowScale();
    //返回当前使用的设置
    void stopCamera();
    void startExistCamera(KylinCameraInfo cameraInfo);
    KylinCameraInfo getCameraInfo();
    void maxWindowView();
protected:

private:
    //创建显示界面
    //未发现摄像头的两个组件
    QWidget *widget1 = nullptr;
    QWidget *widget2 = nullptr;
    QWidget *widget3 = nullptr;
    //未发现摄像头界面
    QWidget *No_device;
    //摄像头显示界面
    QWidget *cameraViewPage;
    QLabel *lab;

    void layout_operation();
    void drawGrid(QPainter *);
    void setNoDevice();
    void setCameraViewPage();
    void timeoutEvent(KylinCameraInfo);
    //
    CameraKeepAlive *perSec = nullptr;
    QTimer *secTimer = nullptr;
    int heartBeatsNum;
    KylinCameraInfo m_camInfo;
    void dealSystemGsettingChange(const QString);
    void refreshThemeBySystemConf();
    void pictureViewPage();
    void videoViewPage();
    void initAudioDevice();
    void getCameraInformation(const char *);
};

#endif // CAMERAPAGE_H
