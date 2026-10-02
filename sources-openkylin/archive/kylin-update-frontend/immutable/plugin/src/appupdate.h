#ifndef APPUPDATE_H
#define APPUPDATE_H

#include <QPushButton>
#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDateTime>
#include <QTextEdit>
#include <QToolTip>
#include "utils.h"
#include "updatelog.h"
#include <QMetaType>
#include <QTimer>
#include "updatedbus.h"
#include <QCheckBox>
#include "mylabel.h"
#include "src/Data/global.h"
#include <kborderbutton.h>
#include <kborderlessbutton.h>
#include <kdialog.h>
#include "detaildialog.h"

#include <QDBusError>

#include <stdio.h>
#include <iostream>
#include <yaml-cpp/yaml.h>
#include <yaml-cpp/exceptions.h>
//#include <ukcc/interface/ukcccommon.h>
#include "ukcc/interface/upgradeukcccommon.h"
//#include <ukcc/widgets/lightlabel.h>
#include "ukcc/widgets/lightlabel.h"
class Detaildialog;
class UpdateLog;
using namespace kdk;
class AppUpdateWid : public QWidget
{
    Q_OBJECT
public:
    explicit AppUpdateWid(AppAllMsg msg, QWidget *parent = nullptr);
    ~AppUpdateWid();




//    Detaildialog *detaillog;

    QString appname = nullptr;

    QFrame *AppFrame;
    UpdateLog *updatelog1;
    UpdateDbus *m_updateMutual;
    //缩略界面
    QLabel *appIcon;
    QLabel *appIconName;
    MyLabel *appNameLab;
    FixLabel *appVersion;
    QLabel *appVersionIcon;

    QLabel *progressLab;  //进度
    QLabel *otherBtnLab;
    QLabel *versionLab;
    KBorderlessButton *detaileInfo;
    QPushButton *updateAPPBtn;
    QWidget *appTitleWid;
    QHBoxLayout *frameLayout;
    QHBoxLayout *iconNameLayout;
    QHBoxLayout *smallHLayout;
    QVBoxLayout *largeVLayout;
    QHBoxLayout *versionlabLayout;
    QHBoxLayout *otherBtnLayout;
    QWidget *largeWidget;
    QHBoxLayout *mainHLayout;
    //展开界面
    QTextEdit *someInfoEdit;  //详情内容
    QPushButton *updatelogBtn;
//    AppMsg *thisAppMessage;
    QString chlog;  //更新日志完整内容
    QString description;  //更新包描述
    QString dispalyName;
private:
    bool isCancel = false;
    int connectTimes = 0;
    bool downloadFinish = false;
    bool stopTimer = false;
    UrlMsg urlmsg;
    QString path;
    QString currentPackage;
    QStringList downloadList;
    QStringList downloadPackages;
    AppAllMsg appAllMsg;
    QProcess *downloadProcess;
    QProcess *workProcess;
    QTimer *timer;
    QString downloadPath;

    bool execFun = true;

public:
    bool isUpdateAll = false;
    bool isAutoUpgrade = false;
    bool haveThemeIcon = false;
    bool downloadstart = false;
    bool shutdown_install = false;
    bool CancelBtnClicked = false;

public slots:
    void updateAllApp();
private:

signals:
    void startWork(QString appName);
    void startMove(QStringList list, QString appName);
    void oneappUpdateResultSignal(bool state,QStringList pkgname,QString error,QString reason);
    void changeUpdateAllSignal(bool isUpdate);
    void downloadFailedSignal(int exitCode);  //网络异常或者其他情况下下载失败时
    void appupdateiscancel();
    void changeTabWidText();
    void sendProgress(QString pkgName, int Progress, QString type);
    void changeupdateall();
    void allappupdatefinishsignal();
    void startoneappupdate();
    void ShutdowninstallFinishSignal();//shutdownfinishsignal

private:
    QString pkgIconPath = "";


/*V11 content*/
public slots:
    void backupResult(bool result);
    void showDetails();
    void oneAppDependResloveResult(bool,bool,QStringList,QStringList,QStringList,QString,QString);
    void showDownloadStatues(int currentserial,int total,quint64 downloadedsize,quint64 totalsize,int speed);
    void showDownloadFinsih(bool state,QStringList pkgname,QString error,QString reason);
public:
    QDBusInterface *backup_interface;
    /*业务逻辑函数*/
    void cancelOrUpdate();
    void startPreUpdate();
    void startBackup();
    void startUpdate();
    /*非业务逻辑函数*/
    QString modifySizeUnit(long long size);
    QString modifySpeedUnit(long size, float time);
private:
    void initConnect(); //初始化信号槽
    void updateAppUi(QString name);
    QMap<QString, QString> getNameAndIconFromJson(QString pkgname);
};

#endif // APPUPDATE_H
