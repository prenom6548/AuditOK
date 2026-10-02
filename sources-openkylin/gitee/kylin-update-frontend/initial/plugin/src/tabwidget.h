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

#include "appupdate.h"
#include "m_updatelog.h"
//#include "updatedeleteprompt.h"
#include "updatesource.h"
#include "ukscconn.h"
#include "backup.h"
#include "checkbutton.h"
//#include <ukcc/widgets/switchbutton.h>
#include <ukcc/widgets/titlelabel.h>
#include <ukcc/widgets/fixlabel.h>
#include <ukcc/widgets/lightlabel.h>
#include "global.h"
#include <kwidget.h>
#include <kprogressbar.h>
#include <kswitchbutton.h>
#include "fixbrokeninstalldialog.h"
#include "dependencyfixdialog.h"
#include "fixupdetaillist.h"
#include "updatedeleteprompt.h"

#include <kysdk/kysdk-base/libkydiagnostics.h>


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

enum{
    ACTION_DEFUALT_STATUS=-1,    //默认状态空闲状态
    ACTION_UPDATE,               //处于更新cache状态
    ACTION_INSTALL,              //处于升级安装包括部分升级、全部升级状态
    ACTION_INSTALL_DEB,          //处于安装deb的状态
    ACTION_CHECK_RESOLVER,       //处于计算依赖过程
    ACTION_DOWNLOADONLY,         //单独下载软件包过程
    ACTION_FIX_BROKEN,           //修复依赖的过程
    ACTION_REMOVE_PACKAGES,      //卸载包的状态中

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
    void updateAllApp(bool status);
    void dependencyupdateAll(bool status);
    void updatedeletepromptaccept();
//    void updateAllbyprompt();
    void disupdateallaccept();
    QString getversion();
//    void showdetaillist(QStringList pkgname,QStringList description,QStringList deletereason);
    QString modifySpeedUnit(long size, float time);
    QString modifySizeUnit(long size);
    QString caltime(uint downloadedsize,uint totalsize,int speed);
    QString getLanguageEnv();
    QString allLableData(bool one,QString detailinfo,uint downloadedsize,uint totalsize,int speed);

    /**
     * @brief 埋点
     *
     * @param pluginName 插件名
     * @param settingsName 设置名
     * @param action 操作
     * @param value 设置的值
     * @return bool 是否埋点成功
     */
    static bool buriedSettings(QString pluginName, QString settingsName, QString action, QString value = nullptr);

//    void updateSettings(); //更新设置页面

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
    //更新设置页面布局
//    QHBoxLayout *tab2HLayout;
    QVBoxLayout *tab2VLayout;

    //系统头像
    QLabel *systemPortraitLab;
    m_button *checkUpdateBtn;  //检测更新
    //三种状态下的版本信息   显示当前版本、可更新版本、或最新版本
    QLabel *versionInformationLab;
    LightLabel *lastRefreshTime;
    KProgressBar *allProgressBar;
    QLabel *progressLabel;
    FixLabel *detailLabel;
    QPushButton *historyUpdateLog;  //历史更新日志界面
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

//    SwitchButton *isAutoCheckSBtn;
    KSwitchButton *isAutoCheckSBtn;
    QFrame *isAutoBackupWidget;
    QHBoxLayout *isAutoBackupLayout;
    QLabel *isAutoBackupLab;
    KSwitchButton *isAutoBackupSBtn;
    //download limit widgets
    QFrame *DownloadHWidget;
    QFrame *DownloadVWidget;
    //QFrame *isDownloadWidget;
    QHBoxLayout *DownloadHLayout;
    QVBoxLayout *DownloadVLayout;
    QLabel *DownloadHLab;
    LightLabel *DownloadVLab;
//    SwitchButton *DownloadHBtn;
    KSwitchButton *DownloadHBtn;
    QComboBox *DownloadHValue;
    QHBoxLayout *DownloadVLabLayout;

    QFrame *isAutoUpgradeWidget;
    QVBoxLayout *isAutoUpgradeLayout;
    QHBoxLayout *autoUpgradeLayout;
    QHBoxLayout *autoUpgradeBtnLayout;
    QLabel *isAutoUpgradeLab;
    LightLabel *autoUpgradeLab;
//    SwitchButton *isAutoUpgradeSBtn;
    KSwitchButton *isAutoUpgradeSBtn;
    /*二選一*/
    QButtonGroup *m_pButtonGroup;
    QRadioButton *poweroffUpgradeBtn;
    QRadioButton *ontimeUpgradeBtn;
    QComboBox *ontimeUpgradeValue;
    QLabel *ontimeUpgradeLab;
    QHBoxLayout *ontimeUpgradeLayout;

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

    bool status;

    QString version;

    /*由信号或者按键弹出的弹窗*/
    m_updatelog *historyLog;
//    updatedeleteprompt *dependsloveptompt;
    fixbrokeninstalldialog *fixbrokeninstall;
    dependencyfixdialog *dependfixdialog;

//    fixupdetaillist *m_instance;
    updatedeleteprompt *m_instance;

//    QDialog *historyLog;
    bool fileLockedStatus = false;  //等待静默安装提示是否弹出
    UpdateDbus *updateMutual;
    bool downloadFailedStatus = false;  //下载失败时的弹窗是否弹出
    //源管理器Dbus对象
    UpdateSource *updateSource;

    int allProgress = 0;
    int allUpgradeNum = 0;
    int lastprogress = 0;

    QFileSystemWatcher* filewatcher;

    QList<pkgProgress> pkgList;

signals:
//    void send_Signal();
//    void parameterSignal(int i);
    void updateAllSignal(bool status);
public slots:
    void showHistoryWidget();
    void showDependSlovePtompt(int updatemode, QStringList pkgname,QStringList description,QStringList deletereason);
    void showFixbrokenPtompt(int updatemode, QStringList pkgname,QStringList description,QStringList deletereason);
    void isAutoCheckedChanged();
    void isAutoUpgradeChanged();
    void slotCancelDownload();
    void loadingOneUpdateMsgSlot(AppAllMsg msg); //逐个加载更新
    void loadingFinishedSlot(int size); //加载完毕信号
    void waitCrucialInstalled();  //等待静默更新安装完的消息提示

    void hideUpdateBtnSlot(bool state,QStringList pkgname,QString error,QString reason);
    void hideUpdateBtnSlotindownload(bool state,QStringList pkgname,QString error,QString reason);
    void oneappUpdateresultSlot(bool state,QStringList pkgname,QString error,QString reason);
    void shutdowninstallfinishslot();
    void allappupdatefinishSlot();
    void changeUpdateAllSlot(bool isUpdate);
    void updatecancel(void);
    void closedpendencyfixdialog();
    void startoneappupdateslot(void);

    void DownloadLimitSwitchChanged();
    void DownloadLimitValueChanged(const QString &);
    void DownloadLimitChanged();
    void ontimeUpgradeValueChanged(const QString &);
    void getAllProgress (QStringList pkgName,int Progress,QString status,QString detailinfo);
    void showDownloadInfo(QStringList,int,int,uint,uint,int);
    void DependResloveResult(bool ResloveStatus, bool Reslove, QStringList deletepkglist, QStringList DeletePkgDeslist,QStringList DeletePkgReasonlist,QString error, QString reason);
    void FixBrokenResloveResult(bool ResloveStatus, bool Reslove, QStringList deletepkglist, QStringList DeletePkgDeslist,QStringList DeletePkgReasonlist,QString error, QString reason);
    void InstallStatus(bool success,QString err);
    void DistupgradeDependResloveResult(bool ResloveStatus, bool Reslove, QStringList deletepkglist, QStringList DeletePkgDeslist, QStringList DeletePkgReasonlist, QString error, QString reason);
    //调用源管理器相关
    void slotUpdateTemplate(QString status);
    void slotUpdateCache(bool result,QStringList pkgname, QString error ,QString reason);
    void slotUpdateCacheProgress(int progress, QString status);

    //解决首次运行卡顿问题
    //DBus单独初始化
    void initDbus();

    void slotReconnTimes(int times);

    void isCancelabled(bool status);
    //自动更新相关
    void progresschanged();

    void fixbrokencancel();
    void fixbrokenremove();
    //修复依赖详情弹窗
    void showdetaillist(QStringList pkgname,QStringList description,QStringList deletereason,int mode);

private:
    UKSCConn *ukscConnect;
    bool isConnectSourceSignal = false;
    bool distUpgradeAllApp(bool status);
//备份还原相关
    void bacupInit(bool isConnect);
    void backupDelete();
    void backupCore();
    BackUp *backup = nullptr;
    QThread *backupThread = nullptr;

    AppAllMsg appAllMsg;

    void backupMessageBox(QString str);
    void backupHideUpdateBtn(int result);

    void getAutoUpgradeStatus();
    bool get_battery();
    bool autoUpdateLoadUpgradeList(bool isBackUp);

    void fileLock();
    void fileUnLock();
//自动更新相关
public Q_SLOTS:
    void autoupgradestatusshow();
    void autoinstallfinish();
    void backupfailure();


signals:
    int needBackUp();
    void startBackUp(int);

public slots:
    void bakeupFinish(int);
    void backupProgress(int);
    void isAutoBackupChanged();

    void getReplyFalseSlot();
    
    void dbusFinished();

private slots:
    void receiveBackupStartResult(int);
    void whenStateIsDuing();
};

#endif // TABWIDGET_H
