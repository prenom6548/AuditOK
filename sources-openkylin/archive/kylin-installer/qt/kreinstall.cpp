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
#include <unistd.h>
#include "kreinstall.h"
#include <QPainterPath>
#include <dlfcn.h>
#include <QCoreApplication>
#include <QApt/Backend>
#include <QDesktopWidget>
#include "widbox.h"
#include <QShortcut>
#include <QJsonArray>
#include "package/PackageUtils.h"
#include "QProgressDialog"
#include "../src/common/utils/CustomBoxUtil.h"
#include "../src/common/utils/SystemInfoUtil.h"
#include "package/PackageFactory.h"
#include "package/Package.h"
#include "package/PackageCheckUtils.h"

int gi_fd;
bool install_status = false;

kreInstall::kreInstall(QString deb_path):m_pKareEnvProgressDialog(nullptr),m_pKylinInstalldbusCallThread(nullptr)
{
    QFile filepath(deb_path);
    if (filepath.exists())
        FilePath = true;
    else
        FilePath = false;
    
    m_debPath = deb_path;

    this->setFixedSize(500,600);
    this->setWindowTitle(QObject::tr("Installer"));

    moveCenterScreen();

    initInstallerLogo();

    processUITimer = new QTimer();
    connect(processUITimer, SIGNAL(timeout()), this, SLOT(initInstallerUI()));
    processUITimer->start(60);
}

kreInstall::~kreInstall()
{
    kreInStallUnlock();
}

void kreInstall::initInstallerUI()
{
    processUITimer->stop();

    setWindowIcon(QIcon::fromTheme("kylin-installer"));

    this->setAttribute(Qt::WA_AlwaysShowToolTips);

    /* 初始化deb信号，获取deb信息 */
    initDebInfo(m_debPath);

    /*判断文件是否存在*/
    isFile();

    m_ptitleWidget = new titleWidget(this);
    m_ptitleWidget->show();

    connect(m_ptitleWidget->m_pMinBtn,   SIGNAL(clicked(bool)), this, SLOT(minBtnClicked()));
    connect(m_ptitleWidget->m_pClostBtn, SIGNAL(clicked(bool)), this, SLOT(closeBtnClicked(bool)));

    /*系统字体大小gsetting初始化*/
    gsettingInit();

    deleteInstallerLogo();

    // 麒麟安装器启动界面
    initOsMainWidget();
    initOsLayoutWidget();

    // 麒麟安装器安装中界面
    initOsInstallIngWidget();
    initOsLayoutIngWidget();
    moveCenterScreen();
    initInstallBtnWidget();

    // 所有界面初始化
    mainWidgetInit();

    // 根据系统下的lsb-release文件，来判别当前系统版本
    getOsVersionValue();
    initKareProgressDialog();
    m_pAllStackedWidget->setCurrentIndex(4);

    //F1按键显示用户手册
    QShortcut *shortcut = new QShortcut(QKeySequence(Qt::Key_F1), this);
    connect (shortcut, &QShortcut::activated, this, [=] {
        // kdk::kabase::UserManualManagement userManualTest;
        // if (!userManualTest.callUserManual("kylin-installer")) {
        //     qCritical() << "user manual call fail!";
        // }
        showGuide();
    });

    emit uiReady();
}

void kreInstall::initInstallerLogo()
{
    init_logoLabel = new QLabel(this);
    QIcon icon = QIcon::fromTheme("kylin-installer");
    init_logoLabel->setPixmap(icon.pixmap(96, 96)); 
    init_logoLabel->setAlignment(Qt::AlignCenter);
    init_logoLabel->setGeometry(rect());
    init_logoLabel->show();
}

void kreInstall::deleteInstallerLogo()
{
    if (init_logoLabel) {
        init_logoLabel->hide();
        init_logoLabel->deleteLater();
        init_logoLabel = nullptr;
    }
}

void kreInstall::installTranslator()
{
    QFileInfo file("/usr/share/qt5/translations/qt_zh_CN.qm");
    QTranslator * translator = new QTranslator();
    if (file.exists()!=false) {
        translator->load("/usr/share/qt5/translations/qt_zh_CN.qm");
        QApplication::installTranslator(translator);
    }

    if (translator->load(QLocale(), QLatin1String("kre-install"), QLatin1String("_"), QLatin1String("/usr/share/kre/kre-install/translations"))) {
        QApplication::installTranslator(translator);
    } else
        qDebug() << "cannot load translator kre-install_" << QLocale::system().name() << ".qm!";

    QTranslator trans1;
    QString local1 = QLocale::system().name();
    if (trans1.load(QString(":translations/gui_" + local1 +".qm"))) {
        QApplication::installTranslator(translator);
    }
}

void kreInstall::isFile()
{
    if(FilePath == false)
        return;

    QFileInfo file(m_debPath);
    if (file.exists()==false) {
        QMessageBox *msgBox = CustomBoxUtil::createCritialBox(tr("open file error"),tr(" file does not exist "),"",this);
        msgBox->exec();
        exit(0);
    }
}

void kreInstall::debDamageInfo()
{
    QMessageBox *msgBox = CustomBoxUtil::createCritialBox(
                                            "",
                                            QObject::tr("The software package is corrupted and cannot be installed!"),
                                            "",
                                            this);
    msgBox->exec();
}

//批量安裝時架構不符調用
bool kreInstall::ArchIsMatch(QString depath_arch, QString tmp)
{
    QProcess *process = new QProcess();
    process->start("dpkg --print-architecture");
    process->waitForFinished(-1);
    QString arch = process->readLine();
    arch.remove("\n");

    if (arch.compare(depath_arch) && depath_arch.compare("all")) {
        if (arch != ""){
            QMessageBox *msgBox = CustomBoxUtil::createCritialBox("",tr("The software package architecture does not match the native architecture"),"",this);
            msgBox->exec();
            msgBox->setContextMenuPolicy(Qt::NoContextMenu);
            return false;
        }
        else
        {
            QMessageBox *msgBox = CustomBoxUtil::createCritialBox("",tr("The package format is not Debian format and cannot be opened."),"",this);
            msgBox->exec();
            msgBox->setContextMenuPolicy(Qt::NoContextMenu);
            return false;
        }
    }
    return true;
}

void kreInstall::gsettingInit()
{
    const QByteArray style_id(ORG_UKUI_STYLE);
    m_pGsettingFontSize = new QGSettings(style_id);

    connect (m_pGsettingFontSize, &QGSettings::changed, this, [=] (const QString &key) {
        if (key==GSETTING_KEY) {

            m_pInstallSuccessLabel->setFont(getSystemFont(18));
            m_pInstallFailLabel->setFont(getSystemFont(18));
            m_pOsDebInstallIngWidget->debAppNameLabel->setFont(getSystemFont(14));
            m_pOsInstallButton->setFont(getSystemFont(14));
            m_pInstallFaillog->setFont(getSystemFont(14));
            m_pInstallFailsure->setFont(getSystemFont(14));
            if (FilePath == false) {
                Batch_installselect->setFont(getSystemFont(14));
                Batch_installdescribe->setFont(getSystemFont(14));
                Batch_install->setFont(getSystemFont(14));
                Batch_addinstall->setFont(getSystemFont(14));
                Batch_viewlog->setFont(getSystemFont(14));
                Batch_return->setFont(getSystemFont(14));
                for(int i = 0; i< batchInstallWidgetList->count();++i){
                    QListWidgetItem *item1 = batchInstallWidgetList->item(i);
                    QWidget * widget2 = batchInstallWidgetList->itemWidget(item1);
                    if(widget2 !=NULL){
                        QList <QLabel*> labellist = widget2->findChildren<QLabel*>();
                        foreach(QLabel *label,labellist){
                            label->setFont(getSystemFont(14));
                        }
                    }
                }
            }

            /*重设安装到系统的界面数据*/
            m_pOsDebMainWidget->resetAppInfo(m_appName, m_appVersion);
            m_pOsDebInstallIngWidget->resetAppInfo(m_appName, m_appVersion);
            /*重设安装到兼容环境的界面数据*/
            m_pdebWidget->resetAppInfo(m_appName, m_appVersion);
            m_pdebInstallIngWidget->resetAppInfo(m_appName, m_appVersion);
        }

        if (key==ORG_UKUI_STYLE) {
            qDebug() << "llllllllll" <<  key;
        } else {
            if (FilePath == false) {
                QString themeNow = m_pGsettingFontSize->get("styleName").toString();
                if (themeNow == "ukui-dark") {
                    Batch_installIcon->clear();
                    Batch_installIcon->setPixmap(QPixmap(":/image/batch-install-dark.png"));
                } else {
                    Batch_installIcon->setPixmap(QPixmap(":/image/batch-install.png"));
                }
            }
        }
    });
}

void kreInstall::initDebInfo(QString debPath)
{
    QProcess terminal;
    QProcess terminal_1;
    QProcess terminal_2;
    QStringList cmdList;
    QStringList cmdList1;
    QStringList cmdList2;
    QString m_app_path;
    m_debPath = debPath;

    if(FilePath == false){
        m_debPath = debPath;
        return;
    }

    QString debPathname = m_debPath;
    if (debPathname.contains("`",Qt::CaseSensitive) || debPathname.contains("$(",Qt::CaseSensitive) || debPathname.contains(";",Qt::CaseSensitive)
            || debPathname.contains("&",Qt::CaseSensitive) || debPathname.contains("|",Qt::CaseSensitive) ){
        QMessageBox *msgBox = CustomBoxUtil::createCritialBox("",tr("Open file error\n"
                                                                    "Naming contains nonstandard characters '` $( & | '"),"",this);
        msgBox->exec();
        exit(0);
    }
    QFileInfo info(m_debPath);
    QMimeDatabase db;
    mime = db.mimeTypeForFile(m_debPath);
    if(mime.name().contains("vnd.snap",Qt::CaseSensitive))
    {
        cmdList << "-c" << QString("snap info "+ debPath + " |grep name");
        terminal.start("/bin/bash",cmdList);
        terminal.waitForFinished();
        m_appName = terminal.readAll();
        m_appName = m_appName.left(m_appName.length() - 1);
        QStringList list = m_appName.split(":");
        m_appName = list[list.count()-1];
        m_appName =m_appName.simplified();

        cmdList1 << "-c" << QString("snap info "+ debPath + " |grep version");
        terminal.start("/bin/bash",cmdList1);
        terminal.waitForFinished();
        m_appVersion = terminal.readAll();
        m_appVersion = m_appVersion.left(m_appVersion.length() - 2);
        list = m_appVersion.split(":");
        m_appVersion = list[list.count()-1];
        m_appVersion = m_appVersion.simplified();
        m_appVersion = m_appVersion.trimmed();

        m_debArch ="";
        m_debPath = debPath;
        return;
    }
    else if(mime.name().contains("vnd.android.package-archive",Qt::CaseSensitive)){
        onAnalysisApkFile(debPath);
        m_debPath = debPath;
    }
    else
    {
        // 此处需要判断包是否有效，如果无效则不再继续
        Package *package= PackageFactory::createPackage(debPath,false);
        if (!PackageCheckUtils::checkAndLogPackageStatus(package, Q_FUNC_INFO))
        {
            debDamageInfo();
            exit(0);
            return;
        }

        if (!package->packageArchIsMatch()) {
            QMessageBox *msgBox = CustomBoxUtil::createCritialBox("",tr("The software package architecture does not match the native architecture"),"",this);
            msgBox->exec();
            exit(0);
        }

        m_appName = package->m_name;
        m_appVersion = package->m_version;
        m_debArch = package->m_architecture;
        m_app_path = m_appName;
        m_installed_version = package->m_installedVersion;
        if (package->m_status == PackageEnum::Status::INSTALLED)
        {
            m_installed_status == QString("ii");
        }
        else
        {
            m_installed_status = QString();
        }
    }

    return;
}

void kreInstall::mainWidgetInit()
{
    this->setFixedSize(500,600);
    this->setWindowTitle(QObject::tr("Installer"));

    /********************安装前界面********************/
    m_pInstallBeforeWidget = new QWidget();
    m_pallLayout = new QVBoxLayout();
    m_pallLayout->setContentsMargins(0, 0, 0, 0);
    m_pallLayout->setSpacing(0);
    m_pdebWidget = new debInfoWidget(m_appName, m_appVersion, m_debPath);

    m_pruntimeLabel = new QLabel();
    m_pruntimeLabel->setFixedSize(300,31);
    m_pinstToLabel  = new QLabel();
    m_pinstToLabel->setText(tr("install:"));
    m_pinstToLabel->adjustSize();

    m_pruntimeWidget = new QWidget();
    m_pruntimeWidget->setFixedHeight(31);
    m_pinstHLayout = new QHBoxLayout();
    m_pinstHLayout->setContentsMargins(0, 0, 0, 0);
    m_pinstHLayout->setSpacing(0);
    m_pinstHLayout->addItem(new QSpacerItem(83, 10, QSizePolicy::Fixed));
    m_pinstHLayout->addWidget(m_pinstToLabel);
    m_pinstHLayout->addWidget(m_pruntimeLabel);
    m_pinstHLayout->addItem(new QSpacerItem(106, 10, QSizePolicy::Fixed));
    m_pruntimeWidget->setLayout(m_pinstHLayout);



    m_pallLayout->addItem(new QSpacerItem(1, 4, QSizePolicy::Fixed));

    m_pallLayout->addItem(new QSpacerItem(1, 30, QSizePolicy::Expanding));
    m_pallLayout->addWidget(m_pdebWidget);

    m_pallLayout->addWidget(m_pruntimeWidget);

    m_pallLayout->addItem(new QSpacerItem(1, 8, QSizePolicy::Expanding));

    m_pallLayout->addItem(new QSpacerItem(10,12,QSizePolicy::Fixed));
    m_pallLayout->addWidget(m_pblackWidget);
    m_pallLayout->addItem(new QSpacerItem(10,75,QSizePolicy::Fixed));
    m_pallLayout->addWidget(m_pInstallBtnWidget);
    m_pallLayout->addItem(new QSpacerItem(10, 24, QSizePolicy::Fixed));

    m_pInstallBeforeWidget->setLayout(m_pallLayout);

    /*****************安装中界面**********************/
    m_pInstallIngWidget = new QWidget();
    m_pInstallIngLayout = new QVBoxLayout();
    m_pInstallIngLayout->setContentsMargins(0, 0, 0, 0);
    m_pInstallIngLayout->setSpacing(0);

    m_pInstallIngLabelWidget = new QWidget();
    m_pInstallIngLabelWidget->setFixedHeight(20);
    m_pInstallIngLabelLayout = new QHBoxLayout();
    m_pInstallIngLabelLayout->setContentsMargins(0, 0, 0, 0);
    m_pInstallIngLabelLayout->setSpacing(0);
    m_pInstallIngLabel = new QLabel();
    m_pInstallIngLabel->setText(tr("installing!"));

    m_pInstallIngLabelLayout->addWidget(m_pInstallIngLabel);
    m_pInstallIngLabelWidget->setLayout(m_pInstallIngLabelLayout);

    m_pdebInstallIngWidget = new debInfoWidget(m_appName, m_appVersion, m_debPath);

    m_pInstallIngLayout->addWidget(m_pdebInstallIngWidget);
    m_pInstallIngLayout->addSpacing(0);
    m_pInstallIngLayout->addWidget(m_pInstallIngLabelWidget);


    connect (m_ptitleWidget->m_menu, &QMenu::triggered, this, [=](QAction *act) {
         if (act->text() == tr("Help")){
            showGuide();

        } else if (act->text() == tr("About")) {
            QStringList cmdList1;
            QProcess terminal_1;
            cmdList1 << "-c" << "dpkg -l | grep kylin-installer | awk '{print $3}' | sed -n '1p'";
            terminal_1.start("/bin/bash",cmdList1);
            terminal_1.waitForFinished();
            kylin_installerversion = terminal_1.readAll();//获取此包已安装的版本号
            kylin_installerversion = kylin_installerversion.left(kylin_installerversion.length() - 1);
            const QIcon icon = QIcon::fromTheme("kylin-installer");
            QString appname;
            QString version;
            appname = tr("kylin-installer");
            version = tr("version") + "：" +kylin_installerversion;
            const QString appinfo = "";
            kdk::KAboutDialog *userabout = new kdk::KAboutDialog(this,icon,appname,version,appinfo);
            userabout->setWindowModality(Qt::WindowModality::ApplicationModal);
            userabout->show();

        } else if (act->text() == tr("Quit")) {
            exit(0);
        }
    });


    m_pInstallIngWidget->setLayout(m_pInstallIngLayout);

    /*****************安装成功界面********************/
    m_pInstallSuccessWidget = new QWidget();
    m_pInstallSuccessLayout = new QVBoxLayout();
    m_pInstallSuccessLayout->setContentsMargins(0, 0, 0, 0);
    m_pInstallSuccessLayout->setSpacing(0);

    m_pInstallSuccessLabelWidget = new QWidget();
    m_pInstallSuccessLabelLayout = new QVBoxLayout();
    m_pInstallSuccessLabelLayout->setContentsMargins(0, 0, 0, 0);
    m_pInstallSuccessLabelLayout->setSpacing(0);
    m_pInstallSuccessIcon = new QLabel();
    m_pInstallSuccessIcon->setPixmap(QPixmap(":/image/install-success.png"));
    m_pInstallSuccessIcon->setAlignment(Qt::AlignCenter);
    m_pInstallSuccessLabel = new QLabel();
    m_pInstallSuccessLabel->adjustSize();
    m_pInstallSuccessLabel->setText(tr("install success"));

    m_pInstallSuccessLabel->setFont(getSystemFont(18));
    m_pInstallSuccessLabel->adjustSize();
    m_pInstallSuccessLabel->setAlignment(Qt::AlignCenter);

    m_pInstallSuccessButton = new QPushButton;
    m_pInstallSuccessButton->setText(QObject::tr("determine"));
    m_pInstallSuccessButton->adjustSize();
    m_pInstallSuccessButton->setMinimumWidth(160);


    m_pInstallSuccessLabelLayout->addWidget(m_pInstallSuccessIcon,0, Qt::AlignHCenter);
    m_pInstallSuccessLabelLayout->addStretch();
    m_pInstallSuccessLabelLayout->addWidget(m_pInstallSuccessLabel, 0, Qt::AlignHCenter);
    m_pInstallSuccessLabelLayout->addSpacing(80);
    m_pInstallSuccessLabelLayout->addWidget(m_pInstallSuccessButton,0, Qt::AlignHCenter);
    m_pInstallSuccessLabelWidget->setLayout(m_pInstallSuccessLabelLayout);
    connect(m_pInstallSuccessButton,SIGNAL(clicked(bool)),this,SLOT(closeBtnClicked(bool)));
    m_pInstallSuccessLayout->addItem(new QSpacerItem(1, 180, QSizePolicy::Fixed));
    m_pInstallSuccessLayout->addWidget(m_pInstallSuccessLabelWidget);
    m_pInstallSuccessLayout->addItem(new QSpacerItem(1, 157, QSizePolicy::Expanding));
    m_pInstallSuccessWidget->setLayout(m_pInstallSuccessLayout);


    /*****************安装失败界面********************/
    m_pInstallFailWidget = new QWidget();
    m_pInstallFailLayout = new QVBoxLayout();
    m_pInstallFailLayout->setContentsMargins(0, 0, 0, 0);
    m_pInstallFailLayout->setSpacing(0);

    /*安装失败的icon和安装失败字样*/
    m_pInstallFailLabelWidget = new QWidget();
    m_pInstallFailLabelLayout = new QVBoxLayout();
    m_pInstallFailLabelLayout->setContentsMargins(0, 0, 0, 0);
    m_pInstallFailLabelLayout->setSpacing(0);

    /*查看日志按钮 和 确认按钮*/
    m_pInstallFailbuttonWidget = new QWidget();
    m_pInstallFailbuttonWidget->setFixedHeight(48);
    m_pInstallFailbuttonLayout = new QHBoxLayout();
    m_pInstallFailbuttonLayout->setContentsMargins(0, 0, 0, 0);
    m_pInstallFailbuttonLayout->setSpacing(0);

    m_pInstallFailIcon = new QLabel();
    m_pInstallFailIcon->setPixmap(QPixmap(":/image/install-fail.png"));
    m_pInstallFailLabel = new QLabel();
    m_pInstallFailLabel->adjustSize();
    m_pInstallFailLabel->setText(tr("install Fail"));

    m_pInstallFaillog = new QPushButton(this);
    m_pInstallFailsure = new QPushButton(this);
    m_pInstallFaillog->setText(QObject::tr("view log"));
    m_pInstallFaillog->adjustSize();
    m_pInstallFaillog->setFixedSize(160,36);
    m_pInstallFailsure->setText(QObject::tr("determine"));
    m_pInstallFailsure->adjustSize();
    m_pInstallFailsure->setFixedSize(160,36);


    m_pInstallFailLabel->setFont(getSystemFont(18));

    m_pInstallFaillog->setFont(getSystemFont(14));

    m_pInstallFaillog->setFont(getSystemFont(14));


    m_pInstallFailLabelLayout->addWidget(m_pInstallFailIcon,0, Qt::AlignHCenter);
    m_pInstallFailLabelLayout->addWidget(m_pInstallFailLabel,0, Qt::AlignHCenter);
    m_pInstallFailLabelLayout->addSpacing(120);
    m_pInstallFailLabelWidget->setLayout(m_pInstallFailLabelLayout);


    m_pInstallFailbuttonLayout->addSpacing(82);
    m_pInstallFailbuttonLayout->addWidget(m_pInstallFailsure);
    m_pInstallFailbuttonLayout->setSpacing(16);
    m_pInstallFailbuttonLayout->addWidget(m_pInstallFaillog);
    m_pInstallFailbuttonLayout->addSpacing(82);


    m_pInstallFailbuttonWidget->setLayout(m_pInstallFailbuttonLayout);


    connect(m_pInstallFaillog,SIGNAL(clicked(bool)),this,SLOT(showinstallfaillogtext()));
    connect(m_pInstallFailsure,SIGNAL(clicked(bool)),this,SLOT(closeBtnClicked(bool)));

    m_pInstallFailLayout->addSpacing(180);
    m_pInstallFailLayout->addWidget(m_pInstallFailLabelWidget);
    m_pInstallFailLayout->addWidget(m_pInstallFailbuttonWidget);
    m_pInstallFailLayout->addSpacing(148);
    m_pInstallFailWidget->setLayout(m_pInstallFailLayout);


    /***********************查看日志界面**********************/

    m_pInstallLogWidget = new QWidget();
    m_pInstallLOGLabelLayout = new QHBoxLayout();
    m_pInstallLOGLabelLayout->setContentsMargins(0, 0, 0, 0);
    m_pInstallLOGLabelLayout->setSpacing(0);

    m_pInstallLogLabel = new QLabel();
    m_pInstallLogLabel->setText(QObject::tr("Installation log"));
    m_pInstallLogLabel->adjustSize();
    m_pInstallLogLabel->setFont(getSystemFont(14));

    m_pInstallLOGLabelLayout->addItem(new QSpacerItem(30, 30, QSizePolicy::Fixed));
    m_pInstallLOGLabelLayout->addWidget(m_pInstallLogLabel);
    m_pInstallLOGLabelLayout->addItem(new QSpacerItem(1, 20, QSizePolicy::Fixed));



    m_pInstallLogWidget_min = new QWidget();
    m_pInstallLOGwidgetLayout = new QHBoxLayout();
    m_pInstallLOGwidgetLayout->setContentsMargins(0, 0, 0, 0);
    m_pInstallLOGwidgetLayout->setSpacing(0);


    m_pInstallLOGLabelTextLayout = new QHBoxLayout();
    m_pInstallLOGLabelTextLayout->setContentsMargins(0, 0, 0, 0);
    m_pInstallLOGLabelTextLayout->setSpacing(0);

    m_pInstallLogtextedit = new QTextEdit(this);

    m_pInstallLogtextedit->setReadOnly(true);
    m_pInstallLogtextedit->setFrameShape(QFrame::NoFrame);
    m_pInstallLogtextedit->setAutoFillBackground(false);

    m_pInstallLOGLabelTextLayout->addItem(new QSpacerItem(0, 20, QSizePolicy::Fixed));
    m_pInstallLOGLabelTextLayout->addWidget(m_pInstallLogtextedit);
    m_pInstallLOGLabelTextLayout->addItem(new QSpacerItem(0, 20, QSizePolicy::Fixed));

    m_pInstallLogWidget_min->setLayout(m_pInstallLOGLabelTextLayout);
    m_pInstallLogWidget_min->setAutoFillBackground(true);
    m_pInstallLogWidget_min->setBackgroundRole(QPalette::Window);

    m_pInstallLOGwidgetLayout->addItem(new QSpacerItem(30, 0, QSizePolicy::Fixed));
    m_pInstallLOGwidgetLayout->addWidget(m_pInstallLogWidget_min);
    m_pInstallLOGwidgetLayout->addItem(new QSpacerItem(30, 0, QSizePolicy::Fixed));

    m_pInstallLogbuttonlayout = new QHBoxLayout();
    m_pInstallLogbuttonlayout->setContentsMargins(0, 0, 0, 0);
    m_pInstallLogbuttonlayout->setSpacing(0);

    m_pInstallLogbutton = new QPushButton(this);
    m_pInstallLogbutton->setText(QObject::tr("determine"));
    m_pInstallLogbutton->adjustSize();
    m_pInstallLogbutton->setFont(getSystemFont(14));
    m_pInstallLogbutton->setMaximumWidth(180);

    return_homepage_log1 = new QPushButton();
    return_homepage_log1->setText(tr("Return home"));
    return_homepage_log1->adjustSize();
    return_homepage_log1->setFont(getSystemFont(14));
    return_homepage_log1->setMaximumWidth(180);
    return_homepage_log1->hide();

    connect(return_homepage_log1,SIGNAL(clicked(bool)),this,SLOT(return_homepage()));
    m_pInstallLogbuttonlayout->addItem(new QSpacerItem((550 - (m_pInstallFaillog->width()*2))/2+60, 10, QSizePolicy::Fixed));
    m_pInstallLogbuttonlayout->addWidget(return_homepage_log1);
    m_pInstallLogbuttonlayout->addSpacing(20);
    m_pInstallLogbuttonlayout->addWidget(m_pInstallLogbutton);
    m_pInstallLogbuttonlayout->addItem(new QSpacerItem(30, 0, QSizePolicy::Fixed));
    m_fiallogtextlayout = new QVBoxLayout();
    m_fiallogtextlayout->setContentsMargins(0, 0, 0, 0);
    m_fiallogtextlayout->setSpacing(0);

    connect(m_pInstallLogbutton,SIGNAL(clicked(bool)),this,SLOT(batchlist()));


    m_fiallogtextlayout->addItem(new QSpacerItem(0, 30, QSizePolicy::Fixed));
    m_fiallogtextlayout->addLayout(m_pInstallLOGLabelLayout);
    m_fiallogtextlayout->addItem(new QSpacerItem(0, 15, QSizePolicy::Fixed));
    m_fiallogtextlayout->addLayout(m_pInstallLOGwidgetLayout);
    m_fiallogtextlayout->addItem(new QSpacerItem(0, 30, QSizePolicy::Fixed));
    m_fiallogtextlayout->addLayout(m_pInstallLogbuttonlayout);
    m_fiallogtextlayout->addItem(new QSpacerItem(0, 30, QSizePolicy::Fixed));

    m_pInstallLogWidget->setLayout(m_fiallogtextlayout);


    /******************************批量安装的批量界面*****************/
    Batch_installwidget_1 = new QWidget(this);
    batchInstallWidgetList = new QListWidget(this);
    batchInstallWidgetList->setFrameShape(QFrame::NoFrame);
    Batch_installlayout_batch = new QHBoxLayout();

    Batch_install = new QPushButton();
    Batch_addinstall = new QPushButton();
    Batch_viewlog = new QPushButton();
    Batch_return  = new QPushButton();
    Batch_install->setText(tr("Install"));
    Batch_addinstall->setText(tr("Add"));
    Batch_viewlog->setText(tr("View log"));
    Batch_return->setText(tr("Return"));

    Batch_install->setMinimumWidth(160);
    Batch_addinstall->setMinimumWidth(160);
    Batch_viewlog->setMaximumHeight(160);
    Batch_viewlog->hide();
    Batch_return->hide();
    connect(Batch_viewlog,SIGNAL(clicked(bool)),this,SLOT(showinstallfaillogtext()));
    connect(Batch_return,SIGNAL(clicked(bool)),this,SLOT(return_homepage()));
    connect(Batch_addinstall, &QPushButton::clicked, this, &kreInstall::addBtnClicked);

    icontime = new QTimer();
    icontime->setInterval(100);
    connect( icontime,SIGNAL(timeout()), this, SLOT(play_installingpicture()));
    connect(Batch_install, &QPushButton::clicked, this, &kreInstall::Batch_OsInstallButtonSlots);
    Batch_installwidget_1a = new QWidget(this);
    Batch_installlayout_batch = new QHBoxLayout();
    Batch_installlayout_batch->setContentsMargins(24, 0, 24, 0);
    Batch_installlayout_batch->setSpacing(0);
    Batch_installlayout_batch->addWidget(batchInstallWidgetList);
    Batch_installwidget_1a->setLayout(Batch_installlayout_batch);

    Batch_installwidget_1b = new QWidget(this);
    Batch_installlayout_add = new QHBoxLayout();
    Batch_installlayout_add->setContentsMargins(82, 0, 82, 0);
    Batch_installlayout_add->setSpacing(0);
    Batch_installlayout_add->addWidget(Batch_addinstall);
    Batch_installlayout_add->setSpacing(16);
    Batch_installlayout_add->addWidget(Batch_install);
    Batch_installlayout_add->setSpacing(16);
    Batch_installlayout_add->addWidget(Batch_return);
    Batch_installlayout_add->setSpacing(16);
    Batch_installlayout_add->addWidget(Batch_viewlog);

    Batch_installwidget_1b->setLayout(Batch_installlayout_add);

    Batch_installlayout_ALL = new QVBoxLayout();
    Batch_installlayout_ALL->setContentsMargins(0, 0, 0, 0);
    Batch_installlayout_ALL->setSpacing(0);


    Batch_installlayout_ALL->addItem(new QSpacerItem(24, 5, QSizePolicy::Fixed));

    Batch_installlayout_ALL->addWidget(Batch_installwidget_1a);

    Batch_installlayout_ALL->addItem(new QSpacerItem(92, 10, QSizePolicy::Fixed));

    Batch_installlayout_ALL->addWidget(Batch_installwidget_1b);

    Batch_installlayout_ALL->addItem(new QSpacerItem(0, 48, QSizePolicy::Fixed));

    Batch_installwidget_1->setLayout(Batch_installlayout_ALL);

    /*******************批量安装完成后的界面***********/

    Batch_installsuccesswidget = new QWidget(this);

    Batch_installsuccessiconwidget = new QWidget();

    Batch_installsuccesstextwidget = new QWidget();

    Batch_installsuccessdetemin = new QPushButton();
    Batch_installsuccessreturn  = new QPushButton();

    Batch_installsuccessreturn->setText(tr("Return"));
    Batch_installsuccessdetemin->setText(tr("Determine"));
    Batch_installsuccessreturn->setMinimumWidth(160);
    Batch_installsuccessreturn->setMinimumWidth(160);
    connect(Batch_installsuccessreturn,SIGNAL(clicked(bool)),this,SLOT(return_homepage()));
    connect(Batch_installsuccessdetemin,SIGNAL(clicked(bool)),this,SLOT(closeBtnClicked(bool)));

    Batch_installsuccesslayout = new QVBoxLayout();
    Batch_installsuccesslayout->setContentsMargins(0, 0, 0, 0);
    Batch_installsuccesslayout->setSpacing(0);


    QHBoxLayout *Batch_installsuccesslayout1 = new QHBoxLayout();
    Batch_installsuccesslayout1->setContentsMargins(0, 200, 0, 0);
    Batch_installsuccesslayout1->setSpacing(0);

    Batch_installsuccessIcon = new QLabel();
    Batch_installsuccessIcon->setPixmap(QIcon::fromTheme("ukui-dialog-success").pixmap(QSize(64,64)));
    Batch_installsuccessIcon->adjustSize();

    Batch_installsuccesslayout1->addWidget(Batch_installsuccessIcon);
    Batch_installsuccesslayout1->setAlignment(Qt::AlignHCenter);

    QHBoxLayout *Batch_installsuccesslayout2 = new QHBoxLayout();
    Batch_installsuccesslayout2->setContentsMargins(0, 0, 0, 0);
    Batch_installsuccesslayout2->setSpacing(0);

    Batch_installsuccesstext = new QLabel();
    Batch_installsuccesstext->adjustSize();
    Batch_installsuccesstext->setText(tr("Installation is complete"));


    Batch_installsuccesstext->setFont(getSystemFont(14));

    Batch_installsuccesslayout2->addWidget(Batch_installsuccesstext);
    Batch_installsuccesslayout2->setAlignment(Qt::AlignHCenter);


    Batch_installsuccesslayout->addLayout(Batch_installsuccesslayout1);
    Batch_installsuccesslayout->setSpacing(16);
    Batch_installsuccesslayout->addLayout(Batch_installsuccesslayout2);

    Batch_installsuccessiconwidget->setLayout(Batch_installsuccesslayout);

    Batch_installsuccessbuttonlayout = new QHBoxLayout();
    Batch_installsuccessbuttonlayout->setContentsMargins(82, 0, 82, 0);
    Batch_installsuccessbuttonlayout->setSpacing(0);
    Batch_installsuccessbuttonlayout->addWidget(Batch_installsuccessreturn);
    Batch_installsuccessbuttonlayout->setSpacing(16);
    Batch_installsuccessbuttonlayout->addWidget(Batch_installsuccessdetemin);
    Batch_installsuccesstextwidget->setLayout(Batch_installsuccessbuttonlayout);

    Batch_installsuccesslayout_ALL = new QVBoxLayout();
    Batch_installsuccesslayout_ALL->setContentsMargins(0, 0, 0, 0);
    Batch_installsuccesslayout_ALL->setSpacing(0);

    Batch_installsuccesslayout_ALL->addWidget(Batch_installsuccessiconwidget);
    Batch_installsuccesslayout_ALL->addWidget(Batch_installsuccesstextwidget);

    Batch_installsuccesswidget->setLayout(Batch_installsuccesslayout_ALL);



    /*QStackedWidget将所有界面排序起来。用 m_pAllStackedWidget->setCurrentIndex(?)去显示，首页从0开始*/
    m_pAllStackedWidget = new QStackedWidget();
    m_pAllStackedWidget->addWidget(m_pInstallBeforeWidget);
    m_pAllStackedWidget->addWidget(m_pInstallIngWidget);
    m_pAllStackedWidget->addWidget(m_pInstallSuccessWidget);
    m_pAllStackedWidget->addWidget(m_pInstallFailWidget);
    m_pAllStackedWidget->addWidget(m_pOsInstallWidget);
    m_pAllStackedWidget->addWidget(m_pOsInstallIngWidget);
    m_pAllStackedWidget->addWidget(m_pInstallLogWidget);
    m_pAllStackedWidget->addWidget(Batch_installwidget_1);
    m_pAllStackedWidget->addWidget(Batch_installsuccesswidget);

    m_pAllStackedLayout = new QVBoxLayout();
    m_pAllStackedLayout->setContentsMargins(0, 0, 0, 0);
    m_pAllStackedLayout->setSpacing(0);
    m_pAllStackedLayout->addSpacing(3);
    m_pAllStackedLayout->addWidget(m_ptitleWidget);

    m_pAllStackedLayout->addSpacing(10);

    m_pAllStackedLayout->addWidget(m_pAllStackedWidget);
    this->setLayout(m_pAllStackedLayout);
}

/* 初始化安装到系统的安装器界面 */
void kreInstall::initOsMainWidget()
{
    m_pOsInstallWidget = new QWidget();
    m_pOsInstallWidget->setContentsMargins(0, 0, 0, 0);

    m_pOsDebMainWidget = new debInfoWidget(m_appName, m_appVersion, m_debPath);
    m_pOsDebMainWidget->debAppNameLabel->setFont(getSystemFont(18));

    m_pOsVboxLayoutWidget = new QVBoxLayout();
    m_pOsVboxLayoutWidget->setContentsMargins(0, 0, 0, 0);
    m_pOsVboxLayoutWidget->setSpacing(0);

    m_pOsInstallButton = new QPushButton();
    m_pOsInstallButton->setText(QObject::tr("one-click install"));
    m_pOsInstallButton->setMaximumHeight(36);
    m_pOsInstallButton->setMinimumSize(130,36);

    connect(m_pOsInstallButton, &QPushButton::clicked, this, &kreInstall::OsInstallButtonSlots);

    return;
}

/*处理QT中纯英文在QLable中的换行问题*/
QString kreInstall::SpliteText(QFontMetrics font, QString text,int nlablesize)
{
    int nTextSize = font.width(text);
    if(nTextSize > nlablesize){
        int nPos =0;
        long nOffset =0;
        for (int i =0;i < text.size();i++){
            nOffset +=font.width(text.at(i));
            if(nOffset >= nlablesize){
                nPos = i;
                break;
            }
        }
        nPos = (nPos -1< 0)? 0:nPos -1;
        QString qstrLeftData =text.left(nPos);
        QString qstrMidData = text.mid(nPos);
        return qstrLeftData + "\n" +SpliteText(font,qstrMidData,nlablesize);
    }

    return text;

}

/* 初始化布局安装到系统的安装器界面 */
void kreInstall::initOsLayoutWidget()
{
    if (FilePath == false) {
        /******************************批量安装的初始界面*****************/
        Batch_installlayout = new QVBoxLayout();
        Batch_installlayout->setContentsMargins(0, 0, 0, 0);
        Batch_installlayout->setSpacing(0);
        Batch_installIcon = new QLabel();
        //点击"添加"按钮选择软件包进行安装
        QString themeNow = m_pGsettingFontSize->get("styleName").toString();
        if (themeNow == "ukui-dark") {
            Batch_installIcon->clear();
            Batch_installIcon->setPixmap(QPixmap(":/image/batch-install-dark.png"));
        } else {
            Batch_installIcon->setPixmap(QPixmap(":/image/batch-install.png"));
        }
        Batch_installdescribe = new QLabel();
        QString info = tr("Click the \"add\" button to select the");
        QString info1 = tr("software package for installation");
        QLocale locale;
        if (locale.language() == QLocale::Chinese)
            Batch_installdescribe->setText(info +info1);
        else
            Batch_installdescribe->setText(info +"\n"+ info1);

        Batch_installdescribe->setWordWrap(true);
        Batch_installdescribe->setAlignment(Qt::AlignCenter);
        Batch_installdescribe->adjustSize();
        Batch_installselect = new QPushButton();
        Batch_installselect->setText(QObject::tr("Add"));
        Batch_installselect->adjustSize();
        Batch_installselect->setFont(getSystemFont(14));
        Batch_installdescribe->setFont(getSystemFont(14));
        Batch_installselect->setMinimumWidth(180);
        Batch_installselect->setMinimumHeight(36);

        QHBoxLayout *ButtonHBoxLayout = new QHBoxLayout();
        ButtonHBoxLayout->setContentsMargins(0, 0, 0, 0);
        ButtonHBoxLayout->setSpacing(0);

        ButtonHBoxLayout->addWidget(Batch_installIcon, 0, Qt::AlignHCenter);

        QHBoxLayout *ButtonlabelLayout = new QHBoxLayout();
        ButtonlabelLayout->setContentsMargins(0, 0, 0, 0);
        ButtonlabelLayout->setSpacing(0);
        ButtonlabelLayout->addWidget(Batch_installdescribe, 0, Qt::AlignHCenter);

        QHBoxLayout *ButtonlabelLayout_select = new QHBoxLayout();
        ButtonlabelLayout_select->setContentsMargins(0, 0, 0, 0);
        ButtonlabelLayout_select->setSpacing(0);
        ButtonlabelLayout_select->addWidget(Batch_installselect, 0, Qt::AlignHCenter);

        Batch_installlayout = new QVBoxLayout();
        Batch_installlayout->setContentsMargins(0, 0, 0, 0);
        Batch_installlayout->setSpacing(0);

        Batch_installlayout->addStretch();
        Batch_installlayout->addWidget(Batch_installIcon,0, Qt::AlignHCenter);
        Batch_installlayout->addSpacing(10);
        Batch_installlayout->addWidget(Batch_installdescribe);
        Batch_installlayout->addSpacing(40);
        Batch_installlayout->addWidget(Batch_installselect,0, Qt::AlignHCenter);
        Batch_installlayout->addStretch();

        m_pOsInstallWidget->setLayout(Batch_installlayout);
        connect(Batch_installselect, &QPushButton::clicked, this, &kreInstall::addBtnClicked);
    
    } else {
        // deb信息界面布局
        QWidget *debWidget = new QWidget();
        debWidget->setContentsMargins(0, 0, 0, 0);

        QHBoxLayout *debHboxLayout = new QHBoxLayout();
        debHboxLayout->setContentsMargins(0, 0, 0, 0);
        debHboxLayout->setSpacing(0);
        debHboxLayout->addItem(new QSpacerItem(0, 10, QSizePolicy::Fixed));
        debHboxLayout->addWidget(m_pOsDebMainWidget);
        debHboxLayout->setSpacing(0);
        debWidget->setLayout(debHboxLayout);

        // 安装按钮布局
        QWidget *buttonWidget = new QWidget();
        buttonWidget->setContentsMargins(0, 0, 0, 0);
        QVBoxLayout *ButtonHBoxLayout = new QVBoxLayout();
        ButtonHBoxLayout->setContentsMargins(0, 0, 0, 0);
        ButtonHBoxLayout->setSpacing(0);
        ButtonHBoxLayout->addWidget(m_pOsInstallButton);
        ButtonHBoxLayout->setAlignment(Qt::AlignHCenter);
        ButtonHBoxLayout->addSpacing(110);
        buttonWidget->setLayout(ButtonHBoxLayout);

        m_pOsVboxLayoutWidget->addWidget(debWidget);
        m_pOsVboxLayoutWidget->addWidget(buttonWidget);
        m_pOsInstallWidget->setLayout(m_pOsVboxLayoutWidget);
    }

    return;
}

/* 初始化变量安装中变量 */
void kreInstall::initOsInstallIngWidget()
{
    m_pOsInstallIngWidget = new QWidget();
    m_pOsInstallIngWidget->setContentsMargins(0, 0, 0, 0);

    m_pOsDebInstallIngWidget = new debInfoWidget(m_appName, m_appVersion, m_debPath);
    m_pOsDebInstallIngWidget->debAppNameLabel->setFont(getSystemFont(18));

    m_pOsVboxLayoutInstallIngWidget = new QVBoxLayout();
    m_pOsVboxLayoutInstallIngWidget->setContentsMargins(0, 0, 0, 0);
    m_pOsVboxLayoutInstallIngWidget->setSpacing(0);

    m_pOsInstallIngLabel = new QLabel();
    m_pOsInstallIngLabel->setText(QObject::tr("In the installation..."));

    pProgressBar = new QProgressBar();
    pProgressBar->setOrientation(Qt::Horizontal);  // 水平方向
    pProgressBar->setMinimum(0);  // 最小值
    pProgressBar->setMaximum(100);  // 最大值
    pProgressBar->setValue(0);  // 当前进度
    pProgressBar->setFixedWidth(240);
    pProgressBar->setFixedHeight(6);
    pProgressBar->setTextVisible(false);

    progress = new QLabel();
    progress->setText("0%");
    progress->adjustSize();
    m_pKareInstallEnvStatusLabel = new QLabel();
    m_pKareInstallEnvStatusLabel->setText(tr("The compatibility environment is being initialized and waiting for installation ..."));
    m_pKareInstallEnvStatusLabel->adjustSize();
    m_pKareInstallEnvCancelPushButton = new KBorderlessButton();
    m_pKareInstallEnvCancelPushButton->setFixedSize(QSize(160,20));
    m_pKareInstallEnvCancelPushButton->setText(QObject::tr("Cancel installation"));
    m_pKareInstallEnvCancelPushButton->setFlat(true);
    return;
}

/* 初始化变量安装中布局 */
void kreInstall::initOsLayoutIngWidget()
{
    // deb信息界面布局
    QWidget *debIngWidget = new QWidget();
    debIngWidget->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout *debHboxLayout = new QHBoxLayout();
    debHboxLayout->setContentsMargins(0, 0, 0, 0);
    debHboxLayout->setSpacing(0);
    debHboxLayout->addItem(new QSpacerItem(0, 10, QSizePolicy::Fixed));
    debHboxLayout->addWidget(m_pOsDebInstallIngWidget);
    debHboxLayout->setAlignment(Qt::AlignCenter);
    debHboxLayout->setSpacing(0);
    debIngWidget->setLayout(debHboxLayout);

    // 安装按钮布局
    m_pPackageInstallLabelWidget = new QWidget();
    m_pPackageInstallLabelWidget->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *LabelHBoxLayout = new QVBoxLayout();
    LabelHBoxLayout->setContentsMargins(0, 0, 0, 0);
    LabelHBoxLayout->setSpacing(0);

    QVBoxLayout *allVBoxLayout = new QVBoxLayout();
    allVBoxLayout->setContentsMargins(0, 0, 0, 0);
    allVBoxLayout->setSpacing(0);
    allVBoxLayout->setAlignment(Qt::AlignCenter);

    LabelHBoxLayout->addWidget(m_pOsInstallIngLabel);
    LabelHBoxLayout->setAlignment(Qt::AlignHCenter);

    QHBoxLayout *qprogressbarHBoxLayout = new QHBoxLayout();
    qprogressbarHBoxLayout->setContentsMargins(0, 0, 0, 0);
    qprogressbarHBoxLayout->setSpacing(0);
    qprogressbarHBoxLayout->setAlignment(Qt::AlignCenter);
    qprogressbarHBoxLayout->addSpacing(110);
    qprogressbarHBoxLayout->addWidget(pProgressBar);
    qprogressbarHBoxLayout->addSpacing(25);
    qprogressbarHBoxLayout->addWidget(progress);
    qprogressbarHBoxLayout->addStretch();

    allVBoxLayout->setAlignment(Qt::AlignCenter);
    allVBoxLayout->addLayout(LabelHBoxLayout);
    allVBoxLayout->addSpacing(15);
    allVBoxLayout->addLayout(qprogressbarHBoxLayout);
    allVBoxLayout->addSpacing(145);
    m_pPackageInstallLabelWidget->setLayout(allVBoxLayout);

    // kareinstall环境初始化状态
    m_pKareInstallEnvLabelWidget = new QWidget();
    m_pKareInstallEnvLabelWidget->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *kareLabelHBoxLayout = new QVBoxLayout();
    kareLabelHBoxLayout->setContentsMargins(0, 0, 0, 0);
    kareLabelHBoxLayout->setSpacing(0);

    QVBoxLayout *kareAllVBoxLayout = new QVBoxLayout();
    kareAllVBoxLayout->setContentsMargins(0, 0, 0, 0);
    kareAllVBoxLayout->setSpacing(0);
    kareAllVBoxLayout->setAlignment(Qt::AlignCenter);

    kareLabelHBoxLayout->addWidget(m_pKareInstallEnvStatusLabel);
    kareLabelHBoxLayout->setAlignment(Qt::AlignHCenter);

    QHBoxLayout *kareHBoxLayout = new QHBoxLayout();
    kareHBoxLayout->setContentsMargins(0, 0, 0, 0);
    kareHBoxLayout->setSpacing(0);
    kareHBoxLayout->setAlignment(Qt::AlignCenter);
    kareHBoxLayout->addWidget(m_pKareInstallEnvCancelPushButton);

    kareAllVBoxLayout->setAlignment(Qt::AlignCenter);
    kareAllVBoxLayout->addLayout(kareLabelHBoxLayout);
    kareAllVBoxLayout->addSpacing(24);
    kareAllVBoxLayout->addLayout(kareHBoxLayout);
    kareAllVBoxLayout->addSpacing(145);
    m_pKareInstallEnvLabelWidget->setLayout(kareAllVBoxLayout);
    m_pKareInstallEnvLabelWidget->hide();


    m_pOsVboxLayoutInstallIngWidget->addWidget(debIngWidget);
    m_pOsVboxLayoutInstallIngWidget->addWidget(m_pPackageInstallLabelWidget);
    m_pOsVboxLayoutInstallIngWidget->addWidget(m_pKareInstallEnvLabelWidget);
    m_pOsInstallIngWidget->setLayout(m_pOsVboxLayoutInstallIngWidget);

    return;
}

void kreInstall::initInstallBtnWidget()
{
    m_pblackWidget = new QWidget();
    m_pblackWidget->setContentsMargins(0, 0, 0, 0);
    m_pblackWidget->setFixedHeight(100);

    m_pInstallBtnWidget = new QWidget();
    m_pInstallBtnWidget->setFixedHeight(36);
    m_pInstallBtnLayout = new QHBoxLayout();
    m_pInstallBtnLayout->setContentsMargins(0, 0, 0, 0);
    m_pInstallBtnLayout->setSpacing(0);

    m_pInstallBtn = new QPushButton();
    m_pInstallBtn->setFixedSize(100,36);
    m_pInstallBtn->setText(tr("install"));
    connect(m_pInstallBtn,SIGNAL(clicked(bool)),this,SLOT(installBtnClicked()));

    m_pInstallBtnLayout->addItem(new QSpacerItem(426,10,QSizePolicy::Expanding));
    m_pInstallBtnLayout->addWidget(m_pInstallBtn);
    m_pInstallBtnLayout->addItem(new QSpacerItem(24, 10, QSizePolicy::Fixed));
    m_pInstallBtnWidget->setLayout(m_pInstallBtnLayout);
}

void kreInstall::getOsVersionValue()
{
    QFile file(OS_VERSION_FILE_PATH);
    QString line;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);  //用文件构造流
        line = in.readLine();//读取一行放到字符串里
        while (!line.isNull()) {
            line = in.readLine();//循环读取下行
            if (line.contains(OS_VERSION_DESCRIPTION)) {
                break;
            }
        }
    }
    file.close();

    QStringList lineStringList = line.split("=");
    QString OsVersionLogo = lineStringList.at(1);
    if (OsVersionLogo == OS_V10_PRO_VERSION) {
        m_OsVersionLogo = OS_V10_PRO_LOGO;
    } else if (OsVersionLogo == OS_V10_VERSION) {
        m_OsVersionLogo = OS_V10_LOGO;
    }
    return;
}

QFont kreInstall::getSystemFont(int size)
{
    double fontsize = (m_pGsettingFontSize->get(GSETTING_FONT_KEY).toFloat());

    double lableBaseFontSize = size * 9/12;//魔鬼数字，自行处理
    double nowFontSize = lableBaseFontSize * double(fontsize) / 11;//11为系统默认大小，魔鬼数字，自行处理
    QFont font;
    if (size == 18){
        font.setFamily("Noto Sans CJK SC");
        font.setWeight(500);

    }
    font.setBold(false);
    font.setPointSizeF(nowFontSize);


    return font;
}

/*批量安装进度返回显示*/
void kreInstall::batchInstalldingStatus(int status, QString appinstall_name)
{
    disconnect(m_pKylinInstalldbusCallThread.get(), &KylinInstalldbusCallThread::kylinbatchOsinstalldingStatus, \
               this, &kreInstall::batchInstalldingStatus);

    // 此处不判断是否为空，因为已经在调用之前判断过了
    Package *package= PackageFactory::createPackage(appinstall_name);
    PackageCheckUtils::checkAndLogPackageStatus(package, Q_FUNC_INFO);
    QString appname_test = package->m_name;
    QString zh_nameapp =  package->m_zhName;
    for(int i = 0; i< batchInstallWidgetList->count();++i){
        QListWidgetItem *item1 = batchInstallWidgetList->item(i);
        QWidget * widget2 = batchInstallWidgetList->itemWidget(item1);
        if(widget2 !=NULL){
            QList <QLabel*> labellist = widget2->findChildren<QLabel*>();
            foreach(QLabel *label,labellist){
                if(label->text().toLower() == appname_test.toLower() || (label->text() == zh_nameapp && zh_nameapp!="")){
                    QList <QProgressBar*> qprolist = widget2->findChildren<QProgressBar*>();
                    foreach(QProgressBar *progress,qprolist)
                        progress->setValue(status);
                    foreach(QLabel *label1,labellist){
                        if (label1->objectName() == "progress")
                            label1->setText(QString::number(status) + "%");
                    }
                }
            }
        }
    }
    connect(m_pKylinInstalldbusCallThread.get(), &KylinInstalldbusCallThread::kylinbatchOsinstalldingStatus, \
            this, &kreInstall::batchInstalldingStatus);
}

/*单个安装进度返回显示*/
void kreInstall::InstalldingStatus(int status)
{
    disconnect(m_pKylinInstalldbusCallThread.get(), &KylinInstalldbusCallThread::kylinOsinstalldingStatus, \
               this, &kreInstall::InstalldingStatus);
    pProgressBar->setValue(status);
    progress->setText(QString::number(status) + "%");

    connect(m_pKylinInstalldbusCallThread.get(), &KylinInstalldbusCallThread::kylinOsinstalldingStatus, \
            this, &kreInstall::InstalldingStatus);
}

QString kreInstall::getDebInfoFromDeb(QString m_debPath, QString type)
{
    QStorageInfo SD;
    SD.setPath("/");
    SD.refresh();
    int disk_space = SD.bytesAvailable()/1024/1024;
    if (disk_space <=10)
    {
        QMessageBox *msgBox = CustomBoxUtil::createCritialBox("",tr("Insufficient disk space!"),"",this);
        msgBox->exec();
        msgBox->setContextMenuPolicy(Qt::NoContextMenu);
        exit(0);
    }
    QProcess terminal;
    QStringList cmdList;
    m_debPath = QDir::toNativeSeparators(m_debPath);


    QApt::DebFile debfile(m_debPath);
    if (debfile.isValid()){
        if (type == "Package"){
            return debfile.packageName();}
        else if (type == "Version"){
            return debfile.version();
        }
        else{
            return debfile.architecture();
        }
    }
    else{
        return "The software package is damaged";
    }

    QRegExp rx("[^a-zA-Z0-9]");
    int pos = rx.indexIn(m_debPath);
    if (pos > -1)
        m_debPath.toHtmlEscaped();

    QFileInfo file(m_debPath);

    cmdList << "-c" << QString("dpkg -I " + m_debPath);
    terminal.start("/bin/bash",cmdList);
    terminal.waitForFinished();

    QString debInfo = terminal.readAll();

    QString debinfoerro = terminal.readAllStandardError().data();
    if ((debInfo.contains(type) || debInfo.contains(type.toLower())) && debinfoerro=="" ){
        QString ret;
        QStringList list = debInfo.split("\n");
        foreach (QString str, list) {
            if(type == "Package"){
                if (str.left(9) == " Package:"){
                    ret = str.remove(" Package:").remove("\n").remove(" ");
                    break;
                }
            } else if(type == "Version"){
                if (str.left(9) == " Version:"){
                    ret = str.remove(" Version:").remove("\n").remove(" ");
                    break;
                }
            } else if(type == "Architecture"){
                if (str.left(14) == " Architecture:"){
                    ret = str.remove(" Architecture:").remove("\n").remove(" ");
                    break;
                }
            }
        }
        if ( debInfo!="" && debInfo.contains(type)){
            qInfo() << ret << "=======================";
           return ret;
        }
    }
    else if (debInfo!="" && debinfoerro.contains("压缩数据已损坏")|| debinfoerro.contains("གནོན་སྐྱུང་གཞི་གྲངས་ལ་སྐྱོན་ཤོར་བ།"))
    {
        //deb包已损坏，无法解析
        return "The software package is damaged";
    }
    else if (file.exists() && debInfo=="")
    {
        //路径包含特殊字符，需要debfile解析
        //QApt::DebFile::isValid();
        QApt::DebFile debfile(m_debPath);
        if (debfile.isValid()){
            if (type == "Package"){
                return debfile.packageName();}
            else if (type == "Version"){
                return debfile.version();
            }
            else{
                return debfile.architecture();
            }
        }
        else{
            return "The software package is damaged";
        }
    }
}

void kreInstall::installBtnClicked()
{
    QVariantList jsonInfo;
    m_runtime  = m_pruntimeLabel->text();


    jsonInfo<<QVariant::fromValue(m_runtime);
    jsonInfo<<QVariant::fromValue(m_appName);
    jsonInfo<<QVariant::fromValue(m_permissionList);
    if (!m_runtime.isEmpty()) {

        m_pDbusCall = new DbusCallThread(jsonInfo, m_runtime, m_appName, m_debPath);
        m_pDbusCall->start();
        connect(m_pDbusCall, SIGNAL(installFail()), this, SLOT(installFailSlot()), Qt::QueuedConnection);
        connect(m_pDbusCall, SIGNAL(installSuccess()), this, SLOT(installSuccessSlot()), Qt::QueuedConnection);

    } else {
        exit(0);
    }
}

/*查看日志函数*/
void kreInstall::showinstallfaillogtext()
{
    return_homepage_log1->show();
    m_pAllStackedWidget->setCurrentIndex(6);

    QWidget *page = m_pAllStackedWidget->widget(6);
    QList<QWidget *> widgets = page->findChildren<QWidget *>();
    for (QWidget *w : widgets)
        w->repaint();
}

/*返回主页*****/
void kreInstall::return_homepage()
{
    pkgListAdded->clear();
    pkgMd5ListAdded->clear();
    m_pInstallLogtextedit->clear();
    maplist.clear();
    int count = batchInstallWidgetList->count();
    Batch_addinstall->show();
    Batch_return->hide();
    Batch_install->show();
    Batch_viewlog->hide();
    return_homepage_log1->hide();
    Batch_install->setEnabled(true);
    appinstallstatus = true;
    for(int i = 0; i< count;i++){
        QListWidgetItem *item1 = batchInstallWidgetList->takeItem(0);
        batchInstallWidgetList->removeItemWidget(item1);
        delete item1;
    }
    m_pAllStackedWidget->setCurrentIndex(4);

}

// 麒麟安装器单个安装完成信号槽
void kreInstall::osInstallOverSoftwareAptSlots(bool status, QString msg)
{
    kreInStallUnlock();

    disconnect(m_pKylinInstalldbusCallThread.get(), &KylinInstalldbusCallThread::kylinOsinstallStatus, \
               this, &kreInstall::osInstallOverSoftwareAptSlots);

    if (mime.name().contains("vnd.snap",Qt::CaseSensitive)) {
        if (status == true) {
            playSoundEffects();
            m_pAllStackedWidget->setCurrentIndex(2);
        } else {
            m_pInstallLogtextedit->setText("\n"+msg);
            m_pAllStackedWidget->setCurrentIndex(3);
            if(msg.contains("架构不符",Qt::CaseSensitive))
                m_pAllStackedWidget->setCurrentIndex(3);
        }

    } else if (mime.name().contains("vnd.android.package-archive",Qt::CaseSensitive)) {
        if (status == true) {
            playSoundEffects();
            m_pAllStackedWidget->setCurrentIndex(2);
        } else {
            m_pInstallLogtextedit->setText("\n"+msg);
            m_pAllStackedWidget->setCurrentIndex(3);
        }

    } else {
        if (status) {
            pProgressBar->setValue(0);
            progress->setText("0%");
            playSoundEffects();
            m_pAllStackedWidget->setCurrentIndex(2);

        } else {
            pProgressBar->setValue(0);
            progress->setText("0%");
            m_pInstallLogtextedit->setText("\n"+msg);
            m_pAllStackedWidget->setCurrentIndex(3);
        }
    }

    install_status = false;
    return;
}

/***********批量安装完成信号槽函数********/
void kreInstall::Batch_osInstallOverSoftwareAptSlots(bool status, QString msg)
{
    icontime->stop();
    QString deskstr = pkgListAdded->length()>0 ? pkgListAdded->at(0) : QString();
    disconnect(m_pKylinInstalldbusCallThread.get(), &KylinInstalldbusCallThread::kylinOsinstallStatus, \
               this, &kreInstall::Batch_osInstallOverSoftwareAptSlots);
    qDebug() << "app: "<<deskstr << " status:"<<status;
    if (deskstr.isEmpty()) return;
    if (status) {
        changebuttonstatus(deskstr,0);
        if (pkgListAdded->length()>0){
            flagstring = pkgListAdded->at(0);
            thread_install_deb();
            automatic_deletewidgetitem();
        } else {
            kreInStallUnlock();
        }
    } else {
        Package *package = PackageFactory::createPackage(deskstr);
        PackageCheckUtils::checkAndLogPackageStatus(package, Q_FUNC_INFO);
        QString appname_test = package->m_name;
        QString zh_nameapp =  package->m_zhName;
        QString info = "";
        if (zh_nameapp.isEmpty())
            info = appname_test + ":";
        else
            info = zh_nameapp + ":";
        m_pInstallLogtextedit->insertPlainText("\n"+info+"\n"+msg);
        appinstallstatus = false;
        changebuttonstatus(pkgListAdded->at(0),1);
        if (pkgListAdded->length()>0)
            thread_install_deb();
        else
            kreInStallUnlock();
    }

    if (pkgListAdded->size() == 0 && batchInstallWidgetList->count()>0 && appinstallstatus ==false) {
        Batch_install->hide();
        Batch_viewlog->show();
        Batch_addinstall->hide();
        Batch_return->show();
    }

    install_status = false;
    return;
}

/*********线程批量安装*********/
void kreInstall::thread_install_deb(){
    pkgListAdded->removeFirst();
    if (pkgListAdded->size()>0){
        app_installed_fast();
        if(pkgListAdded->size()>0){
            Batch_setInstallThread(pkgListAdded->at(0));
        }
    }
}

/*********确定相同版本安装的后续操作函数*******/
void kreInstall::app_installed_fast()
{
    bool app_version_status;
    int lsitcount;
    lsitcount = pkgListAdded->size();
    install_flag = 0;
    Package *package= PackageFactory::createPackage(pkgListAdded->at(0));
    PackageCheckUtils::checkAndLogPackageStatus(package, Q_FUNC_INFO);
    QString appname_test = package->m_name;
    QString zh_nameapp =  package->m_zhName;
    for(int i = 0; i< lsitcount && pkgListAdded->size()>0;i++){
        Package *package= PackageFactory::createPackage(pkgListAdded->at(0));
        PackageCheckUtils::checkAndLogPackageStatus(package, Q_FUNC_INFO);
        QString appname_test = package->m_name;
        QString zh_nameapp =  package->m_zhName;
        app_version_status=app_installed(pkgListAdded->at(0));
        if (app_version_status){
            QMessageBox msgBox(this);
            msgBox.setWindowFlags(Qt::Drawer);
            if (zh_nameapp != nullptr){
                QString lang = getenv("LANG");
                if (lang.contains("zh_CN")){
                    QString info = tr("The current version of the software is already installed on the system");
                    msgBox.setText('\"'+zh_nameapp+'\"'+" "+info);
                }
                else{
                    QString info = tr("The current version of the software is already installed on the system");
                    msgBox.setText('\"'+appname_test+'\"'+" "+info);
                }
            }
            else{
                QString info = tr("The current version of the software is already installed on the system");
                msgBox.setText('\"'+appname_test+'\"'+" "+info);
            }
            CACLINFO = false;
            QPushButton *btnno = new QPushButton();
            btnno->setText(tr("Cancel installation"));
            msgBox.addButton(btnno, QMessageBox::ActionRole);
            QPushButton *btnyes = new QPushButton();
            btnyes->setText(tr("continue installation"));
            msgBox.addButton(btnyes, QMessageBox::RejectRole);
            connect(btnno,SIGNAL(clicked(bool)),this,SLOT(cancel_installed()));
            connect(btnyes,SIGNAL(clicked(bool)),this,SLOT(continue_install()));
            msgBox.exec();
            if(install_flag ==1)
                break;
            else
                if (CACLINFO == false)
                    cancel_installed();
        }

    }

    for(int i = 0; i< batchInstallWidgetList->count();++i){
        QListWidgetItem *item1 = batchInstallWidgetList->item(i);
        QWidget * widget2 = batchInstallWidgetList->itemWidget(item1);
        QList <QLabel*> labellist = widget2->findChildren<QLabel*>();
        if(widget2 !=NULL){
            foreach(QLabel *label,labellist){
                if(label->text().toLower() == appname_test.toLower() || (label->text() == zh_nameapp && zh_nameapp!="")){
                    foreach(QLabel *label1,labellist){
                        if (label1->text() == "Wait"){
                            label1->show();

                        }
                    }
                }
                else{
                    if (label->text() == "Wait")
                        label->show();
                }

            }

        }
    }
}

/*******判断软件的当前版本是否已安装*******/
bool kreInstall::app_installed(QString debpathstring)
{
    Package *package = PackageFactory::createPackage(debpathstring,false);
    PackageCheckUtils::checkAndLogPackageStatus(package, Q_FUNC_INFO);

    if (package != nullptr && package->m_status == PackageEnum::Status::INSTALLED)
        return true;

    return false;
}

/********批量安装取消继续安装当前软件*****/
void kreInstall::cancel_installed()
{
    CACLINFO = true;
    if (FilePath == false) {
        Package *package = PackageFactory::createPackage(pkgListAdded->at(0));
        PackageCheckUtils::checkAndLogPackageStatus(package, Q_FUNC_INFO);

        QString appname_test = package->m_name;
        QString zh_nameapp   = package->m_zhName;

        for (int i = 0; i < batchInstallWidgetList->count(); i++) {
            QListWidgetItem *item1  = batchInstallWidgetList->item(i);
            QWidget * widget2       = batchInstallWidgetList->itemWidget(item1);

            if (widget2 != NULL) {
                QList <QLabel*> labellist = widget2->findChildren<QLabel*>();
                foreach (QLabel *label, labellist) {
                    if (label->text().toLower() == appname_test.toLower() || (label->text() == zh_nameapp && zh_nameapp!="")) {
                        QListWidgetItem *item3 = batchInstallWidgetList->takeItem(i);
                        batchInstallWidgetList->removeItemWidget(item1);
                        delete item3;
                        break;
                    }
                }
            }
        }

        QString md5sum = package->m_md5Sum;
        for (int i = 0; i < pkgMd5ListAdded->count(); i++) {
            if (pkgMd5ListAdded->at(i) == md5sum)
                pkgMd5ListAdded->removeAt(i);
        }

        pkgListAdded->removeFirst();
        if (batchInstallWidgetList->count() == 0)
            return_homepage();

    } else
        CANSCELISNTALL = true;
}

/*********选择确定的时候******/
void kreInstall::continue_install(){
    install_flag = 1;
    CANSCELISNTALL = false;
}

/*****添加包名相同，版本不同的包时判断*****/
void kreInstall::canseaddapplication(){
    canceladd = 1;
}

/******替换文件函数*******/
void kreInstall::replace_application()
{
    Package *package = PackageFactory::createPackage(path_deb);
    PackageCheckUtils::checkAndLogPackageStatus(package, Q_FUNC_INFO);
    QString appname_test = package->m_name;
    QString zh_nameapp =  package->m_zhName;
    for(int i = 0; i< batchInstallWidgetList->count();i++){
        QListWidgetItem *item1 = batchInstallWidgetList->item(i);
        QWidget * widget2 = batchInstallWidgetList->itemWidget(item1);
        if(widget2 !=NULL){
            QList <QLabel*> labellist = widget2->findChildren<QLabel*>();
            foreach(QLabel *label,labellist){
                if(label->text().toLower() == appname_test.toLower() || (label->text() == zh_nameapp && zh_nameapp!="")){
                    QListWidgetItem *item3 = batchInstallWidgetList->takeItem(i);
                    batchInstallWidgetList->removeItemWidget(item1);
                    delete item3;
                    pkgListAdded->removeAt(i);
                    break;
                }

            }

        }
    }

}

void kreInstall::secondaryInstallation(QString debPath)
{
    QStringList importFiles;

    qDebug() << "kreInstall::secondaryInstallation("<<debPath<<")";

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(debPath.toUtf8(), &err);
    if (err.error == QJsonParseError::NoError && doc.isObject()) {
        QJsonArray arr = doc.object().value("importFiles").toArray();

        for (auto v : arr) importFiles << v.toString();
        qDebug() << "Received import package list:" << importFiles;
    } else {
        importFiles.append(debPath);
        qDebug() << "Received non JSON message:" << debPath;
    }

    qDebug() << "m_pAllStackedWidget->currentIndex():"<<m_pAllStackedWidget->currentIndex();

    if (m_pAllStackedWidget->currentIndex() == 1 || m_pAllStackedWidget->currentIndex() == 5) {
        this->hide();
        kdk::UkuiStyleHelper::self()->removeHeader(this);
        this->show();
        this->activateWindow();

    } else if (m_pAllStackedWidget->currentIndex() == 4 && FilePath == false ||  m_pAllStackedWidget->currentIndex() == 7 || importFiles.size() > 1) {

        QMimeDatabase mimeDatabase;
        QMimeType mimeType;

        for (const QString &importPath : importFiles) {
            QFileInfo fileinfo(importPath);
            if (!fileinfo.isFile()) {
                QMessageBox *msgBox = CustomBoxUtil::createCritialBox("", importPath+QObject::tr(" is not file"), "");
                msgBox->exec();
                qDebug() << importPath << " is not file";
                continue;
            }

            mimeType = mimeDatabase.mimeTypeForFile(importPath);
            if (mimeType.name() == "application/vnd.debian.binary-package") {
                m_pAllStackedWidget->setCurrentIndex(7);
                batch_installWidgeted(QStringList() << importPath);

            } else if (mimeType.name() == "application/vnd.android.package-archive") {
                QMessageBox *msgBox = CustomBoxUtil::createCritialBox("", tr("Batch installation does not support apk package installation!"), "", this);
                msgBox->exec();
                continue;
    
            } else if (fileinfo.suffix() == "ok" || fileinfo.suffix() == "oab") {
                m_pAllStackedWidget->setCurrentIndex(7);
                batch_installWidgeted(QStringList() << importPath);

            } else {
                QMessageBox *msgBox = CustomBoxUtil::createCritialBox("", QObject::tr("This file is not a valid installable package."), "");
                msgBox->exec();
                continue;
            }
        }

    } else {
        QFileInfo fileinfo(importFiles.first());
        QString dst_debPath = fileinfo.absoluteFilePath();
        qDebug() << "kreInstall::secondaryInstallation::" << dst_debPath;
        if (dst_debPath == "/usr/bin/kylin-installer") {
            this->raise();
            this->show();
            this->activateWindow();
            return;
        }

        isFile();                    // 判断文件是否存在
        SECENTSTART = true;//  判断是否是第二次启动
        initDebInfo(dst_debPath);   // 初始化deb信号，获取deb信息
        if (EXITQMESSG == true)
            return;
        resetDebInfo();         // 初始化界面数据
        m_pAllStackedWidget->setCurrentIndex(4);

        this->hide();
        kdk::UkuiStyleHelper::self()->removeHeader(this);
        this->show();
        this->activateWindow();
    }
    return;
}


void kreInstall::resetDebInfo()
{
    m_pOsInstallButton->setText(QObject::tr("one-click install"));

    /*重设安装到系统的界面数据*/
    m_pOsDebMainWidget->resetAppInfo(m_appName, m_appVersion);
    m_pOsDebInstallIngWidget->resetAppInfo(m_appName, m_appVersion);

    /*重设安装到兼容环境的界面数据*/
    m_pdebWidget->resetAppInfo(m_appName, m_appVersion);
    m_pdebInstallIngWidget->resetAppInfo(m_appName, m_appVersion);
}

void kreInstall::getKreAppDesktopFileName()
{
    QDomDocument doc;
    QFile file(XML_APPLICATIONS_PATH);
    if (!file.open(QIODevice::ReadOnly | QFile::Text)) {
        printf("Error : open xml file fail\n");
        return;
    }

    if (!doc.setContent(&file)) {
        printf("Error : load xml file fail\n");
        return;
    }
    file.close();

    QDomElement root = doc.documentElement();
    QDomNode first = root.firstChild();
    QString desktopString;
    desktopString.clear();
    while (!first.isNull()) {
        if (first.nodeName() == "appPkgName") {
            QString pkg_name = first.toElement().attribute("name");
            if (pkg_name == m_appName) {
                QDomNode second = first.firstChild();
                while (!second.isNull()) {
                    desktopString = second.toElement().attribute("desktop");
                    second = second.nextSibling();
                    if (desktopString == nullptr) {
                        continue;
                    }
                    QString desktopPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
                    QString fullFilePath = desktopPath + "/" + desktopString;
                    if (fullFilePath == desktopPath + "/") {
                        continue;
                    }
                    QFileInfo fileInfo(fullFilePath);
                    if (fileInfo.exists()) {
                        QString exec = "rm " + fullFilePath;
                        qDebug() << "desktop文件路径，需要删除的文件" << exec;
                        QProcess p(0);
                        p.startDetached(exec);
                        p.waitForStarted();
                    }
                }
            }
        }
        first = first.nextSibling();
    }

    return;
}

// 麒麟安装器一键安装按钮槽函数
void kreInstall::OsInstallButtonSlots()
{
    bool app_version_status;
    Package *package     = PackageFactory::createPackage(m_debPath);
    PackageCheckUtils::checkAndLogPackageStatus(package, Q_FUNC_INFO);
    QString appname_test = package->m_name;
    QString zh_nameapp   = package->m_zhName;
    app_version_status   = app_installed(m_debPath);

    if (app_version_status) {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("install error!"));
        msgBox.setIcon(QMessageBox::Warning);
        if (zh_nameapp != nullptr) {
            QString lang = getenv("LANG");
            if (lang.contains("zh_CN")) {
                QString info = tr("The current version of the software is already installed on the system");
                msgBox.setText('\"'+zh_nameapp+'\"'+" "+info);
            } else {
                QString info = tr("The current version of the software is already installed on the system");
                msgBox.setText('\"'+appname_test+'\"'+" "+info);
            }
        } else {
            QString info = tr("The current version of the software is already installed on the system");
            msgBox.setText('\"'+appname_test+'\"'+" "+info);
        }

        CACLINFO        = false;
        CANSCELISNTALL  = false;
        install_flag    = 0;

        QPushButton *btnno = new QPushButton();
        btnno->setText(tr("Cancel installation"));
        msgBox.addButton(btnno, QMessageBox::ActionRole);

        QPushButton *btnyes = new QPushButton();
        btnyes->setText(tr("continue installation"));
        msgBox.addButton(btnyes, QMessageBox::RejectRole);

        connect(btnno,  SIGNAL(clicked(bool)), this, SLOT(cancel_installed()));
        connect(btnyes, SIGNAL(clicked(bool)), this, SLOT(continue_install()));

        msgBox.exec();

        if (CACLINFO == false && install_flag == 0)
            cancel_installed();
    }

    if (CANSCELISNTALL == true) {
        kreInStallUnlock();
        CANSCELISNTALL=false;
        return;
    } else {
        install_status = true;
        setInstallThread(m_debPath);
    }
}

/* 兼容环境安装失败 */
void kreInstall::installFailSlot()
{
    m_pAllStackedWidget->setCurrentIndex(3);
}

/* 兼容环境安装成功 */
void kreInstall::installSuccessSlot()
{
    m_pAllStackedWidget->setCurrentIndex(2);
    getKreAppDesktopFileName();
}

void kreInstall::minBtnClicked()
{
    qDebug() << "min button clicked, hide .";
    this->showMinimized();
}

void kreInstall::closeBtnClicked(bool status)
{
    if (install_status) {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setIconPixmap(QIcon::fromTheme("dialog-warning").pixmap(24,24));
        msgBox.setText(tr("There are still tasks in progress. Are you sure you want to exit?"));
        msgBox.setWindowFlags(Qt::Drawer);

        QPushButton *btnYes = msgBox.addButton(tr("OK"), QMessageBox::YesRole);
        QPushButton *btnNo  = msgBox.addButton(tr("Cancel"), QMessageBox::NoRole);

        msgBox.exec();

        if (msgBox.clickedButton() == btnNo) {
            qDebug() << "User canceled exit.";
            return;
        }

        qDebug() << "User confirmed exit.";
    }

    qDebug() << "close button clicked, exit.";

    m_pKylinInstalldbusCallThread.release();
    kreInStallUnlock();

    exit(0);
}

void kreInstall::batchlist()
{
    if(FilePath == false)
        m_pAllStackedWidget->setCurrentIndex(7);
    else
        m_pAllStackedWidget->setCurrentIndex(3);

}

/*
 * 解锁
*/
void kreInstall::kreInStallUnlock()
{
    int ret = flock(gi_fd, LOCK_UN);
    qDebug() << "解锁flock 返回值======" << ret;
}

void kreInstall::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    QStyleOption option;
    p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect(), 0, 0); // 左上右下
    p.setOpacity(1);
    p.save();
    p.fillPath(rectPath, option.palette.color(QPalette::Base));
    p.restore();
}


/**************************************安卓函数***************************/
bool kreInstall::isAndroidReady(const QString &username, const QString &uid)
{
    QString value;
    QDBusInterface *interface;
    interface = new QDBusInterface("cn.kylinos.Kmre", 
                                   "/cn/kylinos/Kmre", 
                                   "cn.kylinos.Kmre", 
                                   QDBusConnection::systemBus());

    qDebug() << "[DBus] call GetPropOfContainer: username: " << username
             << ", uid:" << uid.toInt()
             << ", property:" << "sys.kmre.boot_completed";

    QDBusMessage response = interface->call(
                                "GetPropOfContainer", 
                                username, 
                                uid.toInt(), 
                                "sys.kmre.boot_completed" );

    if (response.type() == QDBusMessage::ErrorMessage) {
        qDebug() << "DBus call error:" << response.errorMessage();
        delete interface;
        return false;
    }

    if (response.type() == QDBusMessage::ReplyMessage)
        value = response.arguments().takeFirst().toString();

    delete interface;
    
    qDebug() << "[DBus] call GetPropOfContainer return value: " << value;

    return (value == "1");
}

/********getusername******/
QString kreInstall::getUserName()
{
    std::string user_name;

    struct passwd *pwd = NULL;
    pwd = getpwuid(getuid());
    if (pwd && pwd->pw_name) {
        char* _user = pwd->pw_name;
        struct passwd *q = NULL;
        q = getpwnam(_user);
        if (q && (q->pw_uid != getuid())) {
            fprintf(stderr, "User name doesn't match uid.\n");
            return QString("");
        }

        return QString::fromStdString(std::string(pwd->pw_name));
    }
    else {
        user_name = std::getenv("USER");
        if (user_name.empty()) {
            user_name = std::getenv("USERNAME");
        }
        if (user_name.empty()) {
            char name[16];
            snprintf(name, sizeof(name), "%u", getuid());
            user_name = std::string(name);
        }
    }

    return QString::fromStdString(user_name);
}

/********getusrid********/
QString kreInstall::getUid()
{
    uid_t uid = getuid();
    return QString::number(uid);
}

int kreInstall::isAndroidEnvInstalled()
{
    int nRes = -1;
    QString libPath = SystemInfoUtil::getValidFilePathFromV11(SystemInfoUtil::INSTALL_LIB_PATH,"libkmre.so");
    if (!QFile::exists(libPath)) {
        return nRes;
    }

    void *module_handle;
    char *module_error;
    module_handle = dlopen(libPath.toStdString().c_str(), RTLD_LAZY);
    if (!module_handle) {
        return nRes;
    }

    bool (*is_android_env_installed)();
    is_android_env_installed = (bool(*)())dlsym(module_handle, "is_android_env_installed");
    if ((module_error = dlerror()) != NULL) {
        dlclose(module_handle);
        return nRes;
    }
    bool ret = is_android_env_installed();
    dlclose(module_handle);

    if (ret) {
        nRes = 1;
    }
    else {
        nRes = 0;
    }

    return nRes;
}



/********初始化安卓apk包信息*****/
void kreInstall::onAnalysisApkFile(QString debPath)
{
    QString m_loginUserName = getUserName();
    QString m_loginUserId = getUid();


    int value = isAndroidEnvInstalled();
    if (value == 0) {
        QMessageBox *msgBox = CustomBoxUtil::createCritialBox(tr("open file error"),tr("Kylin mobile operating environment is not installed!!!"),"",this);
        msgBox->exec();
        msgBox->setContextMenuPolicy(Qt::NoContextMenu);
        if (SECENTSTART ==  false){
            exit(0);

        }
        else
        {
            EXITQMESSG = true;
        }
    }

    bool androidReady = isAndroidReady(m_loginUserName, m_loginUserId);
    if (!androidReady) {
        qDebug() << "Kmre boot_completed is false, mobile running environment has not started ...";
        QMessageBox *msgBox = CustomBoxUtil::createCritialBox(tr("open file error"),tr("Kylin mobile running environment has not started, unable to install APK files!!!"),"",this);
        msgBox->exec();
        msgBox->setContextMenuPolicy(Qt::NoContextMenu);
        if (SECENTSTART ==  false)
            exit(0);
        else
            EXITQMESSG = true;
    }

    m_appName.clear();
    m_appVersion.clear();
    QFileInfo file_info(debPath);
    if (!file_info.exists()) {
        return;
    }

    qApp->processEvents();

    QString application_label = "";
    QString application_zh_label = "";
    QString fileSize = "";
    QString iconPath = "";
    QString version = "";

    QProcess process;
    QStringList cmdList;


    fileSize = QString("%1 KB (%2 MB)").arg(file_info.size()/1024).arg(file_info.size()/1048576);

    //s#ifndef KYLIN_V10
    if (QFileInfo("/usr/bin/aapt").exists()) {
        process.start("aapt", QStringList()<< "d" << "badging" << debPath);
        process.waitForStarted(5*1000);
        process.waitForFinished(10*1000);//process.waitForFinished(-1);
        const QString output = process.readAllStandardOutput();//process.readAll();

        for (const auto &line : output.split('\n')) {
            if (line.startsWith("package:")) {
                const QStringList &info = QString(line).split(' ');
                for (int i=0;i<info.size();i++) {
                    if (QString(info.at(i)).startsWith("name=")) {
                        const QStringList &nameinfo = QString(info.at(i)).split('=');
                        m_appName = QString(nameinfo.at(1)).replace("\'", "").trimmed();//com.rightware.BasemarkOSII
                    }
                    if (QString(info.at(i)).startsWith("versionName=")) {
                        const QStringList &versioninfo = QString(info.at(i)).split('=');
                        version = QString(versioninfo.at(1)).replace("\'", "").trimmed();//2.0
                        m_appVersion = version;
                    }
                }
            }

            if (QString(line).startsWith("application-label:")) {//可能不存在该行
                const QStringList &info = QString(line).split(':');
                if (info.size() == 2) {
                    application_label = QString(info.at(1)).replace("\'", "").trimmed();
                }
            }
            if (line.startsWith("application-label-zh-CN:")) {//可能不存在该行
                const QStringList &info = QString(line).split(':');
                if (info.size() == 2) {
                    application_zh_label = QString(info.at(1)).replace("\'", "").trimmed();
                }
            }
            if (line.startsWith("application-label-zh:")) {//可能不存在该行
                const QStringList &info = QString(line).split(':');
                if (info.size() == 2) {
                    if (application_zh_label.isEmpty()) {
                        application_zh_label = QString(info.at(1)).replace("\'", "").trimmed();
                    }
                }
            }
            if (line.startsWith("application: label=")) {
                int firstIndex = QString(line).indexOf(QChar('\''));
                int secondIndex = QString(line).indexOf(QChar('\''), strlen("application: label='"));
                QString labelContent = QString(line).left(secondIndex);
                QString labelStr = labelContent.right(labelContent.length() - 1 - firstIndex);
                if (application_label.isEmpty()) {
                    application_label = labelStr;
                }
                if (application_zh_label.isEmpty()) {
                    application_zh_label = labelStr;
                }

                const QStringList &info = QString(line).split(' ');
                for (int i=0;i<info.size();i++) {
                    if (QString(info.at(i)).startsWith("icon=")) {
                        //icon path
                        if (iconPath.isEmpty()) {
                            const QStringList &iconinfo = QString(info.at(i)).split('=');
                            iconPath = QString(iconinfo.at(1)).replace("\'", "").trimmed();
                        }
                    }
                }
            }
            if (QString(line).startsWith("launchable-activity:")) {//launchable-activity: name='com.rightware.BasemarkOSII.BasemarkOSII'  label='Basemark OS II' icon=''
                int firstIndex = QString(line).indexOf(QChar('\''));
                int secondIndex = QString(line).indexOf(QChar('\''), strlen("application: label='"));
                QString labelContent = QString(line).left(secondIndex);
                QString labelStr = labelContent.right(labelContent.length() - 1 - firstIndex);
                if (application_label.isEmpty()) {
                    application_label = labelStr;
                }
                if (application_zh_label.isEmpty()) {
                    application_zh_label = labelStr;
                }

                const QStringList &info = QString(line).split(' ');
                for (int i=0;i<info.size();i++) {
                    if (QString(info.at(i)).startsWith("icon=")) {
                        //icon path
                        if (iconPath.isEmpty()) {
                            const QStringList &iconinfo = QString(info.at(i)).split('=');
                            iconPath = QString(iconinfo.at(1)).replace("\'", "").trimmed();
                        }
                    }
                }
            }
            if (!application_label.isEmpty() && !(QString(m_appName).isEmpty())) {
                if (application_zh_label.isEmpty()) {
                    application_zh_label = application_label;
                }
                m_application = application_label;
                m_applicationZh = application_zh_label;
            }
        }
    } else {
        qDebug() << "aapt not found, mobile running environment has not started ...";
        QMessageBox *msgBox = CustomBoxUtil::createCritialBox(tr("open file error"),tr("Kylin mobile running environment has not started, unable to install APK files!!!"),"",this);
        msgBox->exec();
        msgBox->setContextMenuPolicy(Qt::NoContextMenu);
        if (SECENTSTART ==  false) {
            exit(0);
        } else {
            EXITQMESSG = true;
        }
    }
}

/**********批量安装添加文件后界面显示**********/
void kreInstall::batch_installWidgeted(QStringList pkgPathLists)
{
    qDebug() <<"Batch install add files:" << pkgPathLists;

    if (pkgListAdded->count() == 0)
        Batch_install->setEnabled(true);

    m_pAllStackedWidget->setCurrentIndex(7);

    for (int i = 0; i < pkgPathLists.size(); ++i) {
        QString currentPkgPath = pkgPathLists.at(i);

        Package *package = PackageFactory::createPackage(currentPkgPath);
        if (!PackageCheckUtils::checkAndLogPackageStatus(package, Q_FUNC_INFO)) {
            debDamageInfo();
            continue;
        }

        QString md5sum      = package->m_md5Sum;
        QString appName     = package->m_name;
        QString appNameZh   = package->m_zhName;
        QString appVersion  = package->m_version;
        QString appArch     = package->m_architecture;

        if (QFileInfo(currentPkgPath).fileName().contains("`",Qt::CaseSensitive)) {
            QMessageBox *msgBox = CustomBoxUtil::createCritialBox(
                                                        "",
                                                        tr("Open file error\n"
                                                            "Naming contains nonstandard characters '`'"),
                                                        "",
                                                        this);
            msgBox->exec();
            continue;
        }

        canceladd   = 0;
        path_deb    = currentPkgPath;

        QFileInfo fileInfo(currentPkgPath);
        if (fileInfo.suffix() == "deb") {
            if (!package->packageArchIsMatch()) {
                QMessageBox *msgBox = CustomBoxUtil::createCritialBox("",tr("The software package architecture does not match the native architecture"),"",this);
                msgBox->exec();
                msgBox->setContextMenuPolicy(Qt::NoContextMenu);
                continue;
            }
        }

        for (QMap<QString, QString>::iterator it = maplist.begin(); it != maplist.end(); it++) {
            if ((appNameZh == it.key() || appName == it.key()) && appVersion != it.value()) {
                QMessageBox msgBox(this);
                msgBox.setWindowFlags(Qt::Drawer);

                if (appNameZh != nullptr && QString(getenv("LANG")).contains("zh_CN")) {
                    msgBox.setText( tr("In the list ") + \
                                    appNameZh + \
                                    tr(" already exists ") + \
                                    it.value() + \
                                    tr(" version, currently to be added ") + \
                                    appNameZh + \
                                    tr(" of ") + \
                                    appVersion + \
                                    tr(" version, select replace?"));
                } else {
                    msgBox.setText( tr("In the list ") + \
                                    appName + \
                                    tr(" already exists ") + \
                                    it.value() + \
                                    tr(" version, currently to be added ") + \
                                    appName + \
                                    tr(" of ") + \
                                    appVersion + \
                                    tr(" version, select replace?"));
                }

                QPushButton *btnno = new QPushButton();
                btnno->setText(tr("Cancel"));
                msgBox.addButton(btnno, QMessageBox::AcceptRole);

                QPushButton *btnyes = new QPushButton();
                btnyes->setText(tr("Replace"));
                msgBox.addButton(btnyes, QMessageBox::AcceptRole);

                connect(btnyes, SIGNAL(clicked(bool)), this, SLOT(replace_application()));
                connect(btnno,  SIGNAL(clicked(bool)), this, SLOT(canseaddapplication()));

                msgBox.exec();
            }
        }

        if (canceladd == 1)
            continue;

        if (appNameZh != nullptr)
            maplist.insert(appNameZh, appVersion);
        else
            maplist.insert(appName, appVersion);

        if (pkgListAdded->contains(currentPkgPath)) {
            QApplication::setActiveWindow(this);
            this->setFocus();

            QMessageBox msgBox(QApplication::activeWindow());
            msgBox.setWindowFlags(Qt::Drawer);
            msgBox.setIcon(QMessageBox::Warning);
            this->show();

            if (appNameZh != nullptr) {
                if (QString(getenv("LANG")).contains("zh_CN")){
                    QString info = tr("The file already exists in the installation list");
                    msgBox.setText('\"'+appNameZh+'\"'+info);
                } else {
                    QString info = tr("The file already exists in the installation list");
                    msgBox.setText('\"'+appName+'\"'+info);
                }
            } else {
                QString info = tr("The file already exists in the installation list");
                msgBox.setText('\"'+appName+'\"'+info);
            }

            QPushButton *btn = new QPushButton();
            btn->setText(tr("OK"));
            msgBox.addButton(btn, QMessageBox::AcceptRole);
            msgBox.exec();
            continue;

        } else if (pkgMd5ListAdded->contains(md5sum)) {
            QApplication::setActiveWindow(this);
            this->setFocus();

            QMessageBox msgBox(QApplication::activeWindow());
            msgBox.setWindowFlags(Qt::Drawer);;
            msgBox.setIcon(QMessageBox::Warning);

            if (appNameZh != nullptr) {
                QString lang = getenv("LANG");
                if (lang.contains("zh_CN")) {
                    QString info = tr("The file already exists in the installation list");
                    msgBox.setText('\"'+appNameZh+'\"'+info);
                } else {
                    QString info = tr("The file already exists in the installation list");
                    msgBox.setText('\"'+appName+'\"'+info);
                }
            } else {
                QString info = tr("The file already exists in the installation list");
                msgBox.setText('\"'+appName+'\"'+info);
            }

            QPushButton *btn = new QPushButton();
            btn->setText(tr("OK"));
            msgBox.addButton(btn, QMessageBox::AcceptRole);
            msgBox.exec();
            continue;
        }

        pkgMd5ListAdded->append(md5sum);
        pkgListAdded->append(currentPkgPath);
        adddebpathcompail (currentPkgPath);
    }

    if(pkgListAdded->size() == 0)
        return_homepage();
}

/********文件添加函数*******/
void kreInstall::adddebpathcompail(QString pkgPath)
{
    batchInstallWidget      = new Batchinstall(pkgPath);
    batchInstallWidget->setProperty("useIconHighlightEffect", 0x2);

    QListWidgetItem *item   = new QListWidgetItem();
    QSize size = item->sizeHint();
    item->setSizeHint(QSize(size.width(), 70));
    item->setFlags(Qt::ItemIsEnabled);

    batchInstallWidget->setSizeIncrement(QSize(size.width(), 85));

    batchInstallWidgetList->addItem(item);
    batchInstallWidgetList->setItemWidget(item, batchInstallWidget);
    batchInstallWidgetList->setStyleSheet("QListWidget{background-color:rgba(0,0,0,0);border:0px solid #cfcfcf;}QListWidget::item{background-color:rgba(0,0,0,0);height:65px;margin-top:0px;border:0px solid #cfcfcf;}QListWidget::item:hover{background-color:background-color:rgba(0,0,0,0);}");

    connect(batchInstallWidget, &Batchinstall::deleteitemstring, this, &kreInstall::deletewidgetitem);
}

/**************删除界面item***************/
void kreInstall::deletewidgetitem(QString itemwidget)
{
    if (batchInstallWidgetList->count() == pkgListAdded->size()) {
        for (int i = 0; i< batchInstallWidgetList->count(); i++) {
            if (itemwidget == pkgListAdded->at(i)) {
                QListWidgetItem *item1 = batchInstallWidgetList->takeItem(i);
                batchInstallWidgetList->removeItemWidget(item1);
                delete item1;
                pkgListAdded->removeAt(i);
                break;
            }
        }

    } else {
        QString appname_test = getDebInfoFromDeb(itemwidget, "Package");
        QString zh_nameapp   =  m_pdebWidget->getAppCnName(appname_test);

        for (int i = 0; i< batchInstallWidgetList->count(); ++i) {
            QListWidgetItem *item1 = batchInstallWidgetList->item(i);
            QWidget * widget2 = batchInstallWidgetList->itemWidget(item1);
            if (widget2 !=NULL) {
                QList <QLabel*> labellist = widget2->findChildren<QLabel*>();
                foreach(QLabel *label,labellist){
                    if (label->text().toLower() == appname_test.toLower() || (label->text() == zh_nameapp && zh_nameapp!="")) {
                        QListWidgetItem *item3 = batchInstallWidgetList->takeItem(i);
                        batchInstallWidgetList->removeItemWidget(item1);
                        delete item3;
                        break;
                    }
                }
            }
        }

        for (int i = 0; i< pkgListAdded->size(); i++) {
            if (itemwidget == pkgListAdded->at(i))
                pkgListAdded->removeAt(i);
        }
    }


    Package *package = PackageFactory::createPackage(itemwidget);
    PackageCheckUtils::checkAndLogPackageStatus(package, Q_FUNC_INFO);

    QString md5sum = package->m_md5Sum;
    for (int i = 0; i< pkgMd5ListAdded->count(); i++) {
        if (pkgMd5ListAdded->at(i) == md5sum)
            pkgMd5ListAdded->removeAt(i);
    }

    if (pkgListAdded->size() == 0)
        return_homepage();
}

/*******循环播放安装中图片******/
void kreInstall::play_installingpicture()
{
    QString teststr;
    for (int i = 0; i< batchInstallWidgetList->count();++i) {
        Package *package = PackageFactory::createPackage(change_installapp);
        PackageCheckUtils::checkAndLogPackageStatus(package, Q_FUNC_INFO);

        QString appname_test    = package->m_name;
        QString zh_nameapp      =  package->m_zhName;
        QListWidgetItem *item1  = batchInstallWidgetList->item(i);
        QWidget *widget2        = batchInstallWidgetList->itemWidget(item1);

        if (widget2 !=NULL) {
            QList <QLabel*> labellist = widget2->findChildren<QLabel*>();
            foreach (QLabel *label,labellist) {
                if (label->text().toLower() == appname_test.toLower() || (label->text() == zh_nameapp && zh_nameapp != "")) {
                    foreach (QLabel *label1,labellist) {
                        if (label1->objectName() == "app_status") {
                            teststr=QString::number(settime, 10);
                            label1->show();
                            label1->setProperty("useIconHighlightEffect", 0x2);
                            label1->setPixmap(QIcon::fromTheme("ukui-loading-symbolic-"+teststr).pixmap(QSize(20, 20)));
                            settime += 1;
                            if(settime == 7)
                                settime = 0;
                            break;
                        }
                    }
                }
            }
        }
    }
}

/**********自动删除item*****/
void kreInstall::automatic_deletewidgetitem()
{
    Package *package = PackageFactory::createPackage(flagstring);
    PackageCheckUtils::checkAndLogPackageStatus(package, Q_FUNC_INFO);

    QString appname_test    = package->m_name;
    QString zh_nameapp      = package->m_zhName;
    QString md5sum          = package->m_md5Sum;

    for (int i = 0; i< batchInstallWidgetList->count();++i) {
        QListWidgetItem *item1 = batchInstallWidgetList->item(i);
        QWidget * widget2 = batchInstallWidgetList->itemWidget(item1);
        if (widget2 !=NULL) {
            QList <QLabel*> labellist = widget2->findChildren<QLabel*>();
            foreach (QLabel *label,labellist) {
                if (label->text().toLower() == appname_test.toLower() || (label->text() == zh_nameapp && zh_nameapp!="")) {
                    QListWidgetItem *item3 = batchInstallWidgetList->takeItem(i);
                    batchInstallWidgetList->removeItemWidget(item1);
                    delete item3;
                    break;
                }
            }
        }
    }

    for (int i = 0; i< pkgMd5ListAdded->count(); i++) {
        if (pkgMd5ListAdded->at(i) == md5sum)
            pkgMd5ListAdded->removeAt(i);
    }

    if (pkgListAdded->size() == 0 && appinstallstatus == true) {
        playSoundEffects();
        m_pAllStackedWidget->setCurrentIndex(8);
    }
}

//dex=0 安装成功，dex=1，安装失败，dex=2，安装中, dex=3 正在安装kare环境。
/**********************根据安装信号来修改itemwidget中的button状态********************/
void kreInstall::changebuttonstatus(QString itemwidget, int dex)
{
    QPalette pe = palette();
    pe.setColor(QPalette::Text, Qt::red);
    for (int i = 0; i < batchInstallWidgetList->count(); ++i) {
        Package *package = PackageFactory::createPackage(itemwidget);
        PackageCheckUtils::checkAndLogPackageStatus(package, Q_FUNC_INFO);

        QString appname_test = package->m_name;
        QString zh_nameapp = package->m_zhName;
        QListWidgetItem *item1 = batchInstallWidgetList->item(i);
        QWidget * widget2 = batchInstallWidgetList->itemWidget(item1);

        if (widget2 != NULL) {
            if (dex == 0) {
                QList <QLabel*> labellist = widget2->findChildren<QLabel*>();
                foreach(QLabel *label,labellist) {
                    if (label->text().toLower() == appname_test.toLower() || (label->text() == zh_nameapp && zh_nameapp!="")) {
                        QList <QToolButton*> toolButtonList = widget2->findChildren<QToolButton*>();
                        foreach(QToolButton *button, toolButtonList) {
                            if(button->objectName() == "delete_app")
                                button->hide();
                        }

                        foreach(QLabel *label1,labellist){
                            if (label1->objectName() == "app_status"){
                                label1->show();
                                label1->setPixmap(QIcon::fromTheme("ukui-dialog-success").pixmap(QSize(20,20)));
                            }
                            if (label1->objectName() == "progress")
                                label1->hide();
                            if (label1->objectName() == "appstatus")
                                label1->hide();
                            QList <QProgressBar*> qprolist = widget2->findChildren<QProgressBar*>();
                            foreach(QProgressBar *progress,qprolist){
                                progress->hide();
                                progress->setValue(0);
                            }
                        }
                    }
                }
                QList <QProgressBar*> qprolist = widget2->findChildren<QProgressBar*>();
                foreach(QProgressBar *progress,qprolist)
                    progress->hide();
            
            } else if (dex ==1) {

                QList <QLabel*> labellist = widget2->findChildren<QLabel*>();
                foreach(QLabel *label,labellist){
                    if(label->text().toLower() == appname_test.toLower()|| (label->text() == zh_nameapp && zh_nameapp!="")){
                        QList <QToolButton*> toolButtonList = widget2->findChildren<QToolButton*>();
                        foreach(QToolButton *button, toolButtonList) {
                            if(button->objectName() == "delete_app")
                                button->hide();
                        }

                        foreach(QLabel *label1,labellist){
                            if (label1->objectName() == "app_status"){
                                label1->show();
                                label1->setPixmap(QIcon::fromTheme("dialog-error").pixmap(QSize(20,20)));
                            }
                            if (label1->objectName() == "progress")
                                label1->hide();
                            if (label1->objectName() == "appstatus"){
                                label1->setText(tr("install Fail"));
                                pe.setBrush(QPalette::Active,QPalette::Text,QColor(255,0,0));
                                label1->setPalette(pe);
                                label1->show();
                                QList <QProgressBar*> qprolist = widget2->findChildren<QProgressBar*>();
                                foreach(QProgressBar *progress,qprolist){
                                    progress->hide();
                                    progress->setValue(0);
                                }
                            }
                        }
                    }
                }
                QList <QProgressBar*> qprolist = widget2->findChildren<QProgressBar*>();
                foreach(QProgressBar *progress,qprolist)
                    progress->hide();
            
            } else if (dex ==2) {

                QList <QLabel*> labellist = widget2->findChildren<QLabel*>();
                foreach(QLabel *label,labellist) {

                    if (label->text().toLower() == appname_test.toLower() || (label->text() == zh_nameapp && zh_nameapp!="")) {
                        QList <QToolButton*> toolButtonList = widget2->findChildren<QToolButton*>();
                        foreach(QToolButton *button, toolButtonList) {
                            if(button->objectName() == "delete_app")
                                button->hide();
                        }

                        foreach(QLabel *label1,labellist){
                            if (label1->objectName() == "app_status") {
                                label1->hide();
                            }
                            if (label1->objectName() == "progress") {
                                label1->setText("0%");
                                label1->show();
                            }
                            if (label1->objectName() == "appstatus")
                                label1->hide();
                        }

                        QList <QProgressBar*> qprolist = widget2->findChildren<QProgressBar*>();
                        foreach(QProgressBar *progress,qprolist) {
                            progress->setValue(0);
                            progress->show();
                        }
                        break;
                    }
                }

            } else if (dex == 3) {

                QList <QLabel*> labellist = widget2->findChildren<QLabel*>();
                foreach(QLabel *label,labellist) {
                    if(label->text().toLower() == appname_test.toLower() || (label->text() == zh_nameapp && zh_nameapp!="")) {
                        foreach(QLabel *label1,labellist) {
                            if (label1->objectName() == "appstatus")
                                label1->setText(tr("Installing kare environment"));
                        }
                    }
                }
            }
        }
    }
}

//安装成功图标：ukui-dialog-success
//安装失败图标：dialog-erro

/*********批量安装槽函数********/
// 麒麟安装器一键安装按钮槽函数
void kreInstall::Batch_OsInstallButtonSlots()
{
    Batch_install->setEnabled(false);

    if (pkgListAdded->size() == 0)
        return;

    app_installed_fast();

    if (pkgListAdded->size() > 0 ) {
        install_status = true;
        Batch_setInstallThread(pkgListAdded->at(0));
    }
}

bool kreInstall::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == this)
    {
        if(event->type() == QEvent::KeyPress){
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if(keyEvent->key() == Qt::Key_F1){
                //显示用户手册
                // kdk::kabase::UserManualManagement userManualTest;
                // if (!userManualTest.callUserManual("kylin-installer")) {
                //     qCritical() << "user manual call fail!";
                // }
                showGuide();
            }
        }
    }
}

void kreInstall::showGuide()
{
    QString service_name = QString("%1_%2").arg("com.kylinUserGuide.hotel").arg(getuid());
    qDebug() << "service_name  " << service_name;

    QDBusMessage m = QDBusMessage::createMethodCall(QString(service_name),"/","com.guide.hotel","showGuide");
    m << "kylin-installer";

    // 发送Message
    QDBusMessage response = QDBusConnection::sessionBus().call(m);

    // 判断Method是否被正确返回
    if (response.type()!= QDBusMessage::ReplyMessage) {
        qDebug()<<"showGuide In fail!\n";
    }
}

void kreInstall::addBtnClicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    dialog.setNameFilter(tr("Supported Files")+" (*.deb *.ok *.oab)");
    dialog.setDirectory(historyPath);

    const int mode = dialog.exec();
    if (mode != QDialog::Accepted) {
        return;
    }
    const QStringList selected_files = dialog.selectedFiles();
    if (!selected_files.isEmpty()) {
        QFileInfo info(selected_files.at(0));
        historyPath = info.absolutePath();
    }
    batch_installWidgeted(selected_files);
}

void kreInstall::batchInstallListInit(QStringList importFiles)
{
    if (!importFiles.isEmpty()) {
        batch_installWidgeted(importFiles);
    }
}

void kreInstall::moveCenterScreen()
{
    QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
    this->move((availableGeometry.width() - this->width())/2, (availableGeometry.height() - this->height())/2);
}

void kreInstall::Batch_setInstallThread(QString debPath)
{
    change_installapp = debPath;
    if (SystemInfoUtil::isOsTree)
    {
        m_pKylinInstalldbusCallThread.reset(new KylinInstalldbusCallThread(change_installapp,m_appName,m_application,m_applicationZh,m_appVersion));
        connect(m_pKylinInstalldbusCallThread.get(), &KylinInstalldbusCallThread::kylinOsinstallStatus, \
        this, &kreInstall::Batch_osInstallOverSoftwareAptSlots);
        connect(m_pKylinInstalldbusCallThread.get(), &KylinInstalldbusCallThread::kylinbatchOsinstalldingStatus, \
        this, &kreInstall::batchInstalldingStatus);
        connect(m_pKylinInstalldbusCallThread.get(), &KylinInstalldbusCallThread::kylinKareEnvInstallStatus, \
        this, &kreInstall::kareBaseEnvInstallStatusSlot);
        connect(m_pKylinInstalldbusCallThread.get(), &KylinInstalldbusCallThread::kylinKareEnvInstallingStatus, \
        this, &kreInstall::kareBaseEnvInstallingStatusSlot);
        changebuttonstatus(change_installapp,2);
    }
    else
    {
        PackageEnum::Type packageType(PackageEnum::Type::OTHERS);
        QString packageDistribution;
        bool isInstalledKareBaseEnv = false;
        PackageUtils::getPackageEnumInfo(change_installapp, packageType, packageDistribution, isInstalledKareBaseEnv);
        if (packageType == PackageEnum::Type::KARE)
        {
            if (!isInstalledKareBaseEnv) {
                
                if (!confirmInstallKareEnvMessageBox())
                {
                    Batch_osInstallOverSoftwareAptSlots(false,"取消安装");
                    return ;
                }
                if (packageDistribution.isEmpty()) {
                    Batch_osInstallOverSoftwareAptSlots(false,"未找到适配的kare环境");
                    return ;
                }
                m_pKylinInstalldbusCallThread.reset(new KylinInstalldbusCallThread(change_installapp,packageDistribution,PackageEnum::Type::KARE_ENV));
                connect(m_pKylinInstalldbusCallThread.get(), &KylinInstalldbusCallThread::kylinKareEnvInstallStatus, \
                this, &kreInstall::Batch_kareBaseEnvInstallStatusSlot);
                connect(m_pKylinInstalldbusCallThread.get(), &KylinInstalldbusCallThread::kylinKareEnvInstallingStatus, \
                this, &kreInstall::kareBaseEnvInstallingStatusSlot);
                m_pKylinInstalldbusCallThread->start();
                changebuttonstatus(change_installapp,3);
                setKareVersionShow();
                kareEnvResetConnect();
            }
            else
            {           
                m_pKylinInstalldbusCallThread.reset(new KylinInstalldbusCallThread(change_installapp,packageDistribution,packageType));
                connect(m_pKylinInstalldbusCallThread.get(), &KylinInstalldbusCallThread::kylinOsinstallStatus, \
                this, &kreInstall::Batch_osInstallOverSoftwareAptSlots);
                connect(m_pKylinInstalldbusCallThread.get(), &KylinInstalldbusCallThread::kylinbatchOsinstalldingStatus, \
                this, &kreInstall::batchInstalldingStatus);
                changebuttonstatus(change_installapp,2);
                setKareVersionShow();
            }
        }
        else
        {
            m_pKylinInstalldbusCallThread.reset(new KylinInstalldbusCallThread(change_installapp,m_appName,m_application,m_applicationZh,m_appVersion));
            connect(m_pKylinInstalldbusCallThread.get(), &KylinInstalldbusCallThread::kylinOsinstallStatus, \
                this, &kreInstall::Batch_osInstallOverSoftwareAptSlots);

            connect(m_pKylinInstalldbusCallThread.get(), &KylinInstalldbusCallThread::kylinbatchOsinstalldingStatus, \
                this, &kreInstall::batchInstalldingStatus);
            changebuttonstatus(change_installapp,2);
        }

    }
    m_pKylinInstalldbusCallThread->start();
}

void kreInstall::kareBaseEnvInstallStatusSlot(bool status, QString info)
{
    disconnect(m_pKylinInstalldbusCallThread.get(), &KylinInstalldbusCallThread::kylinKareEnvInstallStatus, \
            this, &kreInstall::kareBaseEnvInstallStatusSlot);

    kareEnvBuilded = true;

    if (!this->isVisible())
    {
        closeBtnClicked(true);
    }
    if (resetCloseFlag)
    {
        kareEnvRestoreConnect();
    }
    if (m_pKareEnvProgressDialog->isVisible())
    {
        m_pKareEnvProgressDialog->hide();
    }
    if (status == true)
    {
        m_pPackageInstallLabelWidget->show();
        m_pKareInstallEnvLabelWidget->hide();
        if (!SystemInfoUtil::isOsTree)
        {
            OsInstallButtonSlots();
        }
    }
    else {
        osInstallOverSoftwareAptSlots(false,info);
    }
}

void kreInstall::Batch_kareBaseEnvInstallStatusSlot(bool status, QString info) 
{
    disconnect(m_pKylinInstalldbusCallThread.get(), &KylinInstalldbusCallThread::kylinKareEnvInstallStatus, \
            this, &kreInstall::Batch_kareBaseEnvInstallStatusSlot);
    if (!this->isVisible())
    {
        closeBtnClicked(true);
    }
    if (resetCloseFlag)
    {
        kareEnvRestoreConnect();
    }
    if (m_pKareEnvProgressDialog->isVisible())
    {
        m_pKareEnvProgressDialog->hide();
    }
    if (status == true)
    {
        changebuttonstatus(change_installapp,2);
        if (!SystemInfoUtil::isOsTree)
        {
            Batch_OsInstallButtonSlots();
        }
    }
    else {
        changebuttonstatus(change_installapp,1);
        Batch_osInstallOverSoftwareAptSlots(false,info);
    }

}

void kreInstall::kareBaseEnvInstallingStatusSlot(int progress) 
{
    if (FilePath) {
        if (m_pPackageInstallLabelWidget->isVisible())
            m_pPackageInstallLabelWidget->hide();

        if (!m_pKareInstallEnvLabelWidget->isVisible())
            m_pKareInstallEnvLabelWidget->show();
    } else {
        if (!resetCloseFlag)
            changebuttonstatus(change_installapp,4);
    }

    if (!resetCloseFlag)
        kareEnvResetConnect();

    if (!m_pKareEnvProgressDialog->isVisible() && !kareEnvBuilded)
        updateKareProgressDialog();

    m_pKareEnvProgressDialog->setValue(progress);
}

void kreInstall::setInstallThread(QString debPath)
{
    change_installapp = debPath;
    if (SystemInfoUtil::isOsTree)
    {
        m_pKylinInstalldbusCallThread.reset(new KylinInstalldbusCallThread(change_installapp,m_appName,m_application,m_applicationZh,m_appVersion));
        connect(m_pKylinInstalldbusCallThread.get(), &KylinInstalldbusCallThread::kylinOsinstallStatus, \
        this, &kreInstall::osInstallOverSoftwareAptSlots);
        connect(m_pKylinInstalldbusCallThread.get(), &KylinInstalldbusCallThread::kylinOsinstalldingStatus, \
        this, &kreInstall::InstalldingStatus);
        connect(m_pKylinInstalldbusCallThread.get(), &KylinInstalldbusCallThread::kylinKareEnvInstallStatus, \
        this, &kreInstall::kareBaseEnvInstallStatusSlot);
        connect(m_pKylinInstalldbusCallThread.get(), &KylinInstalldbusCallThread::kylinKareEnvInstallingStatus, \
        this, &kreInstall::kareBaseEnvInstallingStatusSlot);
    }
    else
    {
        PackageEnum::Type packageType(PackageEnum::Type::OTHERS);
        QString packageDistribution;
        bool isInstalledKareBaseEnv = false;
        PackageUtils::getPackageEnumInfo(change_installapp, packageType, packageDistribution, isInstalledKareBaseEnv);
        if (packageType == PackageEnum::Type::KARE)
        {
            if (!isInstalledKareBaseEnv) {
                if (!confirmInstallKareEnvMessageBox())
                {
                    osInstallOverSoftwareAptSlots(false,"取消安装");
                    return ;
                }
                if (packageDistribution.isEmpty()) {
                    osInstallOverSoftwareAptSlots(false,"未找到适配的kare环境");
                    return ;
                }

                m_pKylinInstalldbusCallThread.reset(new KylinInstalldbusCallThread(change_installapp,packageDistribution,PackageEnum::Type::KARE_ENV));
                connect(m_pKylinInstalldbusCallThread.get(), &KylinInstalldbusCallThread::kylinKareEnvInstallStatus, \
                this, &kreInstall::kareBaseEnvInstallStatusSlot);
                connect(m_pKylinInstalldbusCallThread.get(), &KylinInstalldbusCallThread::kylinKareEnvInstallingStatus, \
                this, &kreInstall::kareBaseEnvInstallingStatusSlot);
                setKareVersionShow();
                m_pPackageInstallLabelWidget->hide();
                m_pKareInstallEnvLabelWidget->show();
                kareEnvResetConnect();
            }
            else
            {
                m_pKylinInstalldbusCallThread.reset(new KylinInstalldbusCallThread(change_installapp,packageDistribution,packageType));
                connect(m_pKylinInstalldbusCallThread.get(), &KylinInstalldbusCallThread::kylinOsinstallStatus, \
                this, &kreInstall::osInstallOverSoftwareAptSlots);
                connect(m_pKylinInstalldbusCallThread.get(), &KylinInstalldbusCallThread::kylinOsinstalldingStatus, \
                this, &kreInstall::InstalldingStatus);
                /*重设安装到系统的界面数据*/
                m_pOsDebMainWidget->resetAppInfo(m_appName, m_appVersion+tr("(kare compatible installation)"));
                m_pOsDebInstallIngWidget->resetAppInfo(m_appName, m_appVersion+tr("(kare compatible installation)"));

                /*重设安装到兼容环境的界面数据*/
                m_pdebWidget->resetAppInfo(m_appName, m_appVersion+tr("(kare compatible installation)"));
                m_pdebInstallIngWidget->resetAppInfo(m_appName, m_appVersion+tr("(kare compatible installation)"));
            }
        }
        else
        {
            m_pKylinInstalldbusCallThread.reset(new KylinInstalldbusCallThread(change_installapp,m_appName,m_application,m_applicationZh,m_appVersion));
            connect(m_pKylinInstalldbusCallThread.get(), &KylinInstalldbusCallThread::kylinOsinstallStatus, \
                this, &kreInstall::osInstallOverSoftwareAptSlots);
            connect(m_pKylinInstalldbusCallThread.get(), &KylinInstalldbusCallThread::kylinOsinstalldingStatus, \
                this, &kreInstall::InstalldingStatus);
        }
    }
    m_pKylinInstalldbusCallThread->start();
    m_pAllStackedWidget->setCurrentIndex(5);
}

void kreInstall::initKareProgressDialog()
{
    m_pKareEnvProgressDialog = new KProgressDialog(tr("Initializing compatible environment"));
    m_pKareEnvProgressDialog->setBarType(BarType::NothingBar); // ScrollBar, NothingBar, NormalBar, SpeedBar
    m_pKareEnvProgressDialog->setFixedSize(376, 93);
    m_pKareEnvProgressDialog->setProperty("useStyleWindowManager", false);
    m_pKareEnvProgressDialog->setProperty("useSystemStyleBlur", true);
    m_pKareEnvProgressDialog->hide();
}

void kreInstall::updateKareProgressDialog()
{
    m_pKareEnvProgressDialog->show();

    QRect screenRect = QApplication::desktop()->screenGeometry();

    int x = screenRect.right() - m_pKareEnvProgressDialog->width() - 24;
    int y = screenRect.bottom() - m_pKareEnvProgressDialog->height() - 79;

    WindowManager::setGeometry( m_pKareEnvProgressDialog->windowHandle(),  
                                QRect( x, y,
                                       m_pKareEnvProgressDialog->width(),
                                       m_pKareEnvProgressDialog->height()) );

    WindowManager::setSkipTaskBar (m_pKareEnvProgressDialog->windowHandle(),  true);
}

void kreInstall::kareEnvResetConnect()
{
    if (!resetCloseFlag)
    {
        qDebug()<<"set kare env install";
        disconnect(m_ptitleWidget->m_pClostBtn,SIGNAL(clicked(bool)),this,SLOT(closeBtnClicked(bool)));
        connect(m_ptitleWidget->m_pClostBtn,SIGNAL(clicked(bool)),this,SLOT(hide()));
        connect(m_pKareInstallEnvCancelPushButton,SIGNAL(clicked(bool)),this,SLOT(hide()));
        resetCloseFlag = true;
    }
}

void kreInstall::kareEnvRestoreConnect()
{
    if (resetCloseFlag)
    {
        qDebug()<<"recover kare env install";
        disconnect(m_ptitleWidget->m_pClostBtn,SIGNAL(clicked(bool)),this,SLOT(hide()));
        disconnect(m_pKareInstallEnvCancelPushButton,SIGNAL(clicked(bool)),this,SLOT(hide()));
        connect(m_ptitleWidget->m_pClostBtn,SIGNAL(clicked(bool)),this,SLOT(closeBtnClicked(bool)));
        resetCloseFlag = false;
    }
}


bool kreInstall::confirmInstallKareEnvMessageBox()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText(tr("The version is not compatible. You are about to continue with a compatible environment installation. Do you want to continue? "));
    msgBox.setInformativeText(tr("The KARE compatible environment ensures that the software runs normally on the current system by simulating the environment required by the software. It may take some time to initialize the compatible environment, please ensure that the system is always connected to the internet. "));
    QPushButton *continueButton = new QPushButton(tr("Continue"));
    QPushButton *cancelButton = new QPushButton(tr("Cancel"));
    msgBox.addButton(cancelButton, QMessageBox::RejectRole);
    msgBox.addButton(continueButton, QMessageBox::AcceptRole);
    int ret = msgBox.exec();
    if (msgBox.clickedButton() == cancelButton || msgBox.clickedButton() == nullptr) {
        return false;
    }
    return true;
}

void kreInstall::setKareVersionShow()
{
    m_pOsDebMainWidget->resetAppInfo(m_appName, m_appVersion+tr("(kare compatible installation)"));
    m_pOsDebInstallIngWidget->resetAppInfo(m_appName, m_appVersion+tr("(kare compatible installation)"));

    /*重设安装到兼容环境的界面数据*/
    m_pdebWidget->resetAppInfo(m_appName, m_appVersion+tr("(kare compatible installation)"));
    m_pdebInstallIngWidget->resetAppInfo(m_appName, m_appVersion+tr("(kare compatible installation)"));
}

void kreInstall::playSoundEffects()
{
    QDBusInterface interface("org.ukui.sound.theme.player",
                            "/org/ukui/sound/theme/player",
                            "org.ukui.sound.theme.player",
                            QDBusConnection::sessionBus());

    if (!interface.isValid()) {
        qDebug() << QDBusConnection::sessionBus().lastError().message()<<QString("Try to play the audio file");
        return ;
    }

    QDBusReply<bool> reply = interface.call("playAlertSound", "complete");

    if(reply.isValid()) {
        bool value = reply.value();
        if(value)
            qDebug() << QString("Client play alert sound = %1").arg(value);
        return ;
    } else 
        qDebug() << "Play alert sound method call failed ! Try to play the audio file";

    return ;
}

void kreInstall::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::ActivationChange)
        if (m_ptitleWidget)
            m_ptitleWidget->focuschangeEvent(this->isActiveWindow());

    QWidget::changeEvent(event);
}