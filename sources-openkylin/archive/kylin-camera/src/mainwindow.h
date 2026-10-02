#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include <QMainWindow>
#include <QKeyEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QFont>
#include <QLabel>
#include <QWidget>
#include <QMouseEvent>
#include <QListWidget>
#include <QStackedWidget>
#include <QByteArray>
#include <unistd.h>
#include <QMenu>
#include <QTimer>
#include <QSettings>
#include "widgetstyle.h"
#include "button.h"
#include "pictureviewpage.h"
#include "setting.h"
#include "VideoDisplay.h"
#include "kylincamera_global.h"
#include <QDateTime>
#include <QStandardPaths>
#include <QGSettings/QGSettings>
#include <QPixmap>
#include "settingpage.h"
#include <QProcess>
#include <QDBusUnixFileDescriptor>
#include <QMutex>
#include "util/login1_dbus.h"
#include "cameracodestate.h"
#include "cameratoppage.h"
#include "util/rotatechangeinfo.h"
#include "util/HorizontalOrVerticalMode.h"
#include "customwidget.h"
#include "windowmanager/windowmanager.h"

class MainWindow : public CustomWidget
{
    Q_OBJECT

public:
    MainWindow(QStringList strList,QWidget *parent = nullptr);
    ~MainWindow();
    void initSetting();
    void initConnect();
    void stayTop();  //应用置顶
    void funcListItemClicked(QListWidgetItem* item);//点击设置按钮
    void setbutton();
    void setcamerapage();
    void setpicturepage();
    void loadPhoto();
    QPoint temp;
    kdk::WindowId windowId();
    bool settingShow = false;
    static bool mirrorFlag;   //lhl debug
Q_SIGNALS:
    void click_video();
    void click_picture();
    void winHeight();
    void clickPhotoSignal();

public Q_SLOTS:
    int kylinCameraDbusRequest(QStringList command);
    void modeSwitch();

private Q_SLOTS:
    void slotIntelHVModeChanged(bool,int,int);
    void startCommend(QString cmd);
    void save_dir_change();
    void noPathHandle();   //lhl debug
    void settingPathPrompt(); //lhl debug   
protected:
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void resizeEvent(QResizeEvent *event);
//    virtual bool eventFilter(QObject *watched, QEvent *event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);
private:
    bool check_save_path();
    void inhibit(QString what,QString why);
    void unInhibit();
    //void initCommandDbus();
    bool singleInstance(QStringList strList);
    void initDbus();
    bool started();
    bool videoCodeStop();  //lhl debug
    void handleIconClickedSub();
    void userManualTest(void);
    void exitApp();

private:
    RotateChangeInfo *rotateChangeInfoObj = nullptr;
    deviceMode m_hLayoutFlag = PCMode;

    DefaultGetSet *gsettingItem = nullptr;

    Qt::WindowFlags winFlags;
    QStringList::reverse_iterator iter;
    QStringList picture_list;
    QString fileNameTemp;  //照片名字
    QWidget *ViewWid = nullptr;
    QWidget *PictureWid = nullptr;
    QWidget *btnWid = nullptr;
//    QWidget *titleBarWid = nullptr;
    //  QWidget *setWid;
    Setting *setWid = nullptr;
    QWidget *rightbtn = nullptr;
    QWidget *mainWid = nullptr;
    QHBoxLayout *btnLayout = nullptr;
    QHBoxLayout *mainLayout = nullptr;
    QGridLayout *pictureLayout = nullptr;
    DeviceRead *devRead = nullptr;
    //各个组件
    //CameraPage *camerapage = nullptr;
    CameraTopPage *cameraTopPage = nullptr;
    PictureViewPage *viewpage = nullptr;
    QString imagePath; //保存路径
    QString videoPath;
    QString configPath; //储存中间文件的目录
    //监听设备文件
    QFileSystemWatcher *watcher = nullptr;

    QTimer *timer = nullptr;
    //倒计时
    QWidget *dead_time = nullptr;
    QProcess *proc = nullptr;
    QPoint m_start;
    QPoint m_end;
    int index = -1;
    int timerId;
    int dead_time_sec;
    int dead_time_sec_tmp = 0;
    bool is_video = false;
    bool Burst_mode = false;
    bool press = false;
    bool if_delay = false;
    bool m_needContinue = false;

    login1dbusManager *m_dbus = nullptr;
    QDBusUnixFileDescriptor m_inhibitFileDescriptor;
    DaemonIpcDbus *m_dbusHanler = nullptr;
    QString m_currentUser;
    QMutex m_mutex;

    CameraCodeState *coadState = nullptr;
    kdk::WindowId  m_windowId = "";

};
#endif // MAINWINDOW_H
