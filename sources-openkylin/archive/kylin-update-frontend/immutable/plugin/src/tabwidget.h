#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QWidget>
#include <QTabWidget>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QListWidget>
#include <QListWidgetItem>
#include <QCheckBox>
#include <QFont>
#include <QFile>
#include <QProgressBar>
#include <QComboBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QtDBus/QtDBus>
#include <QFontMetrics>
#include <QProcess>
#include <QStandardPaths>
#include <QSettings>


#include "appupdate.h"
#include "m_updatelog.h"
#include "ukscconn.h"
#include "checkbutton.h"
#include "AddFunction/datetimeutils.h"
//#include "ukcc/widgets/switchbutton.h"
#include "ukcc/widgets/titlelabel.h"
#include "ukcc/widgets/fixlabel.h"
#include "ukcc/widgets/lightlabel.h"
#include "ukcc/interface/upgradeukcccommon.h"
#include "src/Data/global.h"
#include <kwidget.h>
#include <kprogressbar.h>
#include <kswitchbutton.h>
#include <kprogressdialog.h>
#include "SecondaryWidget/setwidget.h"

using namespace kdk;

#define CRUCIAL_FILE_PATH "/var/lib/kylin-software-properties/template/crucial.list"
#define IMPORTANT_FIEL_PATH "/var/lib/kylin-software-properties/template/important.list"


const int needBack = 99;
const int netErrorRetry = 3;

enum{
    UpdateAll=1,
    UpdatePart,
    UpdateSystem,
};

struct BackupEntry {
    QString description;
    QString uuid;
    QDateTime timestamp;
};

enum{
//    ACTION_DEFUALT_STATUS=-1,    //默认状态空闲状态
//    ACTION_UPDATE,               //处于更新cache状态
//    ACTION_INSTALL,              //处于升级安装包括部分升级、全部升级状态
//    ACTION_INSTALL_DEB,          //处于安装deb的状态
//    ACTION_CHECK_RESOLVER,       //处于计算依赖过程
//    ACTION_DOWNLOADONLY,         //单独下载软件包过程
//    ACTION_FIX_BROKEN,           //修复依赖的过程
//    ACTION_REMOVE_PACKAGES,      //卸载包的状态中

    ACTION_DEFUALT_STATUS = -1,
    ACTION_UPDATE = 0,
    ACTION_DOWNLOADONLY = 1,
    ACTION_DEPLOY = 2,
    ACTION_ROLLBACK = 3,
};

class TabWid : public QWidget
{
    Q_OBJECT
public:
    explicit TabWid(QWidget *parent = nullptr);
    ~TabWid();
    void allComponents(); //更新页面里所有组件
    void getpkginfoFromJson(AppAllMsg *msg,QString pkgname);
    void getAllDisplayInformation();
//    void allBinding();  //更新页面里所有组件绑定
    void checkUpdateBtnClicked();  //检测更新、全部更新按钮
    void updateAllApp();
    void dependencyupdateAll(bool status);
    void disupdateallaccept();
    /*2309需求，海关管控项目*/
    QVariantMap getModuleHideStatus();
    void setVisibleBySecurity();
    /*2309需求，海关管控项目*/
    QString getversion();
    QString modifySpeedUnit(long size, float time);
    QString modifySizeUnit(long size);
    QString caltime(uint downloadedsize,uint totalsize,int speed);
    QString getLanguageEnv();
    QString allLableData(bool one,QString detailinfo,uint downloadedsize,uint totalsize,int speed);

    //选项卡页面，具有更新和更新设置两个选项卡及对应界面
    QTabWidget *updateWidget;
    AppUpdateWid *updatewid;
    TitleLabel *labUpdate;
    QWidget *updateTab;
    QWidget *updateSettingTab;
    QVBoxLayout *mainTabLayout;
    QFont ft;
    QCheckBox *isAutoCheckBox;

    //更新页面布局
    QHBoxLayout *tab1HLayout;
    QFrame *systemWidget;
    QVBoxLayout *AppMessage;
    QScrollArea *scrollArea;

    //系统头像
    QLabel *systemPortraitLab;
    m_button *checkUpdateBtn;  //检测更新
    //三种状态下的版本信息   显示当前版本、可更新版本、或最新版本
    QLabel *versionInformationLab;
    QWidget *versionErrorcodeLab;
//    KBorderlessButton *errorcodeButoon;
    QHBoxLayout* versionErrorcodeLablayout;
    LightLabel *lastRefreshTime;
    KProgressBar *allProgressBar;
    QLabel *progressLabel;
    FixLabel *detailLabel;
    QPushButton *historyUpdateLog;  //历史更新日志界面
    QPushButton *AdvancedOption;
    QPushButton *updatedetaileInfo;
    QVBoxLayout *inforLayout;
    QVBoxLayout *controlLayout;

     //更新设置页面组件
    QFrame *updateSettingWidget;
    TitleLabel *updateSettingLab;
    QVBoxLayout *updatesettingLayout;
    QFrame *isAutoCheckWidget;
    QHBoxLayout *isAutoCheckLayout;
    QLabel *isAutoCheckedLab;

    KSwitchButton *isAutoCheckSBtn;
    //download limit widgets
    QFrame *DownloadHWidget;
    QFrame *DownloadVWidget;
    QHBoxLayout *DownloadHLayout;
    QVBoxLayout *DownloadVLayout;
    QLabel *DownloadHLab;
    LightLabel *DownloadVLab;
    KSwitchButton *DownloadHBtn;
    QComboBox *DownloadHValue;
    QHBoxLayout *DownloadVLabLayout;

    QFrame *isAutoUpgradeWidget;
    QVBoxLayout *isAutoUpgradeLayout;
    QHBoxLayout *autoUpgradeLayout;
    QHBoxLayout *autoUpgradeBtnLayout;
    QLabel *isAutoUpgradeLab;
    LightLabel *autoUpgradeLab;
    KSwitchButton *isAutoUpgradeSBtn;
    /*二選一*/
    QButtonGroup *m_pButtonGroup;
    QRadioButton *poweroffUpgradeBtn;
    QRadioButton *ontimeUpgradeBtn;
    QComboBox *ontimeUpgradeValue;
    QLabel *ontimeUpgradeLab;
    QHBoxLayout *ontimeUpgradeLayout;


    /*switchbutton4*/
    QFrame *TrailWidget;
    FixLabel *TrailLabel1;
    FixLabel *TrailLabel2;
    KBorderlessButton *TrailBorderlessButton;
    QHBoxLayout *TrailLayout;
    KSwitchButton *TrailCheckBtn;

    /*switchbutton5*/
    QFrame *RestoreWidget;
    QLabel *RestoreLabel;
    QPushButton *RestoreButton;
    QHBoxLayout *RestoreLayout;

    QFrame *line12;
    QFrame *line23;
    QFrame *line34;
    QFrame *line45;

    QWidget *allUpdateWid;
    QVBoxLayout *allUpdateLayout;

    QList<AppUpdateWid *> widgetList;

    bool isAllUpgrade = false;
    bool isAutoUpgrade = false;
    bool isContinueUpgrade = false;
    bool isCancel = false;
    bool alldownloadstart = false;
    int inumber = 0;
    int retryTimes = 0;

    bool backstatus;

    QString result = "";

    QString version;
    QString checkedtime;
    /*错误码机制*/
    QString error_code;
    void showUserGuide(QString programName);
    /*由信号或者按键弹出的弹窗*/
    m_updatelog *historyLog;
    QDBusInterface * m_dateSessionDbus = nullptr;
    QDBusInterface *soundplayeriface;
//    QDialog *historyLog;
    bool fileLockedStatus = false;  //等待静默安装提示是否弹出
    UpdateDbus *updateMutual;
    bool downloadFailedStatus = false;  //下载失败时的弹窗是否弹出

    /*additional function*/
    DateTimeUtils *datetimeutils;

    int allProgress = 0;
    int allUpgradeNum = 0;
    int lastprogress = 0;

    QFileSystemWatcher* filewatcher;

    QList<pkgProgress> pkgList;
public slots:
    void ShowAdvancedOption();
    void showHistoryWidget();
    void isAutoCheckedChanged();
    void isAutoUpgradeChanged();
    void slotCancelDownload();
    void changeTabWidTextSlot();
    void loadingOneUpdateMsgSlot(AppAllMsg msg); //逐个加载更新
    void loadingFinishedSlot(); //加载完毕信号

    void hideUpdateBtnSlot(bool state,QStringList pkgname,QString error,QString reason);
    void hideUpdateBtnSlotindownload(bool state,QStringList pkgname,QString error,QString reason);
    void oneappUpdateresultSlot(bool state,QStringList pkgname,QString error,QString reason);
    void shutdowninstallfinishslot();
    void allappupdatefinishSlot();
    void changeUpdateAllSlot(bool isUpdate);
    void updatecancel(void);
    void startoneappupdateslot(void);

    void DownloadLimitSwitchChanged();
    void DownloadLimitValueChanged(const QString &);
    void DownloadLimitChanged();
    void ontimeUpgradeValueChanged(const QString &);
    void showDownloadInfo(int,int,quint64,quint64,int);
    void DependResloveResult(bool ResloveStatus, bool Reslove, QStringList deletepkglist, QStringList DeletePkgDeslist,QStringList DeletePkgReasonlist,QString error, QString reason);
    void InstallStatus(bool success,QString err);
    void DistupgradeDependResloveResult(bool ResloveStatus, bool Reslove, QStringList deletepkglist, QStringList DeletePkgDeslist, QStringList DeletePkgReasonlist, QString error, QString reason);
    //调用源管理器相关
    void slotUpdateCache(bool result,QStringList pkgname, QString error ,QString reason);
    void slotUpdateCacheProgress(int progress, QString status);
    /*分步更新新增函数*/
//    void SetInstallStatus(bool,QString);
    /*策略下发功能*/
    void SecurityDownloadChange(QString,QString);
    //解决首次运行卡顿问题
    //DBus单独初始化
    void initDbus();

    void slotReconnTimes(int times);

    void isCancelabled(bool status);
    //自动更新相关
    void progresschanged();
    /*错误码机制*/
    void GetErrorCode(bool,QString);
    /*长短格式时间监听与设置*/

    void UpdateSdkTime(QString dateformat);

    void FeedbackStatusChanged();
    void ShowFeedback();
private:
    UKSCConn *ukscConnect;
    bool isConnectSourceSignal = false;
    bool distUpgradeAllApp(bool status);

    AppAllMsg appAllMsg;

    void backupMessageBox(QString str);
    void backupHideUpdateBtn(int result);

    bool get_battery();
    bool autoUpdateLoadUpgradeList(bool isBackUp);
//自动更新相关
public Q_SLOTS:
    void autoupgradestatusshow();
    void autoinstallfinish();


signals:
    int needBackUp();
    void startBackUp(int);

public slots:
    void isAutoBackupChanged();
    void dbusFinished();

private slots:
    void whenStateIsDuing();
    void OpenBackupTool();

/*v11 内容*/
public:
    BackupEntry parseBackupEntry(const QString &entry);
};

#endif // TABWIDGET_H
