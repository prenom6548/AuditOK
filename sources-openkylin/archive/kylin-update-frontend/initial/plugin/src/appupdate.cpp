#include "appupdate.h"
#include <QPixmap>
#include <stdio.h>
#include <QLocale>
#include <QScrollBar>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
using namespace  std;

#define JSON_FILE_PATH "/usr/share/kylin-update-desktop-config/config/"
#define CONFIG_FILE_PATH "/usr/share/ukui-control-center/upgrade/"

AppUpdateWid::AppUpdateWid(AppAllMsg msg,QWidget *parent):QWidget(parent)
{
        m_updateMutual = UpdateDbus::getInstance(this);
//        m_instance1 = updatedeleteprompt::GetInstance(this);
//        connect(m_instance1,&updatedeleteprompt::updatepromptsolvecancelsignal,this,&AppUpdateWid::updatecancel);
//        connect(m_instance1,&updatedeleteprompt::updateallpromptsolveacceptsignal,this,&AppUpdateWid::updateaccept);
//        connect(m_instance1,&updatedeleteprompt::disupdatedependsolveacceptsignal,this,&AppUpdateWid::disupdateallaccept);
    qRegisterMetaType<AppMsg>("AppMsg"); //注册信号槽类型
    qRegisterMetaType<AppAllMsg>("AppAllMsg"); //注册信号槽类型
    appAllMsg = msg;
    updateAppUi(appAllMsg.name);
    this->setObjectName("AppUpdateWid");
    initConnect();
}

void AppUpdateWid::updateAppUi(QString name)
{
    this->setFixedHeight(60);
    frameLayout = new QHBoxLayout();
    frameLayout->setContentsMargins(0,0,0,0);   //更新信息之间的间距
    frameLayout->setSpacing(0);
    this->setLayout(frameLayout);

    AppFrame = new QFrame(this);  //最外层窗口适配主题样式
    AppFrame->setFrameShape(QFrame::Box);
//    AppFrame->setMinimumSize(QSize(550,0));
    AppFrame->setMaximumSize(QSize(16777215, 16777215));
    frameLayout->addWidget(AppFrame);

    mainVLayout = new QVBoxLayout();
    smallHLayout = new QHBoxLayout();
    largeVLayout = new QVBoxLayout();
    appTitleWid = new QWidget();
    largeWidget = new QWidget();

    iconNameLayout = new QHBoxLayout();
//    appAllMsg = AppAllMsg;

    appIconName = new QLabel(this);
//    appIconName->setMinimumWidth(160);
//    appIconName->setMaximumWidth(400);
    appIconName->setFixedWidth(200);
    appIcon = new QLabel(appIconName);
    appNameLab = new MyLabel(appIconName);
    //    appNameLab->setMinimumWidth(140);
    appIconName->setLayout(iconNameLayout);
    appIcon->setFixedSize(32,32);
//    iconNameLayout->setAlignment(Qt::AlignLeft);
    iconNameLayout->addSpacing(6);
    iconNameLayout->addWidget(appIcon,1);
    iconNameLayout->addSpacing(8);
    iconNameLayout->addWidget(appNameLab,10);
    //    iconNameLayout->addStretch();

    appVersion = new FixLabel(this);
//    appVersion->setMinimumWidth(200);
    appVersion ->setFixedWidth(250);
    //    appVersionIcon = new QLabel(this);
//    appVersionIcon->setFixedSize(16,16);
//    appVersionIcon->setPixmap(QPixmap());
//    appVersion->setMinimumWidth(1400);
    //    QIcon icon = QIcon::fromTheme("dialog-error");
    //    QPixmap pixmap = icon.pixmap(icon.actualSize(QSize(14, 14)));
    //    appVersionIcon->setPixmap(pixmap);
//    appVersion->setStyleSheet("background: #BDD7FD");

    detaileInfo = new KBorderlessButton(this);
    //    detaileInfo->setText(tr("详情"));
    detaileInfo->setText(tr("details"));
    //detaileInfo->setFixedSize(60,30);
//    detaileInfo->adjustSize();
//    detaileInfo->setMaximumWidth(95);

    QPalette palette_datailInfo = detaileInfo->palette();
    palette_datailInfo.setColor(QPalette::ButtonText,QColor(55,144,250));
    detaileInfo->setPalette(palette_datailInfo);

    updateAPPBtn = new QPushButton(this);
    //    updateAPPBtn->setText(tr("更新"));
    updateAPPBtn->setText(tr("Update"));
    updateAPPBtn->setFixedWidth(95);
    //updateAPPBtn->setFixedSize(70,40);
//    updateAPPBtn->adjustSize();


//    versionlabLayout = new QHBoxLayout();  //版本号布局
//    versionlabLayout->setSpacing(0);
//    versionlabLayout->setMargin(0);
//    versionLab = new QLabel(this);
//    versionLab->setMargin(0);
//    versionLab->setMinimumWidth(250);
//    versionLab->setMaximumWidth(500);
//    versionLab->setFixedHeight(60);

    //    versionlabLayout->setAlignment(Qt::AlignLeft);
    //    versionlabLayout->addWidget(appVersionIcon);
    //    versionlabLayout->addWidget(appVersion);
    //    versionLab->setLayout(versionlabLayout);
        QSpacerItem *spaceritem = new QSpacerItem(0,0,QSizePolicy::Expanding,QSizePolicy::Expanding);
        smallHLayout->addWidget(appIconName,0,Qt::AlignLeft);
    //    smallHLayout->addStretch(1);
        smallHLayout->addWidget(appVersion,0,Qt::AlignLeft);
    //    smallHLayout->addStretch(5);
        smallHLayout->addSpacerItem(spaceritem);
        smallHLayout->addWidget(detaileInfo,1);
        smallHLayout->addSpacerItem(spaceritem);
    //    smallHLayout->addStretch(5);
        smallHLayout->addWidget(updateAPPBtn,0,Qt::AlignRight);
    //    smallHLayout->addStretch(1);
    //    smallHLayout->addWidget(otherBtnLab,1);
    //    smallHLayout->setSpacing(0);
        smallHLayout->setContentsMargins(0,0,10,0);
        appTitleWid->setLayout(smallHLayout);


    mainVLayout->addWidget(appTitleWid);
    mainVLayout->setMargin(0);
    mainVLayout->setSpacing(0);
    someInfoEdit = new QTextEdit(this);   //部分changelog  缩略显示
    someInfoEdit->setReadOnly(true);
    someInfoEdit->verticalScrollBar()->setProperty("drawScrollBarGroove" , false);
    QPalette pl = someInfoEdit->palette();
    someInfoEdit->setFixedHeight(120);
    pl.setBrush(QPalette::Base,QBrush(QColor(255,0,0,0)));
    someInfoEdit->setPalette(pl);

    //当前语言环境
    QString locale = QLocale::system().name();
    if (locale == "zh_CN")
        environment=zh_cn;

    updatelogBtn = new QPushButton(this);
    //    updatelogBtn->setText(tr("更新日志"));
    updatelogBtn->setText(tr("Update log"));
    updatelogBtn->setFlat(true);
    updatelog1 = new UpdateLog(this);

    largeVLayout->addWidget(someInfoEdit);
    largeVLayout->addWidget(updatelogBtn,0,Qt::AlignLeft);
    largeVLayout->setSpacing(5);
    largeVLayout->setContentsMargins(50,0,50,5);
    largeWidget->setLayout(largeVLayout);
    //    largeWidget->setFixedHeight(80);
    mainVLayout->addWidget(largeWidget);
    largeWidget->hide();
    //    largeWidget->setFixedHeight(120);
    //    appTitleWid->setFixedHeight(60)
    //    this->setLayout(mainVLayout);
    AppFrame->setLayout(mainVLayout);
    QMap<QString, QString> map = getNameAndIconFromJson(name);
    if (!map.value("name").isNull())
    {
        dispalyName = map.value("name");
        qDebug()<<"map.value.isNull()"<<dispalyName;
    }
    else
    {
        dispalyName = translationVirtualPackage(name);
        qDebug()<<"translationVirtualPackage:"<<dispalyName;
    }
    qDebug()<<"appnamelab's displayname is"<<dispalyName;
    appNameLab->setText(dispalyName);
    /*判断图标，优先级: JSON文件指定 > qrc资源文件中 > 主题 > 默认*/
    if (!map.value("icon").isNull()) {
        haveThemeIcon = true;
        appIcon->setPixmap(QPixmap(map.value("icon")));
    } else if (name.contains("kylin-update-desktop-")||name == "linux-generic") {
        haveThemeIcon = true;
        pkgIconPath = QString(":/img/upgrade/%1.png").arg(name);
        appIcon->setPixmap(QPixmap(pkgIconPath));
    } else if (QIcon::fromTheme(name).hasThemeIcon(name)) {    //判断是否有主题图标并输出
        haveThemeIcon = true;
        QIcon icon = QIcon::fromTheme(name);
        QPixmap pixmap = icon.pixmap(icon.actualSize(QSize(32, 32)));
        appIcon->setPixmap(pixmap);
    } else {
        QIcon icon = QIcon::fromTheme("application-x-desktop");
        QPixmap pixmap = icon.pixmap(icon.actualSize(QSize(32, 32)));
        appIcon->setPixmap(pixmap);
    }
    QString newStrMsg = appAllMsg.availableVersion;

    if(newStrMsg.size()>16)
    {
        //        appVersion->setText(tr("最新：")+newStrMsg);
        //        appVersion->setToolTip(tr("最新：")+newStrMsg);
        appVersion->setText(tr("Version:")+newStrMsg);
        appVersion->setToolTip(tr("Version:")+newStrMsg);
    }
    else
    {
        //        appVersion->setText(tr("最新：")+newStrMsg);
        appVersion->setText(tr("Version:")+newStrMsg);
        appVersion->setToolTip("");
    }

    //获取并输出changelog
    chlog = setDefaultDescription(appAllMsg.longDescription);
    updatelog1->logContent->append(chlog);
//    detaillog->logContent->append(chlog);

    QTextCursor tmpCursor = updatelog1->logContent->textCursor();
    tmpCursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    updatelog1->logContent->setTextCursor(tmpCursor);

    //    updatelog1->logAppName->setText(dispalyName+tr("更新日志"));
    //    updatelog1->logAppVerson->setText(tr("最新:")+appAllMsg.availableVersion);
    updatelog1->logAppName->setText(dispalyName+tr("Update log"));
    updatelog1->logAppVerson->setText(tr("Version:")+appAllMsg.availableVersion);

    //获取并输出description
    description = setDefaultDescription(appAllMsg.shortDescription);

    QFontMetrics fontWidth(someInfoEdit->font());//得到每个字符的宽度
    QString StrMsg = fontWidth.elidedText(description, Qt::ElideRight,600);//最大宽度
    someInfoEdit->append(StrMsg);

    someInfoEdit->append(tr("Download size:")+QString(modifySizeUnit(appAllMsg.msg.allSize)));
    someInfoEdit->append(tr("Install size:")+QString(modifySizeUnit(appAllMsg.msg.installSize)));
    if (!map.value("icon").isNull()) {
        haveThemeIcon = true;
        updatelog1->logAppIcon->setPixmap(QPixmap(map.value("icon")));
    } else if(name.contains("kylin-update-desktop")||name == "linux-generic") {
        pkgIconPath = QString(":/img/upgrade/%1.png").arg(name);
        updatelog1->logAppIcon->setPixmap(QPixmap(pkgIconPath));
    } else {
        if(QIcon::fromTheme(name).hasThemeIcon(name))
        {
            QIcon icon = QIcon::fromTheme(name);
            QPixmap pixmap = icon.pixmap(icon.actualSize(QSize(32, 32)));
            updatelog1->logAppIcon->setPixmap(pixmap);
        }
        else
        {
            QIcon icon = QIcon::fromTheme("application-x-desktop");
            QPixmap pixmap = icon.pixmap(icon.actualSize(QSize(32, 32)));
            updatelog1->logAppIcon->setPixmap(pixmap);
        }
    }
    //获取并输出包大小、当前版本号
    QString currentVersion = appAllMsg.version;
    if(currentVersion != "")
    {
        //        someInfoEdit->append(tr("当前版本：")+currentVersion);
        someInfoEdit->append(tr("Current version:")+currentVersion);
    }
}


AppUpdateWid::~AppUpdateWid()
{
    qDebug() << "delete->"<< appAllMsg.name;

}

void AppUpdateWid::initConnect()
{
    connect(detaileInfo,&QPushButton::clicked,this,&AppUpdateWid::showDetails);
    connect(updatelogBtn,&QPushButton::clicked,this,&AppUpdateWid::showUpdateLog);
    connect(updateAPPBtn,&QPushButton::clicked,this,&AppUpdateWid::cancelOrUpdate);
}

/* remove enter */
void AppUpdateWid::remove_last_enter(char *p_src_in_out)
{
    if (p_src_in_out == NULL) {
        return;
    }

    char *p_tmp = p_src_in_out + strlen(p_src_in_out) - 1;
    if (*p_tmp == '\n') {
        *p_tmp = '\0';
    }

    return;
}

/* analysis config file */
QStringList AppUpdateWid::analysis_config_file(char *p_file_path)
{
    FILE *fd = NULL;
    char p_buf[1024];

    QStringList dst;
    dst.clear();

    memset(p_buf , 0x00 , sizeof(p_buf));

    //printf("Info : p_file_path = [%s]\n" , p_file_path);
    fd = fopen(p_file_path , "r");
    if (fd == NULL) {
        printf("Error : open reboot config file fail\n");
        return dst;
    }

    while (fgets(p_buf , sizeof(p_buf) , fd) != NULL) {
        remove_last_enter(p_buf);
        if (!strlen(p_buf)) {
            memset(p_buf , 0x00 , sizeof(p_buf));
            continue;
        }

        //printf("Info : config file data [%s]\n" , p_buf);

        dst << QString(p_buf);

        memset(p_buf , 0x00 , sizeof(p_buf));
    }

    return dst;
}

//修改界面状态值
void AppUpdateWid::showInstallStatues(QStringList appAptName,int progress,QString status,QString detailinfo)
{
    qDebug() << appAptName[0];
    if(QString::compare(appAllMsg.name,appAptName[0]) == 0)
    {
        /* 临时解决方案 , 获取系统语言环境 , 英文加悬浮框 , 中文不加 */
//        m_updateMutual->fileLock();
        int pgs = progress;
        //        appVersion->setText(tr("正在安装")+"("+QString::number(pgs)+"%)");
        if (!isUpdateAll && progress>50 && !isCancel)
        {
            updateAPPBtn->hide();
                //updateAPPBtn->setEnabled(false);
            if(Cancelfail)
            {
                appVersion->setText(tr("Cancel failed,Being installed"));
                return;
            }
            int cha = progress - 50;
            int percent = cha * 2;
            appVersion->setText(tr("Being installed")+"("+QString("%1").arg(percent)+"%)");
        }
    }
}

void AppUpdateWid::showDownloadFinsih(bool state,QStringList pkgname,QString error,QString reason)
{
    QLocale locale;
    char p_path[1024];
    memset(p_path , 0x00 , sizeof(p_path));
    sprintf(p_path , "%s%s" , CONFIG_FILE_PATH , "kylin-need-reboot-new.conf");
    QStringList reboot = analysis_config_file(p_path);
    qDebug() << "Info : need reboot pkg :" << reboot;

    memset(p_path , 0x00 , sizeof(p_path));
    sprintf(p_path , "%s%s" , CONFIG_FILE_PATH , "kylin-need-logout-new.conf");
    QStringList logout = analysis_config_file(p_path);
    qDebug() << "Info : need logout pkg :" << logout;
    if(QString::compare(appAllMsg.name,pkgname[0]) == 0)
    {
        if(state)
        {
            downloadstart = false;
            Cancelfail=false;
            updateAPPBtn->hide();
//            m_updateMutual->fileUnLock();
            //            appVersion->setText(tr("更新成功！"));

            if (reboot.contains(pkgname[0])) {
                if (locale.language() == QLocale::Chinese) {
                    appVersion->setText(tr("Update succeeded , It is recommended that you restart later!"));
                } else {
                    appVersion->setText(tr("Update succeeded , It is recommended that you restart later!"));
                    appVersion->setToolTip(tr("Update succeeded , It is recommended that you restart later!"));
                }
            } else if (logout.contains(pkgname[0])) {
                if (locale.language() == QLocale::Chinese) {
                    appVersion->setText(tr("Update succeeded , It is recommended that you log out later and log in again!"));
                } else {
                    appVersion->setText(tr("Update succeeded , It is recommended that you log out later and log in again!"));
                    appVersion->setToolTip(tr("Update succeeded , It is recommended that you log out later and log in again!"));
                }
            }
            else
            {
                QDBusInterface upgrade_interface( "com.kylin.systemupgrade",
                                             "/com/kylin/systemupgrade",
                                             "com.kylin.systemupgrade.interface",
                                             QDBusConnection::systemBus() );
                QDBusMessage modelist = upgrade_interface.call("GetConfigValue","InstallMode","shutdown_install");
                QString  mode =modelist.arguments().value(1).toString();
                qDebug()<<mode;
                if(mode =="True")
                {
                    appVersion->setText(tr("Download succeeded!"));
                }
                else
                {
                    appVersion->setText(tr("Update succeeded!"));
                }
            }

//            QIcon icon = QIcon::fromTheme("ukui-dialog-success");
//            QPixmap pixmap = icon.pixmap(icon.actualSize(QSize(16, 16)));
//            appVersionIcon->setPixmap(pixmap);
            m_updateMutual->importantList.removeOne(appAllMsg.name);
            m_updateMutual->failedList.removeOne(appAllMsg.name);
            //            QString message = QString("%1"+tr("更新成功！")).arg(dispalyName);
            QString message = QString("%1"+tr("Update succeeded!")).arg(dispalyName);
            m_updateMutual->onRequestSendDesktopNotify(message);
            detaileInfo->hide();
            largeWidget->hide();
            emit oneappUpdateResultSignal(true,pkgname,"","");
        }
        else
        {
//            m_updateMutual->fileUnLock();
            if(isCancel)
            {
                appVersion->setText(tr("Update has been canceled!"));
                updateAPPBtn->show();//是否需要加入updateAPPBtn==更新？
                isCancel=false;
                emit appupdateiscancel();
            }else{
            Cancelfail=false;
            detaileInfo->hide();
//            updateAPPBtn->hide();
            updateAPPBtn->show();
            updateAPPBtn->setText(tr("Update"));
            //            appVersion->setText(tr("更新失败！"));
            appVersion->setText(tr("Update failed!"));
            //            appVersion->setToolTip(tr("失败原因：")+(appNameLab->dealMessage(errormsg)));
            appVersion->setToolTip(tr("Failure reason:")+ "\r\n"+(appNameLab->dealMessage(error)));
            m_updateMutual->importantList.removeOne(appAllMsg.name);
            m_updateMutual->failedList.append(appAllMsg.name);
//            QIcon icon = QIcon::fromTheme("dialog-error");
//            QPixmap pixmap = icon.pixmap(icon.actualSize(QSize(16, 16)));
//            appVersionIcon->setPixmap(pixmap);
            //            QString message = QString("%1"+tr("更新失败！")).arg(dispalyName);
            QString message = QString("%1"+tr("Update failed!")).arg(dispalyName);
            m_updateMutual->onRequestSendDesktopNotify(message);
            emit oneappUpdateResultSignal(false,pkgname,error,reason);
            }
      }
        disconnect(m_updateMutual->interface,SIGNAL(UpdateDependResloveStatus(bool,bool,QStringList,QStringList,QStringList,QString,QString)),this,SLOT(OneAppDependResloveResult(bool,bool,QStringList,QStringList,QStringList,QString,QString)));
        disconnect(m_updateMutual->interface,SIGNAL(UpdateInstallFinished(bool,QStringList,QString,QString)),this,SLOT(showInstallFinsih(bool,QStringList,QString,QString)));
        disconnect(m_updateMutual->interface,SIGNAL(UpdateDownloadFinished(bool,QStringList,QString,QString)),this,SLOT(showDownloadFinsih(bool,QStringList,QString,QString)));
        disconnect(m_updateMutual->interface,SIGNAL(UpdateDloadAndInstStaChanged(QStringList,int,QString,QString)),this,SLOT(showInstallStatues(QStringList,int,QString,QString)));
    }
}

void AppUpdateWid::showInstallFinsih(bool state,QStringList pkgname,QString error,QString reason)
{
    QLocale locale;
    char p_path[1024];
    memset(p_path , 0x00 , sizeof(p_path));
    sprintf(p_path , "%s%s" , CONFIG_FILE_PATH , "kylin-need-reboot-new.conf");
    QStringList reboot = analysis_config_file(p_path);
    qDebug() << "Info : need reboot pkg :" << reboot;

    memset(p_path , 0x00 , sizeof(p_path));
    sprintf(p_path , "%s%s" , CONFIG_FILE_PATH , "kylin-need-logout-new.conf");
    QStringList logout = analysis_config_file(p_path);
    qDebug() << "Info : need logout pkg :" << logout;
    if(QString::compare(appAllMsg.name,pkgname[0]) == 0)
    {
        if(state)
        {
            downloadstart = false;
            Cancelfail=false;
            updateAPPBtn->hide();
//            m_updateMutual->fileUnLock();
            //            appVersion->setText(tr("更新成功！"));

            if (reboot.contains(pkgname[0])) {
                if (locale.language() == QLocale::Chinese) {
                    appVersion->setText(tr("Update succeeded , It is recommended that you restart later!"));
                } else {
                    appVersion->setText(tr("Update succeeded , It is recommended that you restart later!"));
                    appVersion->setToolTip(tr("Update succeeded , It is recommended that you restart later!"));
                }
            } else if (logout.contains(pkgname[0])) {
                if (locale.language() == QLocale::Chinese) {
                    appVersion->setText(tr("Update succeeded , It is recommended that you log out later and log in again!"));
                } else {
                    appVersion->setText(tr("Update succeeded , It is recommended that you log out later and log in again!"));
                    appVersion->setToolTip(tr("Update succeeded , It is recommended that you log out later and log in again!"));
                }
            } else {
                appVersion->setText(tr("Update succeeded!"));
            }

//            QIcon icon = QIcon::fromTheme("ukui-dialog-success");
//            QPixmap pixmap = icon.pixmap(icon.actualSize(QSize(16, 16)));
//            appVersionIcon->setPixmap(pixmap);
            m_updateMutual->importantList.removeOne(appAllMsg.name);
            m_updateMutual->failedList.removeOne(appAllMsg.name);
            //            QString message = QString("%1"+tr("更新成功！")).arg(dispalyName);
            QString message = QString("%1"+tr("Update succeeded!")).arg(dispalyName);
            m_updateMutual->onRequestSendDesktopNotify(message);
            detaileInfo->hide();
            largeWidget->hide();
            emit oneappUpdateResultSignal(true,pkgname,"","");
        }
        else
        {
//            m_updateMutual->fileUnLock();
            if(isCancel)
            {
                appVersion->setText(tr("Update has been canceled!"));
                updateAPPBtn->show();//是否需要加入updateAPPBtn==更新？
                isCancel=false;
                emit appupdateiscancel();
            }else{
            Cancelfail=false;
            detaileInfo->hide();
//            updateAPPBtn->hide();
            updateAPPBtn->show();
            updateAPPBtn->setText(tr("Update"));
            //            appVersion->setText(tr("更新失败！"));
            appVersion->setText(tr("Update failed!"));
            //            appVersion->setToolTip(tr("失败原因：")+(appNameLab->dealMessage(errormsg)));
            appVersion->setToolTip(tr("Failure reason:")+ "\r\n"+(appNameLab->dealMessage(error)));
            m_updateMutual->importantList.removeOne(appAllMsg.name);
            m_updateMutual->failedList.append(appAllMsg.name);
//            QIcon icon = QIcon::fromTheme("dialog-error");
//            QPixmap pixmap = icon.pixmap(icon.actualSize(QSize(16, 16)));
//            appVersionIcon->setPixmap(pixmap);
            //            QString message = QString("%1"+tr("更新失败！")).arg(dispalyName);
            QString message = QString("%1"+tr("Update failed!")).arg(dispalyName);
            m_updateMutual->onRequestSendDesktopNotify(message);
            emit oneappUpdateResultSignal(false,pkgname,error,reason);
            }
      }
        disconnect(m_updateMutual->interface,SIGNAL(UpdateDependResloveStatus(bool,bool,QStringList,QStringList,QStringList,QString,QString)),this,SLOT(OneAppDependResloveResult(bool,bool,QStringList,QStringList,QStringList,QString,QString)));
        disconnect(m_updateMutual->interface,SIGNAL(UpdateInstallFinished(bool,QStringList,QString,QString)),this,SLOT(showInstallFinsih(bool,QStringList,QString,QString)));
        disconnect(m_updateMutual->interface,SIGNAL(UpdateDloadAndInstStaChanged(QStringList,int,QString,QString)),this,SLOT(showInstallStatues(QStringList,int,QString,QString)));
    }
}

void AppUpdateWid::OneAppDependResloveResult(bool ResloveStatus,bool ResloveResult,QStringList DeletePkgList, QStringList DeletePkgDeslist,QStringList DeletePkgReasonlist,QString Error,QString Reason)
{
    if(!ResloveStatus)
    {
        qDebug()<<Error<<Reason;
        QMessageBox msgBox(qApp->activeModalWidget());
        msgBox.setText(tr("There are unresolved dependency conflicts in this update，Please select update all"));
        msgBox.setWindowTitle(tr("Prompt information"));
        msgBox.setIcon(QMessageBox::Icon::Information);
        msgBox.addButton(tr("Update ALL"), QMessageBox::YesRole);
        msgBox.addButton(tr("Cancel"), QMessageBox::NoRole);

        int ret = msgBox.exec();
        switch (ret) {
        case 0:
            qDebug() << "全部更新。。。。。。";
            emit changeupdateall();
            break;
        case 1:
            qDebug() << "取消更新";
            dependencyconfictupdatecancel();
            break;
        default:
            dependencyconfictupdatecancel();
            break;
        }

    }else{
        if (!ResloveResult)
        {
                distUpgradePartial(true);
        }else{
            oneappshowDependSlovePtompt(DeletePkgList,DeletePkgDeslist,DeletePkgReasonlist);
        }
    }
    disconnect(m_updateMutual->interface,SIGNAL(UpdateDependResloveStatus(bool,bool,QStringList,QStringList,QStringList,QString,QString)),this,SLOT(OneAppDependResloveResult(bool,bool,QStringList,QStringList,QStringList,QString,QString)));

}

void AppUpdateWid::oneappshowDependSlovePtompt(QStringList pkgname,QStringList description,QStringList deletereason)
{
    qDebug()<<"the oneappshowDependSlovePtompt's pkg name is"<<pkgname;
    int mode = 2;
    dependfixdialog1 =  new dependencyfixdialog(mode);
    connect(dependfixdialog1,&dependencyfixdialog::updatedependsolvecancelsignal,this,&AppUpdateWid::dependencyfixdialogcancel);
    connect(dependfixdialog1,&dependencyfixdialog::updatedependsolveacceptsignal,this,&AppUpdateWid::updateaccept);
    connect(dependfixdialog1,&dependencyfixdialog::closedpendencyfixdialog,this,&AppUpdateWid::updatecancel);
    connect(dependfixdialog1,SIGNAL(updatedependshowdetailssignal(QStringList,QStringList,QStringList,int)),this,SLOT(showdetaillist(QStringList,QStringList,QStringList,int)));
//    connect(dependfixdialog1,&dependencyfixdialog::disupdatedependsolveacceptsignal,this,&TabWid::disupdateallaccept);
    int pkgnum = dependfixdialog1->updatedeletepkglist(pkgname,description,deletereason);//更新列表
    dependfixdialog1->pkgname=pkgname;
    dependfixdialog1->description=description;
    dependfixdialog1->deletereason=deletereason;
    QString pkgnamestr = QString::number(pkgnum);
    int UpdatePart=2;

    dependfixdialog1->updatemode=UpdatePart;
    qDebug()<<"app update!!!!!!!!!!!!!!!!";
    dependfixdialog1->attention->setText(tr("There are ")+pkgnamestr + tr(" packages going to be removed,Please confirm whether to accept!"));
    dependfixdialog1->exec();
}

void AppUpdateWid::updateaccept()
{
    distUpgradePartial(true);
    qDebug() << "accept distUpgradePartial! ";
}

void AppUpdateWid::dependencyconfictupdatecancel()//全盘更新点击取消后闪退的问题
{
//    int flag=dependfixdialog1->versionlabelflag;
//    if(!flag)
//    {
        appVersion->setText(tr("Update has been canceled!"));
        updateAPPBtn->setText(tr("Update"));
//    updateAPPBtn->adjustSize();
        updateAPPBtn->show();
        detaileInfo->show();
        isCancel=false;
        emit appupdateiscancel();
        qDebug()<<"app完事啦，该tabwidget了";
//    }
//    else
//    {
//         updateAPPBtn->setText(tr("Update"));
//                 updateAPPBtn->show();
//         detaileInfo->hide();
//         emit appupdateiscancel();
//    }
}

void AppUpdateWid::updatecancel()
{
    qDebug()<<"get signal updatedependsolvecancelsignal";
    qDebug()<<"updatecancel";
    int flag=dependfixdialog1->versionlabelflag;
    if(!flag)
    {
        appVersion->setText(tr("Update has been canceled!"));
        updateAPPBtn->setText(tr("Update"));
//    updateAPPBtn->adjustSize();
        updateAPPBtn->show();
        detaileInfo->show();
        isCancel=false;
        emit appupdateiscancel();
        qDebug()<<"app完事啦，该tabwidget了";
    }
    else
    {
         updateAPPBtn->setText(tr("Cancel"));
                 updateAPPBtn->show();
         detaileInfo->hide();
         emit appupdateiscancel();
    }
}

void AppUpdateWid::dependencyfixdialogcancel()
{
    qDebug()<<"get signal updatedependsolvecancelsignal";
    qDebug()<<"dependencyfixdialogcancel";
    appVersion->setText(tr("Update has been canceled!"));
    updateAPPBtn->setText(tr("Update"));
//    updateAPPBtn->adjustSize();
    updateAPPBtn->show();
    detaileInfo->show();
//    isCancel=false;
//    emit appupdateiscancel();
    qDebug()<<"app完事啦，该tabwidget了";
}


QMap<QString, QString> AppUpdateWid::getNameAndIconFromJson(QString pkgname)
{
    QMap <QString, QString> nameIconList;
    /*判断json文件是否存在*/
    QString filename = QString(JSON_FILE_PATH) +pkgname +".json";
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)){
        qDebug() << "JSON file open failed! ";
        return nameIconList;
    }
    QByteArray jsonData = file.readAll();

    QJsonParseError err_rpt;
    QJsonDocument  root_Doc = QJsonDocument::fromJson(jsonData, &err_rpt); // 字符串格式化为JSON

    if (!root_Doc.isNull() && (err_rpt.error == QJsonParseError::NoError)) {  // 解析未发生错误
        if (root_Doc.isObject()) { // JSON 文档为对象
            QJsonObject object = root_Doc.object();  // 转化为对象
            if (QLocale::system().name() == "zh_CN"){
                QString name  = object.value("name").toObject().value("zh_CN").toString();
                if (!name.isNull()) {
                    nameIconList.insert("name", name);
                }
            }else {
                QString name  = object.value("name").toObject().value("en_US").toString();
                if (!name.isNull()) {
                    nameIconList.insert("name", name);
                }
            }
            QString iconPath = object.value("icon").toString();
            if (!iconPath.isNull())
                nameIconList.insert("icon", iconPath);
        }
    }else{
        qDebug() << "JSON文件格式错误！";
        return nameIconList;
    }

    return nameIconList;
}

void AppUpdateWid::showDetails()
{
    qDebug()<<"show details in app";
    detaillog = new Detaildialog();
//    detaillog->setAttribute(Qt::WA_DeleteOnClose);
    detaillog->setFixedSize(420,314);
    detaillog->logContent->clear();

    YAML::Node config;
    try
    {
        config= YAML::LoadFile("/usr/share/kylin-update-desktop-config/config/kylin-update-desktop-system.yaml");
    }
    catch(YAML::BadFile)
    {
        return;
    }

    QString appname_lab = appNameLab->text();
    qDebug()<<"yaml 1";
//    YAML::TypedBadConversion
    YAML::Node name = config["name"];
    std::string name_str = name.as<string>();
    QString name_qstring = QString::fromStdString(name_str);
    qDebug()<<"the appname_lab is"<<appname_lab;
    qDebug()<<"the name_qstring is"<<name_qstring;

    YAML::Node changelog = config["changelog"];
    std::string changelog_str = changelog.as<string>();
    description = QString::fromStdString(changelog_str);

//    description = setDefaultDescription(appAllMsg.shortDescription);
//    QFontMetrics fontWidth(detaillog->logContent->font());//得到每个字符的宽度
//    QString StrMsg = fontWidth.elidedText(description, Qt::ElideRight,600);//最大宽度
//    if(appname_lab == name_qstring)
    qDebug()<<"the appAllMsg's name is "<<appAllMsg.name;
    if(name_qstring == appAllMsg.name)
        detaillog->logContent->append(description);//strmsg换description
    else
        detaillog->logContent->append("暂无内容");
//    detaillog->logContent->append(tr("Download size:")+QString(modifySizeUnit(appAllMsg.msg.allSize)));
//    detaillog->logContent->append(tr("Install size:")+QString(modifySizeUnit(appAllMsg.msg.installSize)));
//    detaillog->installContent->setText(tr("Install size:")+QString(modifySizeUnit(appAllMsg.msg.installSize)));
//    qDebug()<<"hhhhhhhhhhhhhhhhhhhhhhhhhhhh"<<appAllMsg.availableVersion;
    detaillog->installContent->setText(tr("version:")+appAllMsg.availableVersion+"  "+QString(modifySizeUnit(appAllMsg.msg.installSize)));
    detaillog->show();
}

void AppUpdateWid::showUpdateLog()
{
    //QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
    //updatelog1->move((availableGeometry.width()-updatelog1->width())/2,(availableGeometry.height()- updatelog1->height())/2);
    updatelog1->exec();
}

void AppUpdateWid::cancelOrUpdate()
{
    if(updateAPPBtn->text() == tr("Update"))
    {
        /*判断电量是否支持更新*/
        if (!get_battery())
        {
            QMessageBox msgBox;
//            msgBox.setText(tr("The battery is below 50% and the update cannot be downloaded"));
            msgBox.setText(tr("The update stopped because of low battery."));
            msgBox.setInformativeText(tr("The system update requires that the battery power is not less than 50%"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setButtonText(QMessageBox::Ok,tr("OK"));
            msgBox.exec();
            return ;
        }
        if (isAutoUpgrade)
        {
            updateOneApp(false);
            return ;
        }
//        if(m_updateMutual->isPointOutNotBackup == true)
//        {
//            QMessageBox msgBox(qApp->activeModalWidget());
//            msgBox.setText(tr("A single update will not automatically backup the system, if you want to backup, please click Update All."));
//            msgBox.setWindowTitle(tr("Prompt information"));
//            msgBox.setIcon(QMessageBox::Icon::Warning);
////            msgBox.setStandardButtons(QMessageBox::YesAll
////                                      | QMessageBox::NoToAll|QMessageBox::Cancel);
////            msgBox.setButtonText(QMessageBox::YesAll, tr("Do not backup, continue to update"));
////            msgBox.setButtonText(QMessageBox::NoToAll, tr("Cancel"));
////            msgBox.setButtonText(QMessageBox::Cancel, tr("Cancel update"));

//            msgBox.addButton(tr("Do not backup, continue to update"), QMessageBox::YesRole);
//            msgBox.addButton(tr("Cancel"), QMessageBox::NoRole);
//            QCheckBox *cb = new QCheckBox(&msgBox);
//            msgBox.setCheckBox(cb);
//            msgBox.checkBox()->setText(tr("This time will no longer prompt"));
//            msgBox.checkBox()->show();

//            int ret = msgBox.exec();
//            if(msgBox.checkBox()->checkState() == Qt::Checked) {
//                m_updateMutual->isPointOutNotBackup = false;
//            }
//            if(ret == 0) {
//                emit changeUpdateAllSignal(true);
//                qDebug() << "立即更新!";
//                updateOneApp(false);
//            } else if(ret == 1) {
//                emit changeUpdateAllSignal(false);
//                m_updateMutual->isPointOutNotBackup = true;
//                qDebug() << "不进行更新。";
//            }
//            qDebug() << "m_updateMutual->isPointOutNotBackup = " << m_updateMutual->isPointOutNotBackup;
//        }
//        else{
//            emit changeUpdateAllSignal(true);
//            updateOneApp(false);
//        }

        //先去掉备份提示弹窗
        emit changeUpdateAllSignal(true);
        updateOneApp(false);
    }
    else {
        updateAPPBtn->setText(tr("Update"));
//        updateAPPBtn->adjustSize();
        detaileInfo->show();
//        QString newStrMsg = appAllMsg.availableVersion;
//        if(newStrMsg.size() > 16) {
//            appVersion->setText(tr("Version:") + newStrMsg);
//            appVersion->setToolTip(tr("Version:") + newStrMsg);
//        } else {
//            appVersion->setText(tr("Version:") + newStrMsg);
//            appVersion->setToolTip("");
//        }

        QDBusReply<bool> reply = m_updateMutual->interface->call("CancelDownload");
        if(!reply.isValid())
        {
            qDebug()<<"取消下载接口异常";
            return ;
        }
        bool cancelreply = reply.value();
        if(cancelreply)
        {
            isCancel = true;
            emit appupdateiscancel();
        }
        else
        {
            Cancelfail = true;  //取消失败
        }
    }
}

void AppUpdateWid::updateOneApp(bool status)
{
    distUpgradePartial(status);

}

void AppUpdateWid::distUpgradePartial(bool status)
{
    if(status)
    {
        connect(m_updateMutual->interface,SIGNAL(UpdateDloadAndInstStaChanged(QStringList,int,QString,QString)),this,SLOT(showInstallStatues(QStringList,int,QString,QString)));
        connect(m_updateMutual->interface,SIGNAL(UpdateDownloadInfo(QStringList,int,int,uint,uint,int)),this,SLOT(showDownloadStatues(QStringList,int,int,uint,uint,int)));
        connect(m_updateMutual->interface,SIGNAL(UpdateInstallFinished(bool,QStringList,QString,QString)),this,SLOT(showInstallFinsih(bool,QStringList,QString,QString)));
        connect(m_updateMutual->interface,SIGNAL(UpdateDownloadFinished(bool,QStringList,QString,QString)),this,SLOT(showDownloadFinsih(bool,QStringList,QString,QString)));
        updateAPPBtn->setText(tr("Cancel"));
//        updateAPPBtn->adjustSize();
        updateAPPBtn->setEnabled(false);
        //updateAPPBtn->setFixedWidth(110);
/*11.25*/
        detaileInfo->hide();

//        appVersionIcon->setPixmap(QPixmap());
        appVersion->setText(tr("Ready to update"));
        emit startoneappupdate();
    }
    else
    {
         connect(m_updateMutual->interface,SIGNAL(UpdateDependResloveStatus(bool,bool,QStringList,QStringList,QStringList,QString,QString)),this,SLOT(OneAppDependResloveResult(bool,bool,QStringList,QStringList,QStringList,QString,QString)));
    }
    QStringList pkgname;
    pkgname.append(appAllMsg.name);
    m_updateMutual->interface->asyncCall("DistUpgradePartial",status,pkgname);
}
//转换包大小的单位
QString AppUpdateWid::modifySizeUnit(long long size)
{
    double tmpSize = size/1024.0;
    if(tmpSize >= 1)
    {
        if((tmpSize/1024.0) >= 1)
        {
            if((tmpSize/1024.0/1024.0) >= 1)
                return QString("%1%2").arg((int)(tmpSize/1024.0/1024.0*100 + 0.5)/100.0).arg("GB");//保留小数点后两位，并四舍五入
            else
                return QString("%1%2").arg((int)(tmpSize/1024.0*100 + 0.5)/100.0).arg("MB");//保留小数点后两位，并四舍五入
        }
        else
            return QString("%1%2").arg((int)(tmpSize*100 + 0.5)/100.0).arg("KB");//保留小数点后两位，并四舍五入
    }
    else
        return QString("%1%2").arg(size).arg("B");
}

//当前下载速度单位换算
QString AppUpdateWid::modifySpeedUnit(long size, float time)
{
    if(size < 0)
        size = 0;
    size = size/time;
    if((size/1024) >= 1)
    {
        if((size/1024/1024) >= 1)
        {
            if((size/1024/1024/1024) >= 1)
                return QString("%1%2").arg((int)(size/1024/1024/1024)).arg("GB/S");
            else
                return QString("%1%2").arg((int)(size/1024/1024)).arg("MB/S");
        }
        else
            return QString("%1%2").arg((int)(size/1024)).arg("KB/S");
    }
    else
        return QString("%1%2").arg(size).arg("B/S");
}

void AppUpdateWid::showDownloadStatues(QStringList pkgname,int currentserial,int total,uint downloadedsize,uint totalsize,int downloadSpeed)
{

    if(downloadSpeed > 0)
        downloadstart=true;
//    connect(m_updateMutual->interface,SIGNAL(Cancelable(bool)),this,SLOT(enableupdateAPPBtn));
    QString downloadspeed = modifySpeedUnit(downloadSpeed, 1);
    if(QString::compare(appAllMsg.name,pkgname[0]) == 0)
    {
        updateAPPBtn->setEnabled(true);
    //    updateAPPBtn->adjustSize();
//        updateAPPBtn->setFixedWidth(110);
        if (!isUpdateAll){
            if((downloadedsize == totalsize) && (totalsize == 0))
            {
                if(isCancel)
                {
                    return;
                }
                else
                {
                    appVersion->setText(tr("downloaded"));
                    return;
                }
            }
            if(downloadSpeed==0 && !downloadstart)
            {
                appVersion->setText(tr("downloading")+":"+"("+tr("calculating")+")"+modifySizeUnit(downloadedsize)+"/"+modifySizeUnit(totalsize));
                return;
            }
            appVersion->setText(tr("downloading")+":"+"("+downloadspeed+")"+modifySizeUnit(downloadedsize)+"/"+modifySizeUnit(totalsize));
//            appVersion->setToolTip("");
        }
    }
    if(currentserial==total)
    {
        disconnect(m_updateMutual->interface,SIGNAL(UpdateDownloadInfo(QStringList,int,int,uint,uint,int)),this,SLOT(showDownloadStatues(QStringList,int,int,uint,uint,int)));
//        disconnect(m_updateMutual->interface,SIGNAL(Cancelable(bool)),this,SLOT(enableupdateAPPBtn));
    }
}

void AppUpdateWid::showUpdateBtn()
{
    if (!isUpdateAll)
        updateAPPBtn->show();
    updateAPPBtn->setText(tr("Update"));
//    updateAPPBtn->adjustSize();
}
void AppUpdateWid::hideOrShowUpdateBtnSlot(int result)
{
    if(result == 0)  //0表示备份还原开始
    {
        updateAPPBtn->hide();
    }
    else if(result == 99 ||result == -20)  //99时备份成功 -20时备份还原进程被中断
    {
        if (!isUpdateAll)
            updateAPPBtn->show();
    }

}

QString AppUpdateWid::translationVirtualPackage(QString str)
{
    if(QLocale::system().name()!="zh_CN")
        return str;
    if(str == "kylin-update-desktop-app")
        return "基本应用";
    if(str == "kylin-update-desktop-security")
        return "安全更新";
    if(str == "kylin-update-desktop-support")
        return "系统基础组件";
    if(str == "kylin-update-desktop-ukui")
        return "桌面环境组件";
    if(str == "linux-generic")
        return "系统内核组件";
    if(str == "kylin-update-desktop-kernel")
        return "系统内核组件";
    if(str == "kylin-update-desktop-kernel-3a4000")
        return "系统内核组件";
    if(str == "kylin-update-desktop-kydroid")
        return "kydroid补丁包";
    return str;
}
bool AppUpdateWid::eventFilter(QObject *watched, QEvent *event)
{
    if(watched->metaObject()->className() == QStringLiteral("QLabel"))
    {
        if(event->type() == QEvent :: ToolTip){
            QToolTip::hideText();
            event->ignore();
            return true;
        }
        else
        {
            return false;
        }
    }
}

QString AppUpdateWid::setDefaultDescription(QString str)
{
    if(str == "")
    {
        //        str = tr("暂无内容");
        str = tr("No content.");
    }
    return str;
}

type AppUpdateWid::checkSourcesType()
{
    QFile soucesFile(SOURCESLIST);
    soucesFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QString result = soucesFile.readAll();
    if(result.contains("http://") || result.contains("https://")){
        qDebug() << "当前源为http源";
        return http;
    }else if(result.contains("ftp://")){
        qDebug() << "当前源为ftp源";
        return ftp;
    }else if(result.contains("file://")){
        qDebug() << "当前源为本地源";
        return file;
    }
    return http;
}

//bool AppUpdateWid::get_battery()
//{
//    QStringList users;
//    int battery_value = 0;
//    QDBusInterface m_interface1( "org.freedesktop.UPower",
//                                 "/org/freedesktop/UPower",
//                                 "org.freedesktop.UPower",
//                                 QDBusConnection::systemBus() );
//    if (!m_interface1.isValid()) {
//        qDebug() << "电源管理器dbus接口初始化失败";
//        return true;
//    }

//    QDBusReply<QList<QDBusObjectPath>> obj_reply = m_interface1.call("EnumerateDevices");

//    if (obj_reply.isValid()) {
//        for (QDBusObjectPath op : obj_reply.value())
//            users << op.path();
//        if (users.size()==1 || users.isEmpty()) {
//            qDebug()<<"无法获取电量值,判断此电脑为台式电脑";
//            return true;
//        }
//        foreach (QString str, users) {
//            if (str == users.at(0) || str == users.at(users.size() - 1)) {
//                continue ;
//            }
//            QDBusInterface m_interface( "org.freedesktop.UPower",
//                                        str,
//                                        "org.freedesktop.DBus.Properties",
//                                        QDBusConnection::systemBus());

//            if (!m_interface.isValid()) {
//                qDebug() << "电源管理器dbus接口初始化失败";
//                return true;
//            }

//            QDBusReply<QVariant> obj_reply = m_interface.call("Get","org.freedesktop.UPower.Device","Percentage");
//            int Ele_surplus = obj_reply.value().toInt();
//            battery_value += Ele_surplus;
//            qDebug() << "battery value : " << Ele_surplus;
//        }
//        return true;
//    }
//    /*如果电池总电量小于50不可升级*/
//    if (battery_value < 50)
//        return false;
//    return true;
//}

void AppUpdateWid::showdetaillist(QStringList pkgname,QStringList description,QStringList deletereason,int updatemode)
{
    qDebug()<<"get the siganl";
    m_instance1 = updatedeleteprompt::GetInstance(this);
    int pkgnum = m_instance1->updatedeletepkglist(pkgname,description,deletereason);
    m_instance1->pkgname=pkgname;
    m_instance1->description=description;
    m_instance1->deletereason=deletereason;
    m_instance1->updatemode=updatemode;

    connect(m_instance1,&updatedeleteprompt::updatedependsolveacceptsignal,this,&AppUpdateWid::updateaccept);//函数没改

    m_instance1->show();

}

bool AppUpdateWid::get_battery()
{
    QStringList users;
    int battery_value = 0;
    QDBusInterface m_interface1( "org.freedesktop.UPower",
                                 "/org/freedesktop/UPower",
                                 "org.freedesktop.UPower",
                                 QDBusConnection::systemBus() );
    if (!m_interface1.isValid())
    {
        qDebug() << "电源管理器dbus接口初始化失败";
        return true;
    }

    QDBusReply<QList<QDBusObjectPath>> obj_reply = m_interface1.call("EnumerateDevices");

    if (obj_reply.isValid())
    {
        qDebug()<<"come into EnumerateDevices";
        for (QDBusObjectPath op : obj_reply.value())
        {
            users << op.path();
        }
        if (users.size()==1 || users.isEmpty())
        {
            qDebug()<<"无法获取电量值,判断此电脑为台式电脑";
            return true;
        }
        foreach (QString str, users)
        {
            QDBusInterface m_interface( "org.freedesktop.UPower",
                                        str,
                                        "org.freedesktop.DBus.Properties",
                                        QDBusConnection::systemBus());

            if (!m_interface.isValid())
            {
                qDebug() << "电源管理器dbus接口初始化失败";
                return true;
            }

            QDBusReply<QVariant> obj_reply = m_interface.call("Get","org.freedesktop.UPower.Device","Percentage");
            qDebug()<<"obj_reply is "<<obj_reply;
            int Ele_surplus = obj_reply.value().toInt();
            qDebug()<<"Ele_surplus is"<<Ele_surplus;
            battery_value += Ele_surplus;
            qDebug() << "battery_value is "<<battery_value;
            qDebug() << "battery value : " << Ele_surplus;
        }
    }
    /*如果电池总电量小于50不可升级*/
    if (battery_value < 50)
    {
        qDebug() << "battery value < 50%:"<<battery_value;
        return false;
    }
    qDebug() << "battery value > 50%";
    return true;
}

//void AppUpdateWid::updateaccept()
//{
//    distUpgradePartial(true);
//    qDebug() << "accept distUpgradePartial! ";
//}
