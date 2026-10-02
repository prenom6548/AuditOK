/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef KREINSTALL_H
#define KREINSTALL_H

#include <kysdk/kysdk-system/libkypackages.h>
#include "debinfowidget.h"
#include "batch-install.h"
#include "titlewidget.h"
#include "dbuscallthread.h"
#include "kylininstalldbuscallthread.h"
#include <sys/types.h>
#include <pwd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/file.h>
#include <syslog.h>
#include <unistd.h>
#include <QStringLiteral>
#include <QDesktopServices>
#include <QTextEdit>
#include <QListWidget>
#include <QTimer>
#include <QProgressBar>
#include <kaboutdialog.h>
#include <kmenubutton.h>
#include <QThreadPool>
#include <QStorageInfo>
#include <unistd.h>
#include <stdio.h>
#include <KWindowSystem>
#include <QApt/DebFile>
#include <QMimeDatabase>
#include <ukuistylehelper/ukuistylehelper.h>
#include <QCursor>
#include <QScreen>
#include <kprogressdialog.h>
#include <QProgressDialog>
#include <kborderlessbutton.h>
#include <kprogressbar.h>
#include <windowmanager/windowmanager.h>
#include <QPlainTextEdit>
#include <QPointer>

using namespace kdk;

#ifndef RUNTIME_XML_FILE_PATH
#define RUNTIME_XML_FILE_PATH "/opt/kre/config/runtimes.xml"
#endif

#define LOCK_FILE_PATH "/tmp/lock/kylin-update.lock"
#define LOCK_PATH "/tmp/lock/"

#define PROGRAM_NAME "kylin-installer"
#define PATH_MAX_LEN 1024
#define PID_STRING_LEN 64

#define ORG_UKUI_STYLE            "org.ukui.style"
#define GSETTING_KEY              "systemFontSize"
#define XSBC_KY_RUNTIME_ENV       "XSBC-KyRuntimeEnv"
#define XSBC_PERMISSIONS_LIST     "XSBC-PermissionsList"
#define KRE_VALUE                 "v10"
#define OS_VERSION_FILE_PATH      "/etc/lsb-release"
#define OS_VERSION_DESCRIPTION    "DISTRIB_DESCRIPTION"
#define OS_V10_PRO_VERSION        "\"Kylin V10 SP1\""
#define OS_V10_VERSION            "\"Kylin V10\""
#define OS_V10_PRO_LOGO           "kylin-10.1"
#define OS_V10_LOGO               "kylin-v10"

#define XML_APPLICATIONS_PATH "/opt/kre/config/applications_v2.xml"
class kreInstall : public QWidget
{
    Q_OBJECT
public:
    kreInstall(QString debPath);
    ~kreInstall();

    kdk::KAboutDialog *userabout ;
    QVBoxLayout      *m_pallLayout;
    debInfoWidget    *m_pdebWidget;
    QPointer<titleWidget> m_ptitleWidget;


    QHBoxLayout      *m_pinstHLayout;
    QLabel           *m_pruntimeLabel;
    QWidget          *m_pruntimeWidget;
    QLabel           *m_pinstToLabel;
    QWidget          *m_pblackWidget;
    QStringList       m_permissionList;
    QHash<QString, QCheckBox*> m_CheckBoxValue;

    QStackedWidget   *m_pAllStackedWidget;
    QVBoxLayout      *m_pAllStackedLayout;


    QProgressBar     *pProgressBar;
    QLabel           *progress;
    QLabel           *m_pKareInstallEnvStatusLabel;
    KBorderlessButton *m_pKareInstallEnvCancelPushButton;
    QWidget          *m_pInstallBeforeWidget;

    QWidget          *m_pInstallBtnWidget;
    QHBoxLayout      *m_pInstallBtnLayout;
    QPushButton      *m_pInstallBtn;
    QWidget          *m_pInstallIngWidget;
    QVBoxLayout      *m_pInstallIngLayout;
    QWidget          *m_pInstallIngLabelWidget;
    QHBoxLayout      *m_pInstallIngLabelLayout;
    QLabel           *m_pInstallIngLabel;
    debInfoWidget    *m_pdebInstallIngWidget;
    QWidget          *m_pInstallFailWidget;
    QVBoxLayout      *m_pInstallFailLayout;
    QWidget          *m_pInstallFailLabelWidget;
    QWidget          *m_pInstallFailbuttonWidget;
    QVBoxLayout      *m_pInstallFailLabelLayout;
    QHBoxLayout      *m_pInstallFailbuttonLayout;
    QLabel           *m_pInstallFailLabel;
    QLabel           *m_pInstallFailIcon;
    QWidget          *m_pInstallSuccessWidget;
    QVBoxLayout      *m_pInstallSuccessLayout;
    QWidget          *m_pInstallSuccessLabelWidget;
    QVBoxLayout      *m_pInstallSuccessLabelLayout;
    QLabel           *m_pInstallSuccessLabel;
    QLabel           *m_pInstallSuccessIcon;
    QPushButton      *m_pInstallSuccessButton;

    /*安装日志界面*/
    QWidget          *m_pInstallLogWidget;
    QLabel           *m_pInstallLogLabel;
    QHBoxLayout      *m_pInstallLOGLabelLayout;
    QHBoxLayout      *m_pInstallLOGLabelTextLayout;
    QVBoxLayout      *m_fiallogtextlayout;
    QHBoxLayout      *m_pInstallLogbuttonlayout;
    QPushButton      *m_pInstallLogbutton;
    QTextEdit        *m_pInstallLogtextedit;
    QPushButton      *m_pInstallFaillog;
    QPushButton      *m_pInstallFailsure;
    QPushButton      *return_homepage_log1;

    QHBoxLayout      *m_pInstallLOGwidgetLayout;
    QWidget          *m_pInstallLogWidget_min;

    /*********批量安装界面******/
    QVBoxLayout      *Batch_installlayout;
    QVBoxLayout      *Batch_installlayout_ALL;
    QLabel           *Batch_installIcon;
    QLabel           *Batch_installdescribe;
    QPushButton      *Batch_installselect;

    QListWidget      *batchInstallWidgetList;


    QWidget          *Batch_installwidget_1;
    QWidget          *Batch_installwidget_1a;
    QWidget          *Batch_installwidget_1b;
    QPushButton      *Batch_addinstall;
    QPushButton      *Batch_install;
    QPushButton      *Batch_viewlog;
    QPushButton      *Batch_return;
    QHBoxLayout      *Batch_installlayout_batch;
    QHBoxLayout      *Batch_installlayout_add;

    QString          flagstring;

    /**********批量安装完成后的界面*******/
    QWidget          *Batch_installsuccesswidget;
    QWidget          *Batch_installsuccessiconwidget;
    QWidget          *Batch_installsuccesstextwidget;
    QVBoxLayout      *Batch_installsuccesslayout;
    QHBoxLayout      *Batch_installsuccessbuttonlayout;
    QVBoxLayout      *Batch_installsuccesslayout_ALL;
    QLabel           *Batch_installsuccessIcon;
    QLabel           *Batch_installsuccesstext;
    QPushButton      *Batch_installsuccessdetemin;
    QPushButton      *Batch_installsuccessreturn;

    Batchinstall     *batchInstallWidget;
    QLabel           *init_logoLabel;

    bool SECENTSTART = false;  //判断是否是第二次覆盖启动
    bool EXITQMESSG = false;

    // 主界面
    QWidget         *m_pOsInstallWidget;
    debInfoWidget   *m_pOsDebMainWidget;
    QVBoxLayout     *m_pOsVboxLayoutWidget;
    QPushButton     *m_pOsInstallButton;
    QPushButton     *m_cansel_InstallButton;

    //安装中界面
    QWidget         *m_pOsInstallIngWidget;
    QWidget         *m_pPackageInstallLabelWidget;
    QWidget         *m_pKareInstallEnvLabelWidget;
    debInfoWidget   *m_pOsDebInstallIngWidget;
    QVBoxLayout     *m_pOsVboxLayoutInstallIngWidget;
    QLabel          *m_pOsInstallIngLabel;

    // kare安装界面
    KProgressDialog  *m_pKareEnvProgressDialog;

    QString          m_debPath;
    QString          m_runtime;
    QString          m_appName;
    QString          m_debArch;
    QString          m_appVersion;
    QString          m_application;
    QString          m_applicationZh;
    QString          m_installed_version;
    QString          kylin_installerversion;
    QString          m_installed_status;
    QString          m_OsVersionLogo;
    QGSettings       *m_pGsettingFontSize;
    QStringList      m_kylinRuntimeEnvList;

    QTimer *icontime;
    QTimer *processUITimer;

    DbusCallThread   *m_pDbusCall;
    std::unique_ptr<KylinInstalldbusCallThread>  m_pKylinInstalldbusCallThread;

    QStringList * pkgListAdded      = new QStringList();
    QStringList * pkgMd5ListAdded   = new QStringList();

    QMimeType mime;

    int install_flag = 0;

    bool CACLINFO;

    int canceladd = 0;

    bool CANSCELISNTALL = false;

    QMap<QString, QString> maplist;

    QString path_deb;//用来记住包名相同版本不同时判断的包的路径选择；

    bool appinstallstatus = true;

    QString change_installapp;

    long settime = 0;

    bool FilePath = true;

    int fontsize;
    bool resetCloseFlag = false;

    QString historyPath = QDir::homePath();

    bool kareEnvBuilded = false;

    void installTranslator();

    void showGuide();

    void initKareProgressDialog();

    void initDebInfo(QString debPath);

    void mainWidgetInit();

    void gsettingInit();

    void debDamageInfo();

    bool ArchIsMatch(QString depath_arch,QString tmp);

    void initOsMainWidget();

    void initOsLayoutWidget();

    bool eventFilter(QObject *watched, QEvent *event); //事件过滤器

    void initOsInstallIngWidget();

    void initOsLayoutIngWidget();

    void initInstallBtnWidget();

    void getOsVersionValue();

    QFont  getSystemFont(int size);

    QString getDebInfoFromDeb(QString debPath,QString type);

    QString      SpliteText(QFontMetrics font, QString text,int nlablesize);

    void kreInStallUnlock();

    void isFile();

    void resetDebInfo();

    void getKreAppDesktopFileName();

    QString getUserName();

    QString getUid();

    bool isAndroidReady(const QString &username, const QString &uid);

    int isAndroidEnvInstalled();

    void batch_installWidgeted(QStringList debpath);

    void deletewidgetitem(QString itemwidget);

    void changebuttonstatus(QString itemwidget,int dex);

    void adddebpathcompail(QString);

    void thread_install_deb();

    bool app_installed(QString debpathstring);

    void app_installed_fast();

    void moveCenterScreen();

    void Batch_setInstallThread(QString debPath);

    void setInstallThread(QString debPath);

    void updateKareProgressDialog();

    void playSoundEffects();

    void batchInstallListInit(QStringList);

    // 弹窗ui
    // 执行安装命令的子进程
    QProcess* process;

protected:
    void paintEvent(QPaintEvent *event);

    void changeEvent(QEvent *event) override;

public slots:
    void initInstallerUI();

    void initInstallerLogo();

    void deleteInstallerLogo();

    void addBtnClicked();

    void installBtnClicked();

    void minBtnClicked();

    void closeBtnClicked(bool status);

    void installSuccessSlot();

    void installFailSlot();

    void InstalldingStatus(int status);

    void batchInstalldingStatus(int status,QString appname);

    void OsInstallButtonSlots();

    void Batch_OsInstallButtonSlots();

    //单个安装完成信号槽
    void osInstallOverSoftwareAptSlots(bool status, QString msg);
    //批量安装完成信号槽
    void Batch_osInstallOverSoftwareAptSlots(bool status, QString msg);

    void secondaryInstallation(QString debPath);

    void showinstallfaillogtext();

    void return_homepage();

    void batchlist();

    void onAnalysisApkFile(QString debPath);

    void automatic_deletewidgetitem();

    void play_installingpicture();

    void cancel_installed();
    void continue_install();
    void canseaddapplication();
    void replace_application();

    void kareBaseEnvInstallStatusSlot(bool status, QString info);
    void kareBaseEnvInstallingStatusSlot(int progress);
    void Batch_kareBaseEnvInstallStatusSlot(bool status, QString info);

private:
    void kareEnvResetConnect();
    void kareEnvRestoreConnect();
    bool confirmInstallKareEnvMessageBox();
    void setKareVersionShow();

signals:
    void uiReady();
};
#endif // KREINSTALL_H
