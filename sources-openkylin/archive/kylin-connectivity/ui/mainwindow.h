#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QStackedWidget>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusInterface>
#include <QObject>
#include <QMap>
#include <QThread>
#include <QProcess>
#include <QApplication>
#include <QScreen>
#include <QMetaType>
#include <QMutex>
#include <QGSettings>
#include <kballontip.h>

#include "connectionservice.h"
#include "wifimanager.h"
#include "ftpmanager.h"
#include "filesyncmanage.h"
#include "usbmanage.h"
#include "devicemanage.h"
#include "pcscreenmanage.h"
#include "searchserver.h"
#include "connectinterface.h"
#include "searchdevicewin.h"
#include "devicecodewidget.h"
#include "titlebar.h"
#include "homepage.h"
#include "transmissiondialog.h"
#include "loadanimation.h"
#include "loaddialog.h"
#include "connectedwin.h"
#include "suspendtabbar.h"
#include "discovery.h"
#include "fineftp/server.h"
#include "kaboutdialog.h"
#include "dirmanager.h"
#include "publicattributes.hpp"
#include "filemanagewin.h"
#include "multimediawin.h"
#include "androidhomepage.h"
#include "fileview.h"
#include "messagedialog.h"
#include "tipdisplaywin.h"
#include "tablemodelistener.h"

#define KYLIN_CONNECTIVITY_SEND_SERVICE "com.kylin.connectivity.send"
#define KYLIN_CONNECTIVITY_SEND_PATH "/com/kylin/connectivity"
#define KYLIN_CONNECTIVITY_SEND_INTERFACE "com.kylin.connectivity.send"

#define UKUI_STYLE_GSETTING_PATH "org.ukui.style"

const QString MOUNT_DIR = "/var/lib/mult";

using namespace KProjection;
using namespace fineftp;

class MainWindow : public QWidget
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", KYLIN_CONNECTIVITY_SEND_INTERFACE)
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    quint32 getWinId() const;

    // 提供给dbus的接口
    // 获取当前是否有设备连接
    QString getDeviceName();
    // 获取当前目录文件列表
    void list(QString path);
    // 下载
    void downFile(const QList<FileInfo> &fileList, QString downloadPath);

protected:
    void closeEvent(QCloseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    enum WinIndex {
        WinHomePage = 0,  // 首页
        WinConnect,       // 连接页面
        WinConnected,     // 已连接页面
    };

    enum TYPE {
        None = 0,
        Mobile,  // 手机连接
        Search,  // 搜索
        Pctopc,  // Pc连接
    };

    void initService();
    void initGsetting();
    void initDbus();
    void disconnectScreen();
    void disconnectFtp();
    void stopAllDown();
    void disconnectService();
    void deleterService();
    bool mountFtp(QString url, QString userName, QString pwd, QString path);
    bool umountFtp(QString path);
    void clearConnectionInfo();
    void startTimer();
    void initSearchServer(QString searchPath);
    void startLoadUI();
    void initUI();
    void connectUI();
    void updateResult();
    void initMultiMediaWin(QMap<int, int> &map);
    void initTransmissionDialog();
    void clearView();
    void setToolTipWin(QString msg, kdk::TipType type);
    void initMobileFileSync();
    bool setScreenOption();
    int getScreenIndex();
    void deleterUI();
    void deleterDialog();
    void deleteTransmissionDialog();
    void deleteStackedWidget();
    void initPcSearchInfo();
    void discInit();
    void deInitDisc();
    void addBackList(QStringList &list);
    void checkAndroidHomePage();
    void checkFileView(QStringList &pathList);
    void crumbTagToAndroidItemType(QString tag);
    void checkPage(QStringList crumbPathList);
    void moveMessageBox();
    void switchUploadPath();
    void setTipDisplayWin(QString text);

Q_SIGNALS:
    // 提供dbus服务
    void sigConnected(QString);
    void sigDisconnect();
    void sigFileInfoList(const QMap<QString, FileInfo> &);
public Q_SLOTS:
    // 外部
    void setLocalPathList(const QStringList &list);

    // 服务
    void slotVolumeConnected(QString volumeName);
    void slotVolumeDisconnected(QString volumeName);
    void slotWifiConnected(QString wifiIP);
    void slotWifiDisConnected();
    void slotConnectRequest(QString deviceName);
    void slotConnectDenied();
    void slotConnect(const ConnectionService::ConnectionInfo &connectInfo);
    void slotDisconnect();
    void slotNotFountApk();
    void slotNoUsbDevice();
    void slotConnectFailed();
    void slotFtpConnected();
    void slotCurrentDirectoryList(const QMap<QString, FileInfo> &map);
    void slotTransferProgress(qint64 readBytes, qint64 totalBytes);
    void slotDownFileFinish(QString flag, QString filepath);
    void slotDownAllFileFinish(QString flag);
    void slotDownTempFileFinish(QString filePath);
    void slotDownFileFail(QString fileName, QString flag);
    void slotDownFileFail(QString fileName);
    void slotMountError(bool isMount, QString error);
    void slotAllDeviceInfo(const QMap<QString, KCommon::DiscoveryDeviceInfo> &map);
    void slotRequestReceived(PcScreenManage::ScreenMsg msg);
    void slotModeChanged(bool isTable);
    void slotSearchLoadFail();
    void slotRejectTran(FtpManager::ErrorType type);

    // UI
    void changeTheme();
    void changeFontSize();
    void changeFont(const QString &key);
    void slotAboutWinShow();
    void slotSearchResult(const QMap<QString, FileInfo> &searchResult);
    void slotUSBConnectOnClicked();
    void slotCrumbIndexChange(QString text, QStringList &list);
    void slotFileManageBtnClicked(FileManageWin::BtnType type);
    void slotSearchTextChanged(QString text);
    void slotDownFile(QString path, QList<FileInfo> &fileList);
    void slotUploadFile(QString dirName, const QStringList &localPathList);
    void slotAndroidBtnClicked(AndroidItem::Type type);
    void slotCdDirectory(QString path);
    void slotOpenFile(FileInfo::FileType type, QString path);
    void slotFileItemSelecAll(bool isSelectAll);
    void slotContinueSearch();
    void slotAbortTransmission();
    void slotConnectBtnClicked();
    void slotInitDiscServerResult(bool);
    void slotReturnHomePage();
    void slotConnectServiceUI(QString address);
    void slotConnectedWinBtnClicked(ConnectedWin::BtnType type);
    void slotExitMobileScreen();
    void slotExitPcScreen();
    void slotTabBtnClicked(SuspendTabBar::BtnType type);
    void slotSearchInfoFinish(QString searchInfoPath);

private:
    quint32 m_winId = 0;

    ConnectionService *m_connectionService = nullptr;   // 连接服务
    KDiscovery::Discovery *m_discovery = nullptr;       // 搜索手机设备信息
    FtpManager *m_ftpClient = nullptr;                  // ftp服务器
    DeviceManage *m_deviceManage = nullptr;             // 连接手机投屏
    UsbManage *m_usbManage = nullptr;                   // 监听USB插拔信号
    WifiManager *m_wifiManager = nullptr;               // 监听wifi信号
    QThread *m_wifiManagerThread = nullptr;             // 另起线程，监听wifi信号
    FileSyncManage *m_fileSyncManage = nullptr;         // 文件同步
    SearchServer *m_searchServer = nullptr;             // 文件搜索
    QThread *m_searchThread = nullptr;                  // 文件搜索线程
    FtpServer *m_ftpServer = nullptr;                   // ftp服务器
    PcScreenManage *m_pcScreen = nullptr;               // PC投屏
    QDBusInterface *m_dbusInterface = nullptr;          // 挂载ftpdubs
    DirManager *m_dirManager = nullptr;                 // 本地用户目录信息管理
    TableModeListener *m_tableModeListener = nullptr;   // 平板模式切换
    QDBusInterface *m_activityDbusInterface = nullptr;  // 防止熄屏
    QTimer *m_timer = nullptr;                          // 连接加载定时器，连接超时处理
    // disc
    QProcess *m_discServer = nullptr;
    QProcess *m_discClient = nullptr;
    QGSettings *m_fontData = nullptr;  // 字体变化

    Titlebar *m_titlebar = nullptr;                      // 标题栏
    HomePage *m_homepage = nullptr;                      // 首页
    ConnectInterface *m_connectInterfaceWin = nullptr;   // 连接界面
    ConnectedWin *m_connectedWin = nullptr;              // 已连接界面
    QStackedWidget *m_mainStackedWin = nullptr;          // 主界面切换堆栈窗口
    FileManageWin *m_fileManageWin = nullptr;            // 文件管理title
    AndroidHomePage *m_androidHomePage = nullptr;        // 安卓文件管理首页
    MultiMediaWin *m_multiMediaWin = nullptr;            // QQ | 微信文件首页
    FileView *m_fileView = nullptr;                      // 文件信息列表
    kdk::KAboutDialog *m_aboutWindow = nullptr;          // 关于窗口
    TransmissionDialog *m_transmissionDialog = nullptr;  // 传输进度窗口
    MessageDialog *m_messageBox = nullptr;               // 请求提示框
    SuspendTabBar *m_suspendTabBar = nullptr;            // 投屏控制
    kdk::KBallonTip *m_tipWin = nullptr;                 // 消息提示
    LoadDialog *m_loadDialog = nullptr;                  // 加载等待
    QLabel *m_tipDisplayWin = nullptr;                   // 显示当前目录状态以及搜索结果状态界面
    TipDisplayWin *m_tipDisplayLay = nullptr;            // 居中显示当前目录状态以及搜索结果状态布局

    bool m_isConnect = false;                                                     // 当前是否连接
    FileSyncManage::FileType m_downFileType = FileSyncManage::FileType::FileNum;  // 当前下载类型
    FileSyncManage::FileKey m_fileKey = FileSyncManage::FileKey::Default;         // 当前文件同步类型
    QUrl m_url;                                                                   // 服务器地址
    ConnectionService::ConnectionInfo m_connectInfo;                              // 当前连接设备信息
    bool m_isDbusOperation = false;                                               // 当前操作是否为dbus调用
    bool m_isMountFtp = false;                                                    // 是否挂载ftp
    QString m_wifiIp = "";                                                        // 本机IP
    QString m_uploadPath = "";                                                    // 当前上传路径
    bool m_isPcSearchInfo = false;                                                // 是否为PC数据库文件
    int m_searchLoadNum = 0;                                                      // 搜索数据加载次数
    QString m_reconnectAddress = "";                                              // 重连ip
    quint32 m_inhibitValue = 0;                                                   // 熄屏参数
    PublicAttributes::Model m_model = PublicAttributes::Model::PcModel;           // 当前模式
    double m_fontSize = 10;                                                       // 系统字号
    QList<QStringList> m_backList;                                                // 回退列表
    QList<QStringList> m_forwardList;                                             // 前进列表
    bool m_isFileViewPage = true;                                                 // 当前是否为文件视图
    QStringList m_lastPathList;                                        // 搜索文件前最后一次面包屑路径
    QListView::ViewMode m_viewMode = QListView::IconMode;              // 手机存储当前视图模式
    QString m_tipMesseage = "";                                        // 提示消息
    PublicAttributes::Theme m_theme = PublicAttributes::Theme::Light;  // 主题
    bool m_isDownloadingThumbanil = false;
    bool m_isDusUpload = false;
    bool m_isOpeningFile = false;
    bool m_isRestartScreen = false;
    bool m_isClose = false;
};

#endif  // MAINWINDOW_H
