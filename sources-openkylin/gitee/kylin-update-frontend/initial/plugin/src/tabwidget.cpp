#include "tabwidget.h"
#include <QProcess>
#include "backuptools-define.h"
#include "ukuistylehelper/ukuistylehelper.h"
#include "windowmanager/windowmanager.h"

#define JSON_FILE_PATH "/var/lib/kylin-system-updater/json/"

TabWid::TabWid(QWidget *parent):QWidget(parent)
{
    allComponents();
}

void TabWid::initDbus()
{
    /* 更新管理器dbus接口 */
    updateMutual = UpdateDbus::getInstance();//systemupgrade

    /* 源管理器dbus接口 */
    updateSource = new UpdateSource();//software properties

//    fixbrokeninstall = new fixbrokeninstalldialog();

    QThread *tmp_thread = new QThread;
    connect(tmp_thread,&QThread::started,updateSource,&UpdateSource::startDbus);
    connect(updateSource,&UpdateSource::startDbusFinished,this,&TabWid::dbusFinished);
    updateSource->moveToThread(tmp_thread);
    tmp_thread->start();

    ukscConnect = new UKSCConn();

    /*备份还原初始化*/
    backup = new BackUp;
    backupThread = new QThread;
    backup->moveToThread(backupThread);
    backupThread->start();

    connect(checkUpdateBtn,&QPushButton::clicked,this,&TabWid::checkUpdateBtnClicked);
    connect(this, &TabWid::updateAllSignal, this, &TabWid::updateAllApp);
    connect(historyUpdateLog,&QPushButton::clicked,this,&TabWid::showHistoryWidget);
//    connect(isAutoCheckSBtn,&SwitchButton::checkedChanged,this,&TabWid::isAutoCheckedChanged);
    connect(isAutoCheckSBtn,&KSwitchButton::stateChanged,this,&TabWid::isAutoCheckedChanged);
    connect(isAutoCheckSBtn,&KSwitchButton::stateChanged,this,[=] (bool status){
        buriedSettings(QString("Upgrade"), QString("isAutoCheckSBtn"), QString("settings"),  status ? "true":"false");
    });
//    connect(m_pButtonGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked),this, &TabWid::UpgradeModeChangedSlot);
    //connect(isAutoBackupSBtn,&SwitchButton::checkedChanged,this,&TabWid::isAutoBackupChanged);
    connect(isAutoBackupSBtn,&KSwitchButton::stateChanged,this,&TabWid::isAutoBackupChanged);
//    connect(isAutoUpgradeSBtn, &SwitchButton::checkedChanged, this, &TabWid::isAutoUpgradeChanged);
    connect(isAutoUpgradeSBtn, &KSwitchButton::stateChanged, this, &TabWid::isAutoUpgradeChanged);
    connect(isAutoUpgradeSBtn,&KSwitchButton::stateChanged,this,[=] (bool status){
        buriedSettings(QString("Upgrade"), QString("isAutoUpgradeSBtn"), QString("settings"),  status ? "true":"false");
    });
    connect(updateSource,&UpdateSource::getReplyFalseSignal,this,&TabWid::getReplyFalseSlot);
//    connect(DownloadHBtn,&SwitchButton::checkedChanged,this,&TabWid::DownloadLimitChanged);
    connect(DownloadHBtn,&KSwitchButton::stateChanged,this,&TabWid::DownloadLimitChanged);
    connect(DownloadHBtn,&KSwitchButton::stateChanged,this,[=] (bool status){
        buriedSettings(QString("Upgrade"), QString("DownloadHBtn"), QString("settings"),  status ? "true":"false");
    });
    connect(DownloadHValue,&QComboBox::currentTextChanged,this,&TabWid::DownloadLimitValueChanged);
    connect(updateMutual->interface,SIGNAL(UpdateFixBrokenStatus(bool,bool,QStringList,QStringList,QStringList,QString,QString)),this,SLOT(FixBrokenResloveResult(bool,bool,QStringList,QStringList,QStringList,QString,QString)));

//    /*疑问：该处的connect应该在此处，还是在弹出备份并更新那里呢？*/
//    m_instance = updatedeleteprompt::GetInstance(this);
//    connect(m_instance,&updatedeleteprompt::updatedependsolvecancelsignal,this,&TabWid::updatecancel);
//    connect(m_instance,&updatedeleteprompt::updatealldependsolveacceptsignal,this,&TabWid::updateAllApp);
//    connect(m_instance,&updatedeleteprompt::disupdatedependsolveacceptsignal,this,&TabWid::disupdateallaccept);

    //    connect(dependfixdialog,SIGNAL(updatedependshowdetailssignal(QStringList pkgname,QStringList description,QStringList deletereason)),this,SLOT(showdetaillist(QStringList pkgname,QStringList description,QStringList deletereason)));
    //    bacupInit();//初始化备份
    checkUpdateBtn->stop();
    //    checkUpdateBtn->setText(tr("检查更新"));
    checkUpdateBtn->setText(tr("Check Update"));

}


bool TabWid::buriedSettings(QString pluginName, QString settingsName, QString action, QString value)
{
    // 埋点数据
    qInfo()<<"ukcc buriedSettings";
    char appName[] = "ukui-control-center";
    QByteArray actiontr = action.toLocal8Bit();  // toLocal8Bit 支持中文
    char *messageType = actiontr.data();
    qInfo()<<action;
    KBuriedPoint pt[3];
    pt[0].key = "pluginName";
    std::string pluginStr = pluginName.toStdString();
    pt[0].value = pluginStr.c_str();
    qInfo()<<pluginName;
    pt[1].key = "settingsName";
    std::string settingStr = settingsName.toStdString();
    pt[1].value = settingStr.c_str();
    qInfo()<<settingsName;
    pt[2].key = "value";
    std::string valueStr = value.toStdString();
    qInfo()<<value;
    pt[2].value = valueStr.c_str();
    qInfo()<<value<<"burried value";
    if (kdk_buried_point(appName , messageType , pt , 3) == -1) {
        qInfo() << __FUNCTION__ << "messageType:" << action << "pluginName:" << pluginName
                 << "settingsName:" << settingsName << "value:" << value << "buried point fail !" << __LINE__;
        return false;
    }

    return true;
}

void TabWid::isAutoUpgradeChanged()
{
    bool list;
    if (isAutoUpgradeSBtn->isChecked())
    {
        list = true;
    }
    else
    {
        list = false;
    }
//    updateMutual->SetAutoUpgrade(false,list);
    updateMutual->SetAutoUpgradeState(list);
}


void TabWid::dbusFinished()
{
    /*获取自动更新开关的状态*/
    QString filename = "/var/lib/unattended-upgrades/unattended-upgrades-policy.conf";
    QSettings *autoupgrade_allow = new QSettings(filename,QSettings::IniFormat);
    QString autoUpgradeState = autoupgrade_allow->value("autoUpgradePolicy/autoUpgradeState","off").toString();
    if(!autoUpgradeState.compare("off"))
    {
        isAutoUpgradeSBtn->setChecked(false);
    }
    else if(!autoUpgradeState.compare("on"))
    {
        isAutoUpgradeSBtn->setChecked(true);
    }
//    QStringList list;
//    QDBusPendingReply<QString> list;
//    QString list = updateMutual->GetAutoUpgrade(true);
//    list << "CONTROL_CENTER/autoupdate_allow";
//    QString ret =  updateSource->getOrSetConf("get", list);
//    QString ret = list.value();
//    if (!list.compare("false"))
//        isAutoUpgradeSBtn->setChecked(false);
//    else
//        isAutoUpgradeSBtn->setChecked(true);

    /*获取当前自动更新的状态*/
    //getAutoUpgradeStatus();
    checkUpdateBtnClicked();
}

void TabWid::getAutoUpgradeStatus()
{
//    fileLock();
    QStringList list;
    list << "CONTROL_CENTER/autoupdate_run_status";
    QString ret =  updateSource->getOrSetConf("get", list);

    if (!ret.compare("backup")) {
        isAutoUpgrade = true;
        /*如果自动更新在备份中，那就直接绑定备份还原信号即可*/
        isAllUpgrade = true;
        bool ret = autoUpdateLoadUpgradeList(true);
        if (ret) {
            bacupInit(true);
            backup->creatInterface();
            backup->setProgress = true;
            backupProgress(0);
        } else {
            bacupInit(false);
        }
    } else if (!ret.compare("download")) {
        /*如果自动更新在下载中，调用dbus去kill掉下载程序，继续原流程，不进行多余操作*/
        QFile file("/var/run/apt-download.pid");
        QString pid;
        if (file.open(QIODevice::ReadOnly)) {
            pid = file.readAll();
        }
        file.close();
        updateMutual->killProcessSignal(pid.toInt(), 10);
        checkUpdateBtn->setEnabled(true);
        checkUpdateBtn->setText(tr("Check Update"));
        checkUpdateBtnClicked();
    } else if (!ret.compare("install")){
        isAutoUpgrade = true;
        /*如果自动更新在安装中，绑定更新管理器dbus接收信号即可*/
        isAllUpgrade = true;
        checkUpdateBtn->hide();
        checkUpdateBtn->setText(tr("UpdateAll"));
        checkUpdateBtn->adjustSize();
        bool ret = autoUpdateLoadUpgradeList(false);
        if (!ret)
            updateMutual->disconnectDbusSignal();
    } else if (!ret.compare("idle")) {
        /*如果没有进行自动更新，那就不需要操作 */
        checkUpdateBtn->setEnabled(true);
        checkUpdateBtn->setText(tr("Check Update"));
        checkUpdateBtnClicked();
    } else {
        /*如果读不到，默认也不进行操作*/
        checkUpdateBtn->setEnabled(true);
        checkUpdateBtn->setText(tr("Check Update"));
        checkUpdateBtnClicked();
    }
}

bool TabWid::autoUpdateLoadUpgradeList(bool isBackUp)
{
    QSettings get("/var/lib/kylin-auto-upgrade/kylin-autoupgrade-pkglist.conf", QSettings::IniFormat);
    QString str;
    if (isBackUp)
        str = get.value("DOWNLOAD/pkgname").toString();
    else
        str = get.value("DOWNLOAD/uninstpkg").toString();
    qDebug() << "----------pkgname---->" << str;

    if (str.isNull())
    {
        version = getversion();
        versionInformationLab->setText(tr("Your system is the latest:")+version);
//        versionInformationLab->setToolTip("");
//        QString updatetime = tr("No Information!");
//        QSqlQuery queryInstall(QSqlDatabase::database("A"));
//        queryInstall.exec("select * from updateinfos order by id desc");
//        while (queryInstall.next()) {
//            QString statusType = queryInstall.value("keyword").toString();
//            if (statusType == "" || statusType =="1") {
//                updatetime = queryInstall.value("date").toString();
//                break;
//            }
//        }
//        lastRefreshTime->setText(tr("Last refresh:")+ updatetime);
        QString checkedtime = tr("No information!");
        QSqlQuery query(QSqlDatabase::database("A"));
        query.exec("select * from display");
        while(query.next())
        {
            checkedtime = query.value("check_time").toString();
        }
        lastRefreshTime->setText(tr("Last Checked:")+checkedtime);
        lastRefreshTime->show();
        checkUpdateBtn->setText(tr("Check Update"));
        checkUpdateBtn->hide();
        return false;
    } else {
        QStringList list;
        if(str.contains(" ")) {
            list = str.split(" ");
        } else {
            list << str;
        }
        versionInformationLab->setText(tr("Downloading and installing updates..."));
        lastRefreshTime->hide();
        allProgressBar->show();
        allProgressBar->setValue(10);
        updateMutual->getAppMessage(list);
        return true;
    }
}

TabWid::~TabWid()
{
    qDebug() << "~TabWid" ;
//    delete updateMutual;
//    updateMutual = nullptr;
//    backupDelete();//回收资源


//    /*析构取消apt update命令*/因为会导致控制面板打不开+影响自动下载，暂时取消该段代码
//    QDBusReply<qint32> obj_reply = updateMutual->interface->call("GetBackendStatus",getLanguageEnv());
//    int backendStatus = obj_reply.value();
//    qDebug()<<"get backendstatus================="<<backendStatus;
//    switch(backendStatus)
//    {
//        qDebug()<<"backendStatus is "<<backendStatus;
//        case ACTION_DEFUALT_STATUS:
//        break;
//        case ACTION_UPDATE:
//            updateMutual->interface->call("CancelDownload");
////            versionInformationLab->setText(tr("The progress is updating..."));
////            checkUpdateBtn->hide();
////            connect(updateMutual->interface,SIGNAL(UpdateDetectFinished(bool,QStringList,QString,QString)),this,SLOT(slotUpdateCache(bool,QStringList,QString,QString)));
////            connect(updateMutual->interface,SIGNAL(UpdateDetectStatusChanged(int,QString)),this,SLOT(slotUpdateCacheProgress(int,QString)));
//            return;
//        break;
//        case ACTION_INSTALL:
//        break;
//        default:
//        break;
//    }
    delete updateMutual;
    updateMutual = nullptr;
    backupDelete();//回收资源
//    fileUnLock();
}

void TabWid::backupMessageBox(QString str)
{
    QMessageBox msgBox;
    msgBox.setText(str);
    msgBox.addButton(tr("Keeping update"), QMessageBox::AcceptRole);
    msgBox.addButton(tr("Cancel"), QMessageBox::RejectRole);
    msgBox.setInformativeText(tr("It is recommended to back up the system before all updates to avoid unnecessary losses!"));
    int ret = msgBox.exec();
    if(ret == 0)
    {
        qDebug() << "立即更新!";
        foreach (AppUpdateWid *wid, widgetList)
        {
            if(wid->updateAPPBtn->text() == tr("Update"))
                wid->updateAPPBtn->setEnabled(false);
        }
//        checkUpdateBtn->start();
//        checkUpdateBtn->setEnabled(false);
        checkUpdateBtn->hide();
        versionInformationLab->setText(tr("Being updated..."));
        updateMutual->isPointOutNotBackup = false;   //全部更新时不再弹出单个更新未备份提示
        emit updateAllSignal(false);
    }
    else if(ret == 1)
    {
        bacupInit(false);
        qDebug() << "不进行全部更新。";
//        checkUpdateBtn->stop();
        checkUpdateBtn->setEnabled(true);
        //       checkUpdateBtn->setText(tr("全部更新"));
        versionInformationLab->setText(tr("Updatable app detected on your system!"));
//        QString updatetime = tr("No Information!");
//        QSqlQuery queryInstall(QSqlDatabase::database("A"));
//        queryInstall.exec("select * from updateinfos order by id desc");
//        while (queryInstall.next()) {
//            QString statusType = queryInstall.value("keyword").toString();
//            if (statusType == "" || statusType =="1") {
//                updatetime = queryInstall.value("date").toString();
//                break;
//            }
//        }
//        lastRefreshTime->setText(tr("Last refresh:")+ updatetime);
        QString checkedtime = tr("No information!");
        QSqlQuery query(QSqlDatabase::database("A"));
        query.exec("select * from display");
        while(query.next())
        {
            checkedtime = query.value("check_time").toString();
        }
        lastRefreshTime->setText(tr("Last Checked:")+checkedtime);
        checkUpdateBtn->setText(tr("UpdateAll"));
        foreach (AppUpdateWid *wid, widgetList)
        {
//            wid->updateAPPBtn->show();
            wid->updateAPPBtn->setEnabled(true);
        }
        isAllUpgrade=false;
    }
}

void TabWid::backupCore()
{
    int initresult = emit needBackUp();
    foreach (AppUpdateWid *wid, widgetList) {
        wid->updateAPPBtn->hide();
        wid->hide();
    }
    switch (initresult) {
    case -1:
        backupMessageBox(tr("The backup restore partition could not be found. The system will not be backed up in this update!"));
        //如果是则立即更新,否的话取消全部更新
        return;
    case -2:
        versionInformationLab->setText(tr("Kylin backup restore tool is doing other operations, please update later."));
        bacupInit(false);
        isAllUpgrade=false;
        return;
    case -3:
        versionInformationLab->setText(tr("The source manager configuration file is abnormal, the system temporarily unable to update!"));
        bacupInit(false);
        isAllUpgrade=false;
        return;
    case -4:
        versionInformationLab->setText(tr("Backup already, no need to backup again."));
        checkUpdateBtn->start();
        checkUpdateBtn->setEnabled(false);
        updateMutual->isPointOutNotBackup = false;   //全部更新时不再弹出单个更新未备份提示
        emit updateAllSignal(false);
        bacupInit(false);
        return;
    case 1://正在备份
        emit startBackUp(0);
        versionInformationLab->setText(tr("Start backup,getting progress")+"...");
        foreach (AppUpdateWid *wid, widgetList)
        {
//            if(wid->updateAPPBtn->text() == tr("Update"))
                wid->updateAPPBtn->setEnabled(false);
        }
        break;
    case needBack://需要备份
        emit startBackUp(1);
        foreach (AppUpdateWid *wid, widgetList)
        {
//            if(wid->updateAPPBtn->text() == tr("Update"))
                wid->updateAPPBtn->setEnabled(false);
        }
        break;
    case -9://备份还原工具不存在
        backupMessageBox(tr("Kylin backup restore tool does not exist, this update will not backup the system!"));
        return;
    default:
        qDebug()<<"备份还原工具状态码"<<initresult;
        isAllUpgrade=false;
        bacupInit(false);
        return;
    }
    qDebug()<<"符合备份工具运行条件";
}

void TabWid::isCancelabled(bool status)
{
    if(status)
    {
        if(isAllUpgrade){
        checkUpdateBtn->setText(tr("Cancel"));
        checkUpdateBtn->setEnabled(true);
        }
//        foreach (AppUpdateWid *wid, widgetList) {
//            if(wid->updateAPPBtn->text() == tr("Cancel"))
//                wid->updateAPPBtn->setEnabled(true);
//        }
    }
    else{
        if(isAllUpgrade){
        checkUpdateBtn->setText(tr("Cancel"));
        checkUpdateBtn->setEnabled(false);
        }
//        foreach (AppUpdateWid *wid, widgetList) {
//            if(wid->updateAPPBtn->text() == tr("Cancel"))
//                wid->updateAPPBtn->setEnabled(false);
//        }
    }

}

void TabWid::DependResloveResult(bool ResloveStatus,bool ResloveResult,QStringList DeletePkgList, QStringList DeletePkgDeslist,QStringList DeletePkgReasonlist,QString Error,QString Reason)
{
    checkUpdateBtn->stop();
    checkUpdateBtn->setText(tr("Calculated"));

    if(!ResloveStatus)
    {
        qDebug()<<Error<<Reason;

        QMessageBox msgBox(qApp->activeModalWidget());
        msgBox.setText(tr("There are unresolved dependency conflicts in this update，Please select Dist-upgrade"));
        msgBox.setWindowTitle(tr("Prompt information"));
        msgBox.setIcon(QMessageBox::Icon::Information);
        msgBox.addButton(tr("Dist-upgrade"), QMessageBox::YesRole);
        msgBox.addButton(tr("Cancel"), QMessageBox::NoRole);

        int ret = msgBox.exec();
        switch (ret)
        {
        case 0:
            qDebug() << "全盘修复。。。。。。";
//            distUpgradeAllApp(false);
            connect(updateMutual->interface,SIGNAL(DistupgradeDependResloveStatus(bool,bool,QStringList,QStringList,QStringList,QString,QString)),this,SLOT(DistupgradeDependResloveResult(bool,bool,QStringList,QStringList,QStringList,QString,QString)));
            updateMutual->DistUpgradeSystem(false);
            break;
        case 1:
            qDebug() << "取消更新";
            updatecancel();
            break;
        default:
            break;
        }
    }
    else
    {
        if (!ResloveResult)
        {
            emit updateAllSignal(true);
        }
        else
        {
            showDependSlovePtompt(UpdateAll,DeletePkgList,DeletePkgDeslist,DeletePkgReasonlist);
        }
    }
    disconnect(updateMutual->interface,SIGNAL(UpdateDependResloveStatus(bool,bool,QStringList,QStringList,QStringList,QString,QString)),this,SLOT(DependResloveResult(bool,bool,QStringList,QStringList,QStringList,QString,QString)));
}

void TabWid::FixBrokenResloveResult(bool ResloveStatus,bool ResloveResult,QStringList DeletePkgList, QStringList DeletePkgDeslist,QStringList DeletePkgReasonlist,QString Error,QString Reason)
{     
    showFixbrokenPtompt(UpdateAll,DeletePkgList,DeletePkgDeslist,DeletePkgReasonlist);
}

void TabWid::InstallStatus(bool success, QString err)
{
    if(!success)
    {
        if(err == "error-not-disk-space")
        {
            //弹窗提示空间不足
            QMessageBox msgbox;
            msgbox.setText(tr("Insufficient disk space to download updates!"));
            msgbox.setStandardButtons(QMessageBox::Ok);
            msgbox.setButtonText(QMessageBox::Ok,tr("OK"));
            msgbox.exec();
            return ;
        }
    }
}

void TabWid::DistupgradeDependResloveResult(bool ResloveStatus,bool ResloveResult,QStringList DeletePkgList, QStringList DeletePkgDeslist,QStringList DeletePkgReasonlist,QString Error,QString Reason)
{
    if(!ResloveStatus)
    {
        qDebug()<<Error<<Reason;
        //遇到无法修复的依赖
        QMessageBox msgBox(qApp->activeModalWidget());
        msgBox.setText(tr("There are unresolved dependency conflicts in this update，Please contact the administrator!"));
        msgBox.setWindowTitle(tr("Prompt information"));
        msgBox.setIcon(QMessageBox::Icon::Information);
        msgBox.addButton(tr("Sure"), QMessageBox::YesRole);

        int ret = msgBox.exec();
        switch (ret)
        {
        case 0:
            qDebug() << "取消更新";
            updatecancel();
            break;
        default:
            break;
        }
    }
    else
    {
        if (!ResloveResult)
        {
            foreach (AppUpdateWid *wid, widgetList) {
                wid->hide();
            }
            connect(updateMutual->interface,SIGNAL(UpdateDloadAndInstStaChanged(QStringList,int,QString,QString)),this,SLOT(getAllProgress(QStringList,int,QString,QString)));
            connect(updateMutual->interface,SIGNAL(UpdateDownloadInfo(QStringList,int,int,uint,uint,int)),this,SLOT(showDownloadInfo(QStringList,int,int,uint,uint,int)));
            connect(updateMutual->interface,SIGNAL(UpdateInstallFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlot(bool,QStringList,QString,QString)));
            connect(updateMutual->interface,SIGNAL(UpdateDownloadFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlotindownload(bool,QStringList,QString,QString)));
            updateMutual->DistUpgradeSystem(true);
//            distUpgradeAllApp(true);
        }
        else
        {
//           QStringList DeletePkgReasonlist={""};
           showDependSlovePtompt(UpdateSystem,DeletePkgList,DeletePkgDeslist,DeletePkgReasonlist);
        }
    }
}

QString TabWid::allLableData (bool one,QString detailinfo,uint downloadedsize,uint totalsize,int speed)//处理两个信号内容进行数据结合的
{
    static QString _detailinfo;
    static uint _downloadedsize;
    static uint _totalsize;
    static int _speed;
    if(one == true)
    {
        _detailinfo = detailinfo;
    }
    else
    {
        _downloadedsize = downloadedsize;
        _totalsize = totalsize;
        _speed = speed;
    }
    qDebug()<<_detailinfo<<_downloadedsize<<_totalsize<<_speed;
    QString labelname = _detailinfo+":"+modifySizeUnit(_downloadedsize)+"/"+modifySizeUnit(_totalsize)+"("+modifySpeedUnit(_speed, 1)+")"+" "+tr("supposed")+caltime(_downloadedsize,_totalsize,_speed);
//    QString labelname = modifySizeUnit(_downloadedsize)+"/"+modifySizeUnit(_totalsize)+" "+modifySpeedUnit(_speed, 1)+"预计";
    return labelname;
}


void TabWid::getAllProgress (QStringList pkgName,int Progress,QString status,QString detailinfo)
{
    if (!isAllUpgrade)
    {
        return ;
    }
//    if(isContinueUpgrade)//判断下载和安装    /*3-11 fixbug*/
//    {
//        checkUpdateBtn->show();
//        isContinueUpgrade=false;
//    }

    TabWid::allLableData(true,status,0,0,0);
    qDebug() << pkgName << Progress << status;
    QDBusInterface upgrade_interface( "com.kylin.systemupgrade",
                                 "/com/kylin/systemupgrade",
                                 "com.kylin.systemupgrade.interface",
                                 QDBusConnection::systemBus() );
    QDBusMessage modelist = upgrade_interface.call("GetConfigValue","InstallMode","shutdown_install");
    QString  mode =modelist.arguments().value(1).toString();
    qDebug()<<mode;
    if(mode =="True")
    {
        versionInformationLab->setText(tr("The system is downloading the update!"));
    }
    else
    {
        if(Progress < 50)
            versionInformationLab->setText(tr("Downloading the updates..."));
        else
            versionInformationLab->setText(tr("Installing the updates..."));
    }


//    if (Progress < 100)
//        detailLabel->setText(detailinfo);
    allProgressBar->setValue(Progress);
    allProgressBar->show();
    lastRefreshTime->show();
//    detailLabel->show();
    if (Progress == 100)
    {
        disconnect(updateMutual->interface,SIGNAL(UpdateDloadAndInstStaChanged(QStringList,int,QString,QString)),this,SLOT(getAllProgress(QStringList,int,QString,QString)));
        disconnect(updateMutual->interface,SIGNAL(Cancelable(bool)),this,SLOT(isCancelabled(bool)));
    }
}

QString TabWid::caltime(uint downloadedsize,uint totalsize,int speed)
{
    int cha = totalsize-downloadedsize;
    qDebug()<<"cha is"<<cha;
    if(cha<0)
        return QString("%1").arg("----");
    else
    {
        if(speed <= 0)
            return QString("%1").arg("----");
        else
        {
            uint time = cha/speed;
            qDebug()<<"time is"<<cha;
            if(time<60)
                return QString("%1%2").arg(time).arg(tr("s"));
            else if((time/60)<60)
                return QString("%1%2").arg(time/60).arg(tr("min"));
            else if((time/3600)<24)
                return QString("%1%2").arg(time/3600).arg(tr("h"));
            else
                return QString("%1").arg("超过一天");
        }
    }
}

//当前下载速度单位换算
QString TabWid::modifySpeedUnit(long size, float time)
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

//转换包大小的单位
QString TabWid::modifySizeUnit(long size)
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

void TabWid::showDownloadInfo(QStringList pkgname,int currentserial,int total,uint downloadedsize,uint totalsize,int speed)
{
    qDebug()<<"showDownloadinfo";
    QString info=TabWid::allLableData(false,"",downloadedsize,totalsize,speed);
    Q_UNUSED(pkgname);
    if(speed > 0)
        alldownloadstart=true;
    QString downloadspeed = modifySpeedUnit(speed, 1);
    if(currentserial==total)
    {
        lastRefreshTime->setText(tr("In the install"));
        disconnect(updateMutual->interface,SIGNAL(UpdateDownloadInfo(QStringList,int,int,uint,uint,int)),this,SLOT(showDownloadInfo(QStringList,int,int,uint,uint,int)));
        return;
    }
    if(speed == 0 && !alldownloadstart)
    {
//        lastRefreshTime->setText(tr("In the download")+":"+pkgname+modifySizeUnit(downloadedsize)+"/"+modifySizeUnit(totalsize)+"("+tr("calculating")+")");
        lastRefreshTime->setText(info);
        lastRefreshTime->setText(tr("In the download")+":"+modifySizeUnit(downloadedsize)+"/"+modifySizeUnit(totalsize)+"("+tr("calculating")+")");
        return;
    }
    lastRefreshTime->setText(tr("In the download")+":"+modifySizeUnit(downloadedsize)+"/"+modifySizeUnit(totalsize)+"("+downloadspeed+")");
    lastRefreshTime->setText(info);
}

void TabWid::backupProgress(int progress)
{
    qDebug() <<"备份进度：---->" << progress;
    if(progress==100)
    {
        bacupInit(false);
        //备份完成，开始安装
        qDebug()<<"备份完成，开始安装";
        versionInformationLab->setText(tr("Backup complete."));
        allProgressBar->setValue(progress);
        checkUpdateBtn->show();
        sleep(1);
        allProgressBar->hide();
        lastRefreshTime->show();
        checkUpdateBtn->start();
        //        versionInformationLab->setText("正在更新...");
        versionInformationLab->setText(tr("Being updated..."));
        updateMutual->isPointOutNotBackup = false;   //全部更新时不再弹出单个更新未备份提示
        emit updateAllSignal(false);
        foreach (AppUpdateWid *wid, widgetList)
        {
            if(wid->updateAPPBtn->text() == tr("Cancel"))
                wid->updateAPPBtn->hide();
        }
        return;
    }
    versionInformationLab->setText(tr("System is backing up..."));
    allProgressBar->setValue(progress);
    checkUpdateBtn->hide();
    allProgressBar->show();
    lastRefreshTime->hide();
    foreach (AppUpdateWid *wid, widgetList)
    {
//            if(wid->updateAPPBtn->text() == tr("Update"))
            wid->updateAPPBtn->setEnabled(false);
    }
}

void TabWid::bakeupFinish(int result)
{
    switch (result)
    {
    case -20:
        //        versionInformationLab->setText(tr("备份过程被中断，停止更新！"));
        versionInformationLab->setText(tr("Backup interrupted, stop updating!"));
        foreach (AppUpdateWid *wid, widgetList) {
//            if(wid->updateAPPBtn->text() == tr("Update"))
                wid->updateAPPBtn->setEnabled(true);
        }
        allProgressBar->hide();
        checkUpdateBtn->show();
        lastRefreshTime->show();
        checkUpdateBtn->stop();
        checkUpdateBtn->setEnabled(true);
        //        checkUpdateBtn->setText(tr("全部更新"));
        checkUpdateBtn->setText(tr("UpdateAll"));
        checkUpdateBtn->adjustSize();
        break;
    case 99:
        break;
    default:
        versionInformationLab->setText(tr("The status of backup completion is abnormal"));
        //        backupMessageBox(tr("麒麟备份还原工具异常：")+QString::number(result)+","+tr("本次更新不会备份系统！"));
        backupMessageBox(tr("Kylin backup restore tool exception:")+QString::number(result)+","+tr("There will be no backup in this update!"));
        break;
    }
}

void TabWid::bacupInit(bool isConnect)
{
    qDebug() << "======>tabwid info: " <<backupThread ;
    connect(this,&TabWid::needBackUp,backup,&BackUp::needBacdUp,Qt::BlockingQueuedConnection);//同步信号，阻塞，取返回值
    if (isConnect) {
        qDebug() << "is connect ;;";
        connect(this,&TabWid::startBackUp,backup,&BackUp::startBackUp);
        connect(backup, &BackUp::calCapacity, this, &TabWid::whenStateIsDuing);
        connect(backup, &BackUp::backupStartRestult, this, &TabWid::receiveBackupStartResult);
        connect(backup,&BackUp::bakeupFinish,this,&TabWid::bakeupFinish);
        connect(backup,&BackUp::backupProgress,this,&TabWid::backupProgress);
    } else {
        qDebug() << "is disconnect;;";
        disconnect(this,&TabWid::startBackUp,backup,&BackUp::startBackUp);
        disconnect(backup, &BackUp::calCapacity, this, &TabWid::whenStateIsDuing);
        disconnect(backup, &BackUp::backupStartRestult, this, &TabWid::receiveBackupStartResult);
        disconnect(backup,&BackUp::bakeupFinish,this,&TabWid::bakeupFinish);
        disconnect(backup,&BackUp::backupProgress,this,&TabWid::backupProgress);
    }

}

void TabWid::backupDelete()
{
    backup->deleteLater();
    backupThread->deleteLater();
}

void TabWid::slotUpdateTemplate(QString status)
{
    qDebug() <<  "源管理器：" <<"update template status :" << status;

}

void TabWid::getpkginfoFromJson(AppAllMsg *msg,QString pkgname)
{
    /*判断json文件是否存在*/
    QString filename = QString(JSON_FILE_PATH) +pkgname +".json";
    qDebug() << filename;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)){
        qDebug() << "JSON file open failed! ";
        return ;
    }
    QByteArray jsonData = file.readAll();

    QJsonParseError err_rpt;
    QJsonDocument  root_Doc = QJsonDocument::fromJson(jsonData, &err_rpt); // 字符串格式化为JSON

    if (!root_Doc.isNull() && (err_rpt.error == QJsonParseError::NoError)) {  // 解析未发生错误
        if (root_Doc.isObject())
        { // JSON 文档为对象
            QJsonObject object = root_Doc.object();  // 转化为对象
            if (QLocale::system().name() == "zh_CN")
            {
                QString name  = object.value("name").toObject().value("zh_CN").toString();
                QString description  = object.value("description").toObject().value("zh_CN").toString();
                if (!name.isNull()) {
                    msg->name=name;
                }
                if (!description.isNull()) {
                    msg->shortDescription=description;
                }
            }
            else
            {
                QString name  = object.value("name").toObject().value("en_US").toString();
                QString description  = object.value("description").toObject().value("en_US").toString();
                if (!name.isNull()) {
                    msg->name=name;
                }
                if (!description.isNull()) {
                    msg->shortDescription=description;
                }
            }
            QString upgrade_download_size_str  = object.value("upgrade_list").toObject().value("total_download_size").toString();
            long upgrade_download_size  = upgrade_download_size_str.toLong();
            QString install_download_size_str  = object.value("install_list").toObject().value("total_download_size").toString();
            long install_download_size  = install_download_size_str.toLong();
            msg->msg.allSize = upgrade_download_size + install_download_size;
            QString upgrade_install_size_str  = object.value("upgrade_list").toObject().value("total_install_size").toString();
            long upgrade_install_size  = upgrade_install_size_str.toLong();
            QString install_install_size_str  = object.value("install_list").toObject().value("total_install_size").toString();
            long install_install_size = install_install_size_str.toLong();
            msg->msg.installSize = upgrade_install_size + install_install_size;

            QString new_version = object.value("new_version").toString();
            if (!new_version.isNull())
                msg->availableVersion=new_version;
            QString cur_version = object.value("cur_version").toString();
            if (!cur_version.isNull())
                msg->version=cur_version;
            QString changelog = object.value("changelog").toString();
            if (!changelog.isNull())
                msg->longDescription=changelog;
        }
    }else{
        qDebug() << "JSON文件格式错误！";
        return ;
    }
    file.close();
}

void TabWid::slotUpdateCache(bool result,QStringList pkgname, QString error ,QString reason)
{
        AppAllMsg msg;
        isConnectSourceSignal = true;
        int size=0;
        lastRefreshTime->show();
        checkUpdateBtn->show();
        qDebug() <<"源管理器：" << "update cache status :" << result;
        QDir dir("/tmp/upgrade");
        if(!dir.exists("/tmp/upgrade/"))
        {
            dir.mkdir("/tmp/upgrade/");
        }
        if (result)
        {
            versionInformationLab->setText(tr("Getting update list")+"...");
            QList<QString>::Iterator it = pkgname.begin(),itend = pkgname.end();
            int i = 0;
            for (;it != itend; it++,i++){
                qDebug() << "slotUpdateCache函数：获取到的包列表：" << pkgname[i];
                QFile file("/tmp/upgrade/upgrade.list");
                file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
                QString package=pkgname[i]+" ";
                file.write(package.toUtf8());
                file.close();
                getpkginfoFromJson(&msg,pkgname[i]);
                msg.name=pkgname[i];
                loadingOneUpdateMsgSlot(msg);
                size++;
            }
            loadingFinishedSlot(size);
        }
        else
        {
            if(pkgname[0]=="kylin-system-updater")
            {
                checkUpdateBtn->hide();
                versionInformationLab->setText(error);
                versionInformationLab->setToolTip(reason);
            }
            else
            {
                qDebug() << "更新失败 ";
                checkUpdateBtn->setEnabled(true);
                checkUpdateBtn->stop();
                //                checkUpdateBtn->setText(tr("检查更新"));
                checkUpdateBtn->setText(tr("Retry"));
                //                versionInformationLab->setText(tr("软件源更新失败：")+failedInfo );
                versionInformationLab->setText(error);
                versionInformationLab->setToolTip(reason);
                QString checkedtime = tr("No information!");
                QSqlQuery query(QSqlDatabase::database("A"));
                query.exec("select * from display");
                while(query.next())
                {
                    checkedtime = query.value("check_time").toString();
                }
                lastRefreshTime->setText(tr("Last Checked:")+checkedtime);
                checkUpdateBtn->show();
            disconnect(updateSource->serviceInterface,SIGNAL(updateTemplateStatus(QString)),this,SLOT(slotUpdateTemplate(QString)));
            disconnect(updateMutual->interface,SIGNAL(UpdateDetectFinished(bool,QStringList,QString,QString)),this,SLOT(slotUpdateCache(bool,QStringList,QString,QString)));
            disconnect(updateMutual->interface,SIGNAL(UpdateDetectStatusChanged(int,QString)),this,SLOT(slotUpdateCacheProgress(int,QString)));
            }
        }  
}

void TabWid::slotUpdateCacheProgress(int progress, QString status)
{
    checkUpdateBtn->hide();
//    detailLabel->show();
    isConnectSourceSignal = true;
//    checkUpdateBtn->hide();
    QString updatetime =  tr("No Information!");
    if (progress > 100) {
//        progress = 100;
        return;
    }

    if  (lastprogress > progress)
        return;

    lastprogress = progress;

//    lastRefreshTime->setText(status);
    lastRefreshTime->hide();

    if(lastprogress==92)
    {
        lastprogress=0;

//        QSqlQuery queryInstall(QSqlDatabase::database("A"));
//        queryInstall.exec("select * from updateinfos order by id desc");
//        while (queryInstall.next())
//        {

//            QString statusType = queryInstall.value("keyword").toString();
//            if (statusType == "" || statusType =="1")
//            {
//                updatetime = queryInstall.value("date").toString();
//                break;
//            }
//        }
//        lastRefreshTime->setText(tr("Last refresh:")+ updatetime);
        QString checkedtime = tr("No information!");
        QSqlQuery query(QSqlDatabase::database("A"));
        query.exec("select * from display");
        while(query.next())
        {
            checkedtime = query.value("check_time").toString();
        }
        lastRefreshTime->setText(tr("Last Checked:")+checkedtime);
    }

    qDebug() << "update cache progress :" << progress;

    versionInformationLab->setText(tr("The system is checking update :") + QString::number(progress)+"%");
//    lastRefreshTime->setText(status);
    //versionInformationLab->setText(tr("更新软件源进度：") + QString::number(progress)+"%");

}

void TabWid::allComponents()
{
    AppMessage = new QVBoxLayout();
    AppMessage->setAlignment(Qt::AlignTop);
    this->setLayout(AppMessage);
    AppMessage->setContentsMargins(0, 0, 0, 0);

    /*系统更新第一部分*/
    /*update一级标题*/
    labUpdate = new TitleLabel(this);
    //    labUpdate->setText(tr("更新"));
    labUpdate->setText(tr("Update"));
    /*检查更细框内布局设置*/
    systemWidget = new QFrame(this);
    systemWidget->setFrameShape(QFrame::Box);
//    systemWidget->setMinimumSize(QSize(550,0));
//    systemWidget->setMaximumSize(QSize(16777215, 16777215));
    /*appwidget初始布局*/
    allUpdateWid = new QWidget(this);
    allUpdateLayout = new QVBoxLayout();
    allUpdateLayout->setAlignment(Qt::AlignTop);
    allUpdateWid->setLayout(allUpdateLayout);
    allUpdateLayout->setSpacing(0);
    allUpdateLayout->setMargin(0);

    /*sp2中删除的布局设置，可能在sp3中仍然使用*/
//    labUpdate->setFixedHeight(27);
//    systemWidget->resize(560,140);
//    systemWidget->setFixedHeight(128);
    tab1HLayout = new QHBoxLayout();
//    tab1HLayout->setMargin(0);
//    tab1HLayout->setSpacing(0);
    systemWidget->setLayout(tab1HLayout);
    tab1HLayout->setContentsMargins(16,0,16,0);
    systemWidget->setFixedHeight(132);

    /*左侧图片*/
    systemPortraitLab = new QLabel(systemWidget);
    systemPortraitLab->setFixedSize(96,96);
    systemPortraitLab->setPixmap(QPixmap(":/img/upgrade/normal.png").scaled(QSize(96,96)));



    /*中间布局*/
    QWidget *inforWidget = new QWidget();
    inforLayout = new QVBoxLayout();
    inforWidget->setLayout(inforLayout);

    versionInformationLab = new QLabel();
    allProgressBar = new KProgressBar(systemWidget);
    allProgressBar->setFixedHeight(20);
    allProgressBar->setBodyWidth(12);
//    allProgressBar->setMaximumWidth(500);
    allProgressBar->hide();
    allProgressBar->setValue(0);
//    allProgressBar->adjustSize();
//    allProgressBar->setTextVisible(false);
    lastRefreshTime = new LightLabel(systemWidget);
//    detailLabel = new FixLabel(systemWidget);
//    detailLabel->hide();
//    inforLayout->setAlignment(Qt::AlignTop|Qt::AlignLeft);
//    inforLayout->addSpacing(8);
//    inforLayout->addSpacerItem();
//    QSpacerItem *mItem = new QSpacerItem(0, 20, QSizePolicy::Expanding);
//    inforLayout->addSpacerItem(mItem);
    inforLayout->addStretch();
    inforLayout->addWidget(versionInformationLab);
    inforLayout->addWidget(allProgressBar,Qt::AlignVCenter);
    inforLayout->addWidget(lastRefreshTime);
    inforLayout->addStretch();
    allProgressBar->hide();
//    inforLayout->addSpacerItem(mItem);
//    inforLayout->addSpacing(4);
//    //    inforLayout->addWidget(progressLabel,Qt::AlignHCenter);
//    //    inforLayout->addSpacing(4);
//    inforLayout->addWidget(detailLabel,Qt::AlignHCenter);
//    inforLayout->addSpacing(4);
//    inforLayout->setContentsMargins(8,0,0,0);

//    detailLabel = new QLabel(systemWidget);
//    detailLabel->adjustSize();
//    detailLabel->hide();


    /*右侧按键布局*/
    checkUpdateBtn = new m_button(systemWidget);
    checkUpdateBtn->setDefault(true);
    checkUpdateBtn->start();
//    checkUpdateBtn->setFixedSize(120,36);
//    checkUpdateBtn->adjustSize();

////    updatedetaileInfo = new QPushButton(this);
////    //    detaileInfo->setText(tr("详情"));
////    updatedetaileInfo->setText(tr("details"));
////    updatedetaileInfo->hide();
////    updatedetaileInfo->setFixedSize(120,36);
//    controlLayout = new QVBoxLayout();
//    controlLayout->addWidget(checkUpdateBtn);
////    controlLayout->addSpacing(8);
////    controlLayout->addWidget(updatedetaileInfo);
//    QWidget *checkUpdateBtnWidget = new QWidget();
//    checkUpdateBtnWidget->setLayout(controlLayout);

//    QWidget *controlWidget = new QWidget(systemWidget);
//    controlWidget->setLayout(controlLayout);

    tab1HLayout->addWidget(systemPortraitLab,0,Qt::AlignHCenter|Qt::AlignLeft);
    tab1HLayout->addWidget(inforWidget,Qt::AlignLeft|Qt::AlignTop);
//    tab1HLayout->addSpacerItem(new QSpacerItem(50, 2, QSizePolicy::Expanding));
//    tab1HLayout->addWidget(controlWidget);
    tab1HLayout->addWidget(checkUpdateBtn);
//    tab1HLayout->setContentsMargins(8,25,20,20);
    tab1HLayout->setContentsMargins(16,16,16,16);
    tab1HLayout->setSpacing(0);
//    tab1HLayout->setMargin(0);



    /*******Update Settings*******/
    updateSettingLab = new TitleLabel();
    //    updateSettingLab->setText(tr("更新设置"));
    //~ contents_path /upgrade/Update Settings
    updateSettingLab->setText(tr("Update Settings"));
    updateSettingLab->setFixedHeight(27);
    /*the whole update setting layout*/
    updateSettingWidget = new QFrame(this);
    updateSettingWidget->setFrameShape(QFrame::Box);
    updatesettingLayout = new QVBoxLayout();
    updateSettingWidget->setLayout(updatesettingLayout);

    /*switchbutton1：允许自动更新*/
    isAutoCheckWidget = new QFrame();
    isAutoCheckWidget->setFrameShape(QFrame::Box);
    isAutoCheckWidget->setFixedHeight(60);
    isAutoCheckLayout = new QHBoxLayout();
    isAutoCheckedLab = new QLabel();
    //    isAutoCheckedLab->setText(tr("允许通知可更新的应用"));
    isAutoCheckedLab->setText(tr("Allowed to renewable notice"));
//    isAutoCheckSBtn = new SwitchButton();
    isAutoCheckSBtn = new KSwitchButton();
    isAutoCheckSBtn->setFixedWidth(50);
    isAutoCheckWidget->setLayout(isAutoCheckLayout);
    isAutoCheckLayout->setContentsMargins(8, 0, 14, 0);
    isAutoCheckLayout->addSpacing(6);
    isAutoCheckLayout->addWidget(isAutoCheckedLab);
    isAutoCheckLayout->addWidget(isAutoCheckSBtn);

    /*没用到的开关：自动下载和安装更新，后期处理以后可以删除了*/
    isAutoBackupWidget = new QFrame();
    isAutoBackupWidget->setFrameShape(QFrame::Box);
    isAutoBackupLayout = new QHBoxLayout();
    isAutoBackupLayout->setContentsMargins(8, 0, 14, 0);
    isAutoBackupLab = new QLabel();
    isAutoBackupLab->setText(tr("Backup current system before updates all"));
    //    isAutoBackupLab->setText(tr("全部更新前备份系统"));
    isAutoBackupSBtn = new KSwitchButton();
    isAutoBackupLayout->addWidget(isAutoBackupLab);
    isAutoBackupLayout->addWidget(isAutoBackupSBtn);
    isAutoBackupWidget->setLayout(isAutoBackupLayout);

    /*switchbutton3：下载限速*/
    DownloadVWidget = new QFrame();
    DownloadVWidget->setFrameShape(QFrame::Box);
    DownloadVWidget->setFixedHeight(60);
    DownloadVLayout = new QVBoxLayout();
    DownloadVLayout->setContentsMargins(8, 0, 14, 0);
    DownloadHLayout = new QHBoxLayout();
    DownloadVLabLayout = new QHBoxLayout();
    DownloadHLab = new QLabel();
    DownloadHLab->setText(tr("Download Limit"));
//    DownloadHBtn = new SwitchButton();
    DownloadHBtn = new KSwitchButton();
    DownloadHBtn->setFixedWidth(50);
    DownloadHValue = new QComboBox();
    QStringList strList;
    strList<<"50KB/s"<<"100KB/s"<<"200KB/s"<<"300KB/s"<<"600KB/s";
    DownloadHValue->addItems(strList);
    DownloadVLab = new LightLabel(tr("It will be avaliable in the next download."));
//    DownloadVLab->setText(tr("It will be avaliable in the next download."));
//    DownloadVLab->setStyleSheet("background:transparent;color:#626c6e;");
//    DownloadVLab->adjustSize();
    DownloadVLab->setWordWrap(true);
    DownloadVLab->setAlignment(Qt::AlignTop);
//    QPalette DownloadVlabelPalette = DownloadVLab->palette();
//    DownloadVlabelPalette.setBrush(QPalette::WindowText, DownloadVlabelPalette.color(QPalette::PlaceholderText));
//    DownloadVLab->setPalette(DownloadVlabelPalette);
    DownloadVLab->hide();
    DownloadHLayout->addSpacing(6);
    DownloadHLayout->addWidget(DownloadHLab);
    DownloadHLayout->addWidget(DownloadHValue);
    DownloadHLayout->addWidget(DownloadHBtn);
    DownloadVLayout->addLayout(DownloadHLayout);
    DownloadVLayout->addLayout(DownloadVLabLayout);
    DownloadVLabLayout->addSpacing(6);
    DownloadVLabLayout->addWidget(DownloadVLab);
    DownloadVWidget->setLayout(DownloadVLayout);

    /*switchbutton2：自动下载和安装更新*/
    isAutoUpgradeWidget = new QFrame();
    isAutoUpgradeWidget->setFrameShape(QFrame::Box);
    isAutoUpgradeWidget->setFixedHeight(60);
    isAutoUpgradeLayout = new QVBoxLayout();
    isAutoUpgradeLayout->setContentsMargins(8, 0, 14, 0);
    isAutoUpgradeLab = new QLabel();
    isAutoUpgradeLab->setText(tr("Automatically updates"));
    autoUpgradeLab = new LightLabel(tr("The system will automatically updates when there is an available network and backup."));
//    explicit LightLabel(QString text , QWidget *parent = nullptr);
//    autoUpgradeLab->setText(tr("The system will automatically updates when there is an available network and backup."));//The system will automatically updates when there is an available network and backup.//After it is turned on, the system will automatically download and install updates when there is an available network and available backup and restore partitions.
//    autoUpgradeLab->adjustSize();
//    autoUpgradeLab->setStyleSheet("background:transparent;color:#626c6e;");
    autoUpgradeLab->setWordWrap(true);
    autoUpgradeLab->setAlignment(Qt::AlignTop);
    autoUpgradeLab->hide();
//    QPalette labelPalette = autoUpgradeLab->palette();
//    labelPalette.setBrush(QPalette::WindowText, labelPalette.color(QPalette::PlaceholderText));
//    autoUpgradeLab->setPalette(labelPalette);
//    isAutoUpgradeSBtn = new SwitchButton();
    isAutoUpgradeSBtn = new KSwitchButton();
    isAutoUpgradeSBtn->setFixedWidth(50);
    autoUpgradeBtnLayout = new QHBoxLayout();
    autoUpgradeBtnLayout->addSpacing(6);
    autoUpgradeBtnLayout->addWidget(isAutoUpgradeLab);
    autoUpgradeBtnLayout->addWidget(isAutoUpgradeSBtn);
    /*二選一*/
//    m_pButtonGroup = new QButtonGroup();
//    poweroffUpgradeBtn = new QRadioButton(tr("Upgrade during poweroff"), this);
//    ontimeUpgradeBtn = new QRadioButton(tr("Upgrade at period time"), this);
//    ontimeUpgradeLab = new QLabel();
//    ontimeUpgradeValue = new QComboBox();
//    ontimeUpgradeLayout = new QHBoxLayout();
//    QStringList TimeList;
//    TimeList<<"1"<<"3"<<"7"<<"15"<<"30";
//    ontimeUpgradeValue->addItems(TimeList);
//    ontimeUpgradeValue->setFixedWidth(80);
//    ontimeUpgradeValue->setEnabled(false);
//    ontimeUpgradeLab->setText(tr("days to update"));
////    autoUpgradeLab->adjustSize();
////    ontimeUpgradeLab->setWordWrap(true);
////    ontimeUpgradeLab->setAlignment(Qt::AlignTop);
//    /*字体置灰*/
////    QPalette ontimeupgradelabelPalette = ontimeUpgradeLab->palette();
////    ontimeupgradelabelPalette.setBrush(QPalette::WindowText, ontimeupgradelabelPalette.color(QPalette::PlaceholderText));
////    ontimeUpgradeLab->setPalette(ontimeupgradelabelPalette);
//    poweroffUpgradeBtn->setProperty("value", "poweroff");
//    ontimeUpgradeBtn->setProperty("value","fixedtime");
//    m_pButtonGroup->addButton(poweroffUpgradeBtn);
//    m_pButtonGroup->addButton(ontimeUpgradeBtn);
//    isAutoUpgradeLayout->addLayout(autoUpgradeBtnLayout);
//    isAutoUpgradeLayout->addWidget(autoUpgradeLab);
//    isAutoUpgradeLayout->addWidget(poweroffUpgradeBtn);
//    ontimeUpgradeLayout->addWidget(ontimeUpgradeBtn);
//    ontimeUpgradeLayout->addWidget(ontimeUpgradeValue);
//    ontimeUpgradeLayout->addWidget(ontimeUpgradeLab);
//    isAutoUpgradeLayout->addLayout(ontimeUpgradeLayout);
//    isAutoUpgradeLayout->addWidget(ontimeUpgradeBtn);
//    isAutoUpgradeWidget->setLayout(isAutoUpgradeLayout);
    isAutoUpgradeLayout->addLayout(autoUpgradeBtnLayout);
    autoUpgradeLayout = new QHBoxLayout();
    autoUpgradeLayout->addSpacing(6);
    autoUpgradeLayout->addWidget(autoUpgradeLab);
    isAutoUpgradeLayout->addLayout(autoUpgradeLayout);
    isAutoUpgradeWidget->setLayout(isAutoUpgradeLayout);
    /*临时加入的分割线，在后期可以化成一个模块*/
    QFrame *line12 = new QFrame(this);
    line12->setMinimumSize(QSize(0, 1));
    line12->setMaximumSize(QSize(16777215, 1));
    line12->setLineWidth(0);
    line12->setFrameShape(QFrame::HLine);
    line12->setFrameShadow(QFrame::Sunken);

    QFrame *line23 = new QFrame(this);
    line23->setMinimumSize(QSize(0, 1));
    line23->setMaximumSize(QSize(16777215, 1));
    line23->setLineWidth(0);
    line23->setFrameShape(QFrame::HLine);
    line23->setFrameShadow(QFrame::Sunken);

    QWidget *historyUpdateLogWid = new QWidget(this);
    QHBoxLayout *historyUpdateLogLayout = new QHBoxLayout();
    historyUpdateLog = new QPushButton(this); // 历史日志弹出窗口控制按钮
    //    historyUpdateLog->setText(tr("查看更新历史"));
    historyUpdateLog->setText(tr("View history"));
    historyUpdateLog->setFixedHeight(36);
    historyUpdateLogWid->setLayout(historyUpdateLogLayout);
    historyUpdateLogLayout->setAlignment(Qt::AlignLeft);
    historyUpdateLogLayout->addWidget(historyUpdateLog);
    historyUpdateLogLayout->setSpacing(0);
    historyUpdateLogLayout->setMargin(0);


    /*第二个titlelabel的布局*/
    updatesettingLayout->setAlignment(Qt::AlignTop);
//    updatesettingLayout->addWidget(updateSettingLab);
//    updatesettingLayout->addSpacing(10);
    updatesettingLayout->addWidget(isAutoCheckWidget);
    updatesettingLayout->setSpacing(2);/*加了两行参数*/
    updatesettingLayout->setMargin(0);/*加了两行参数*/
updatesettingLayout->addWidget(line12);
    updatesettingLayout->addWidget(isAutoUpgradeWidget);
    updatesettingLayout->setSpacing(2);
    updatesettingLayout->setMargin(0);
updatesettingLayout->addWidget(line23);
    updatesettingLayout->addWidget(DownloadVWidget);
    updatesettingLayout->setSpacing(2);
    updatesettingLayout->setMargin(0);

    AppMessage->addWidget(labUpdate);
    AppMessage->addWidget(systemWidget);
    AppMessage->addWidget(allUpdateWid);
    AppMessage->addSpacing(30);
    AppMessage->addWidget(updateSettingLab);
    AppMessage->addWidget(updateSettingWidget);
    AppMessage->addSpacing(2);
    AppMessage->addWidget(historyUpdateLogWid);
    AppMessage->addStretch();
    AppMessage->setContentsMargins(0,0,0,0);

//    mainTabLayout->setAlignment(Qt::AlignTop);
//    mainTabLayout->addWidget(scrollArea);
//    //    mainTabLayout->setSpacing(0);
//    mainTabLayout->setMargin(0);
//    this->setLayout(mainTabLayout);
    getAllDisplayInformation();

}

void TabWid::loadingOneUpdateMsgSlot(AppAllMsg msg)
{
    //    checkUpdateBtn->setText();
    if(updateMutual->importantList.indexOf(msg.name) == -1)
    {
        struct pkgProgress pkg;
        pkg.name = msg.name;
        pkgList << pkg;
        updateMutual->importantList.append(msg.name);   //重要更新列表中添加appname
        AppUpdateWid *appWidget = new AppUpdateWid(msg, this);
        widgetList << appWidget;
        connect(appWidget, &AppUpdateWid::allappupdatefinishsignal, this, &TabWid::allappupdatefinishSlot);
        connect(appWidget, &AppUpdateWid::changeupdateall, this, &TabWid::checkUpdateBtnClicked);
        connect(appWidget, &AppUpdateWid::appupdateiscancel, this, &TabWid::slotCancelDownload);
        connect(appWidget,&AppUpdateWid::oneappUpdateResultSignal,this,&TabWid::oneappUpdateresultSlot);
        connect(appWidget,&AppUpdateWid::ShutdowninstallFinishSignal,this,&TabWid::shutdowninstallfinishslot);
        connect(appWidget,&AppUpdateWid::changeUpdateAllSignal,this,&TabWid::changeUpdateAllSlot);
        connect(updateMutual,&UpdateDbus::sendFinishGetMsgSignal,appWidget,&AppUpdateWid::showUpdateBtn);
        connect(appWidget,&AppUpdateWid::filelockedSignal,this,&TabWid::waitCrucialInstalled);
//        connect(backup,&BackUp::bakeupFinish,appWidget,&AppUpdateWid::hideOrShowUpdateBtnSlot);
        connect(appWidget,&AppUpdateWid::startoneappupdate,this,&TabWid::startoneappupdateslot);
        /*判断是否是后台自动更新*/
        if (isAutoUpgrade) {
           // connect(appWidget, &AppUpdateWid::sendProgress, this, &TabWid::getAllProgress);
            appWidget->isUpdateAll = true;
            appWidget->isAutoUpgrade = true;
            appWidget->updateAPPBtn->hide();
            appWidget->appVersion->setText(tr("Ready to install"));
        }
        if(ukscConnect->isConnectUskc == true)
        {
            QStringList list = ukscConnect->getInfoByName(msg.name);
            if(list[2] != "")
            {
                qDebug()<<"ukscconnect->appnamelab is"<<list[2];
                appWidget->appNameLab->setText(list[2]);
                appWidget->dispalyName = list[2];

            }
            if(list[1] != "" && QLocale::system().name()=="zh_CN")
            {
                qDebug()<<"ukscconnect->appnamelab is"<<list[1];
                appWidget->appNameLab->setText(list[1]);
                appWidget->dispalyName = list[1];
            }
            if(list[0] != "" && !appWidget->haveThemeIcon)
            {
                appWidget->appIcon->setPixmap(QPixmap(list[0]).scaled(32, 32));
            }
        }
        allUpdateLayout->addWidget(appWidget);
        qDebug() << "更新管理器：" << "loadingOneUpdateMsgSlot:" << appWidget->dispalyName;
    }

}

void TabWid::loadingFinishedSlot(int size)
{
    if (isAutoUpgrade)
        bacupInit(true);
    else
        bacupInit(false);
    disconnect(updateMutual->interface,SIGNAL(UpdateDetectFinished(bool,QStringList,QString,QString)),this,SLOT(slotUpdateCache(bool,QStringList,QString,QString)));
    disconnect(updateMutual->interface,SIGNAL(UpdateDetectStatusChanged(int,QString)),this,SLOT(slotUpdateCacheProgress(int,QString)));
    if(updateMutual->importantList.size() == 0) {
    //        updateMutual->fileUnLock();
        checkUpdateBtn->setEnabled(true);
        checkUpdateBtn->stop();
        //        checkUpdateBtn->setText(tr("检查更新"));
        checkUpdateBtn->setText(tr("Check Update"));
        //        versionInformationLab->setText(tr("您的系统已是最新！"));
        version = getversion();
        versionInformationLab->setText(tr("Your system is the latest:")+version);
        checkUpdateBtn->hide();
        disconnect(updateMutual->interface,SIGNAL(UpdateDloadAndInstStaChanged(QStringList,int,QString,QString)),this,SLOT(getAllProgress(QStringList,int,QString,QString)));
        QString checkedtime = tr("No information!");
        QSqlQuery query(QSqlDatabase::database("A"));
        query.exec("select * from display");
        while(query.next())
        {
            checkedtime = query.value("check_time").toString();
        }
        lastRefreshTime->setText(tr("Last Checked:")+checkedtime);
        lastRefreshTime->show();
        allProgressBar->hide();
    }
    else
    {
        updateMutual->importantSize = updateMutual->importantList.size();   //此次检测结果的更新数量
        checkUpdateBtn->stop();
        checkUpdateBtn->setEnabled(true);
        //        checkUpdateBtn->setText(tr("全部更新"));
        checkUpdateBtn->setText(tr("UpdateAll"));
        checkUpdateBtn->adjustSize();
        //        versionInformationLab->setText(tr("检测到你的系统有可更新的应用！"));
        QString checkedtime = tr("No information!");
        QSqlQuery query(QSqlDatabase::database("A"));
        query.exec("select * from display");
        while(query.next())
        {
            checkedtime = query.value("check_time").toString();
        }
        lastRefreshTime->setText(tr("Last Checked:")+checkedtime);
        if (!isAutoUpgrade)
        {
            versionInformationLab->setText(tr("Updatable app detected on your system!"));

        }
        systemPortraitLab->setPixmap(QPixmap(":/img/upgrade/update.png").scaled(96,96));
    }

}

void TabWid::allappupdatefinishSlot()
{
//    updateMutual->fileUnLock();
    checkUpdateBtn->setEnabled(true);
    checkUpdateBtn->stop();
    //        checkUpdateBtn->setText(tr("检查更新"));
    checkUpdateBtn->setText(tr("Check Update"));
    version = getversion();
    //        versionInformationLab->setText(tr("您的系统已是最新！"));
    versionInformationLab->setText(tr("Your system is the latest:")+version);
    checkUpdateBtn->hide();
    disconnect(updateMutual->interface,SIGNAL(UpdateDloadAndInstStaChanged(QStringList,int,QString,QString)),this,SLOT(getAllProgress(QStringList,int,QString,QString)));
    allProgressBar->hide();
//    QString updatetime = tr("No Information!");
//    QSqlQuery queryInstall(QSqlDatabase::database("A"));
//    queryInstall.exec("select * from updateinfos order by id desc");
//    while (queryInstall.next()) {
//        QString statusType = queryInstall.value("keyword").toString();
//        if (statusType == "" || statusType =="1") {
//            updatetime = queryInstall.value("date").toString();
//            break;
//        }
//    }
//    lastRefreshTime->setText(tr("Last refresh:")+ updatetime);
    QString checkedtime = tr("No information!");
    QSqlQuery query(QSqlDatabase::database("A"));
    query.exec("select * from display");
    while(query.next())
    {
        checkedtime = query.value("check_time").toString();
    }
    lastRefreshTime->setText(tr("Last Checked:")+checkedtime);
    lastRefreshTime->show();
    allProgressBar->hide();
}

void TabWid::getAllDisplayInformation()
{
    QSqlQuery query(QSqlDatabase::database("A"));
    QString updatetime;
    QString checkedtime;
    QString checkedstatues;
    QString backupStatus;
    QString downloadlimitstatus;
    QString downloadlimitvalue;
    QString allowshutdownupgradestatus;
    QString time;
    query.exec("select * from display");
    while(query.next())
    {
        checkedtime = query.value("check_time").toString();
        checkedstatues = query.value("auto_check").toString();
        backupStatus = query.value("auto_backup").toString();
        downloadlimitstatus = query.value("download_limit").toString();
        downloadlimitvalue = query.value("download_limit_value").toString();
//        allowshutdownupgradestatus = query.value("allow_unattended_upgrades_shutdown").toString();
//        time = query.value("update_period").toString();
    }
    qDebug()<<"downloadlimitstatus:"<<downloadlimitstatus;
    QSqlQuery queryInstall(QSqlDatabase::database("A"));
    updatetime = tr("No Information!");
    queryInstall.exec("select * from updateinfos order by id desc");
    while(queryInstall.next())
    {
        QString statusType = queryInstall.value("keyword").toString();
        if(statusType == "" || statusType =="1") {
            updatetime = queryInstall.value("date").toString();
            break;
        }
    }
    lastRefreshTime->setText(tr("Last Checked:") + checkedtime);
//    versionInformationLab->setText(tr("Last Checked:")+checkedtime);
    if(checkedstatues == "false")
    {
        isAutoCheckSBtn->setChecked(false);
//        ontimeUpgradeBtn->hide();
//        poweroffUpgradeBtn->hide();
//        ontimeUpgradeLab->hide();
//        ontimeUpgradeValue->hide();
    }
    else
    {
        isAutoCheckSBtn->setChecked(true);
//        ontimeUpgradeBtn->show();
//        poweroffUpgradeBtn->show();
//        ontimeUpgradeLab->show();
//        ontimeUpgradeValue->show();
    }
    isAutoCheckSBtn->setEnabled(true);

    if(backupStatus == "false")
    {
        isAutoBackupSBtn->setChecked(false);
    }
    else
    {
        isAutoBackupSBtn->setChecked(true);
    }
    if(downloadlimitstatus == "false")
    {
        DownloadHBtn->setChecked(false);
        DownloadHValue->setCurrentText(downloadlimitvalue+"KB/s");
        DownloadHValue->setEnabled(false);
    }
    else
    {
        DownloadHBtn->setChecked(true);
        DownloadHValue->setEnabled(true);
        DownloadHValue->setCurrentText(downloadlimitvalue+"KB/s");
    }
//    if(allowshutdownupgradestatus == "false")
//    {
//        ontimeUpgradeBtn->setChecked(true);
//        ontimeUpgradeValue->setEnabled(true);
//        ontimeUpgradeValue->setCurrentText(time);
//    }
//    else
//    {
//        poweroffUpgradeBtn->setChecked(true);
//        ontimeUpgradeValue->setEnabled(false);
//    }
}

void TabWid::showHistoryWidget()
{
    buriedSettings("Upgrade","historyLog",QString("clicked"));
    historyLog = m_updatelog::GetInstance(this);
    kdk::UkuiStyleHelper::self()->removeHeader(historyLog);
    //在屏幕中央显示
    //QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
    //historyLog->move((availableGeometry.width()-historyLog->width())/2,(availableGeometry.height()- historyLog->height())/2);
    historyLog->show();
}

void TabWid::showDependSlovePtompt(int updatemode,QStringList pkgname,QStringList description,QStringList deletereason)
{
    dependfixdialog =  new dependencyfixdialog(updatemode);
    qDebug()<<"the updatemode is: "<<updatemode;
    //在屏幕中央显示
    //QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
    //historyLog->move((availableGeometry.width()-historyLog->width())/2,(availableGeometry.height()- historyLog->height())/2);

    /*解决依赖冲突，在全部更新的dependencyfixdialog::fixdependency中没有part更新*/
//    connect(dependfixdialog,&dependencyfixdialog::updatedependsolvecancelsignal,this,&TabWid::updatecancel);
    connect(dependfixdialog,&dependencyfixdialog::updatealldependsolveacceptsignal,this,&TabWid::dependencyupdateAll);
    connect(dependfixdialog,&dependencyfixdialog::disupdatedependsolveacceptsignal,this,&TabWid::disupdateallaccept);
    /*显示包详情*/
    connect(dependfixdialog,SIGNAL(updatedependshowdetailssignal(QStringList,QStringList,QStringList,int)),this,SLOT(showdetaillist(QStringList,QStringList,QStringList,int)));
    /*关闭会话框*/
    connect(dependfixdialog,&dependencyfixdialog::closedpendencyfixdialog,this,&TabWid::closedpendencyfixdialog);
    int pkgnum = dependfixdialog->updatedeletepkglist(pkgname,description,deletereason);//更新列表
    QString pkgnamestr = QString::number(pkgnum);
    if(updatemode==UpdateAll){
//        dependfixdialog->updatemode=UpdateAll;
        dependfixdialog->pkgname=pkgname;
        dependfixdialog->description=description;
        dependfixdialog->deletereason=deletereason;
    }else if(updatemode==UpdatePart)
    {
//        dependfixdialog->updatemode=UpdatePart;
        dependfixdialog->pkgname=pkgname;
        dependfixdialog->description=description;
        dependfixdialog->deletereason=deletereason;
    }
    else if(updatemode==UpdateSystem)
    {
//        dependfixdialog->updatemode=UpdateSystem;
        dependfixdialog->pkgname=pkgname;
        dependfixdialog->description=description;
        dependfixdialog->deletereason=deletereason;
        dependfixdialog->attention->setText(tr("Dependency conflict exists in this update,need to be completely repaired!"));
    }
    dependfixdialog->attention->setText(tr("There are ")+pkgnamestr + tr(" packages going to be removed,Please confirm whether to accept!"));
    dependfixdialog->exec();
}

void TabWid::showFixbrokenPtompt(int updatemode,QStringList pkgname,QStringList description,QStringList deletereason)
{
    fixbrokeninstall = fixbrokeninstalldialog::GetInstance(this);
    //在屏幕中央显示
    //QRect availableGeometry = qApp->primaryScreen()->availableGeometry();
    //historyLog->move((availableGeometry.width()-historyLog->width())/2,(availableGeometry.height()- historyLog->height())/2);
//    connect(fixbrokeninstall,&fixbrokeninstalldialog::updatedependsolvecancelsignal,this,&TabWid::updatecancel);
//    connect(fixbrokeninstall,&fixbrokeninstalldialog::updatealldependsolveacceptsignal,this,&TabWid::updateAllApp);
//    connect(fixbrokeninstall,&fixbrokeninstalldialog::disupdatedependsolveacceptsignal,this,&TabWid::disupdateallaccept);
    int pkgnum = fixbrokeninstall->updatedeletepkglist(pkgname,description,deletereason);//更新列表
    QString pkgnamestr = QString::number(pkgnum);
    if(updatemode==UpdateAll)
    {
        fixbrokeninstall->updatemode=UpdateAll;
    }
    else if(updatemode==UpdatePart)
    {
        fixbrokeninstall->updatemode=UpdatePart;
    }
    else if(updatemode==UpdateSystem)
    {
        fixbrokeninstall->updatemode=UpdateSystem;
        fixbrokeninstall->conflictpromptTab->setText(tr("Dependency conflict exists in this update,need to be completely repaired!"));
    }
    fixbrokeninstall->uninstallinfoTab->setText(pkgnamestr +" "+ tr("packages are going to be removed,Please confirm whether to accept!"));
    fixbrokeninstall->show();
//        connect(fixbrokeninstall,&fixbrokeninstalldialog::updatedependsolvecancelsignal,this,&TabWid::updatecancel);
//        connect(fixbrokeninstall,&fixbrokeninstalldialog::updatealldependsolveacceptsignal,this,&TabWid::updateAllApp);
        connect(fixbrokeninstall,&fixbrokeninstalldialog::fixbrokenpkgkeepbtnclickedsignal,this,&TabWid::fixbrokencancel);
        connect(fixbrokeninstall,&fixbrokeninstalldialog::fixbrokenpkgremovebtnclickedsignal,this,&TabWid::fixbrokenremove);
//        fixbrokenpkgkeepbtnclickedsignal
}

void TabWid::slotReconnTimes(int times) {
    qDebug() << "更新模板失败 ， 重新连接 " << times << "次";
    versionInformationLab->setText(tr("trying to reconnect ") + QString::number(times) + tr(" times"));
}

//void TabWid::showDetails()
//{
//    if(detailLabel->isHidden())
//    {
//        detailLabel->show();
//        progressLabel->show();
//        //        updatedetaileInfo->setText(tr("收起"));
////        updatedetaileInfo->setText(tr("back"));
//    }
//    else
//    {
//        detailLabel->hide();
//        progressLabel->hide();
//        //        updatedetaileInfo->setText(tr("详情"));
//        //updatedetaileInfo->setText(tr("details"));
//    }
//}

void TabWid::checkUpdateBtnClicked()
{
    updateMutual = UpdateDbus::getInstance();

    if(checkUpdateBtn->text() == tr("Cancel"))
    {
        updateMutual->interface->call("CancelDownload");
        isCancel=true;
        return;
    }

    /*加入自动更新功能安装时在systemwidget中的显示*/
    QFile file("/var/lib/unattended-upgrades/unattended-upgrade.conf");
    qDebug()<<"进入button click";
    if(file.exists())
    {
        qDebug()<<"标志位文件存在";
        QSettings settings("/var/lib/unattended-upgrades/unattended-upgrade.conf",QSettings::IniFormat);
        QString ret = settings.value("UNATTENDED_UPGRADE/autoupdate_run_status").toString();

        if (!ret.compare("backup"))
        {
//            versionInformationLab->setText("后台自动更新进程正在备份中......");
            versionInformationLab->setText(tr("Auto-Update is backing up......"));
            checkUpdateBtn->hide();
            QDBusConnection::systemBus().connect(QString(),QString("/"),"com.kylin.install.notification","InstallStart",this,SLOT(autoupgradestatusshow()));
            QDBusConnection::systemBus().connect(QString(),QString("/"),"com.kylin.install.notification","BackupFailure",this,SLOT(backupfailure()));
            return;
        }
        if (!ret.compare("install"))
        {
            qDebug()<<"开始安装";
            autoupgradestatusshow();
            return;
        }
        if (!ret.compare("download"))/*需要杀掉程序*/
        {
            /*如果自动更新在下载中，调用dbus去kill掉下载程序，继续原流程，不进行多余操作*/
            QString pid;
            QFile pidfile("/var/run/unattended-upgrades.pid");
            if (pidfile.exists())
            {
                pidfile.open(QIODevice::ReadOnly);//| QIODevice::Text | QIODevice::Append
                pid = pidfile.readAll();
                pidfile.close();
                if(pid == nullptr)
                {
                    return;
                }
//                pidfile.close();
                updateSource->killProcessSignal(pid.toInt(), 10);
            }

            checkUpdateBtn->setEnabled(true);
            checkUpdateBtn->setText(tr("Check Update"));
        }
        if (!ret.compare("preinstall"))/*需要杀掉程序*/
        {
            /*如果自动更新在下载中，调用dbus去kill掉下载程序，继续原流程，不进行多余操作*/
            QString pid;
            QFile pidfile("/var/run/unattended-upgrades.pid");
            if (pidfile.exists())
            {
                pidfile.open(QIODevice::ReadOnly);//| QIODevice::Text | QIODevice::Append
                pid = pidfile.readAll();
                pidfile.close();
                if(pid == nullptr)
                {
                    return;
                }
//                pidfile.close();
                updateSource->killProcessSignal(pid.toInt(), 10);
            }

            checkUpdateBtn->setEnabled(true);
            checkUpdateBtn->setText(tr("Check Update"));
        }
        if (!ret.compare("idle")) {
            /*如果没有进行自动更新，那就不需要操作 */
//            checkUpdateBtn->setEnabled(true);
//            checkUpdateBtn->setText(tr("Check Update"));
        }
    }


    QDBusInterface upfc("com.kylin.systemupgrade",
                         "/com/kylin/systemupgrade",
                         "com.kylin.systemupgrade.interface",
                         QDBusConnection::systemBus());

    QDBusMessage msg = upfc.call("GetBackendStatus",getLanguageEnv());
    status = msg.errorMessage().isEmpty();
    if(!status)
    {
        qDebug()<<"the dbus is not start";
        versionInformationLab->setText(tr("The updater is NOT start"));
        checkUpdateBtn->hide();
        return;
    }


    QDBusReply<qint32> obj_reply = updateMutual->interface->call("GetBackendStatus",getLanguageEnv());
    int backendStatus = obj_reply.value();
    switch(backendStatus)
    {
        qDebug()<<"backendStatus is "<<backendStatus;
        case ACTION_DEFUALT_STATUS:
        break;
        case ACTION_UPDATE:
            qDebug()<<"access to ACTION_UPDATE";
            versionInformationLab->setText(tr("The progress is updating..."));
            checkUpdateBtn->hide();
            connect(updateMutual->interface,SIGNAL(UpdateDetectFinished(bool,QStringList,QString,QString)),this,SLOT(slotUpdateCache(bool,QStringList,QString,QString)));
            connect(updateMutual->interface,SIGNAL(UpdateDetectStatusChanged(int,QString)),this,SLOT(slotUpdateCacheProgress(int,QString)));
            return;
        break;
        case ACTION_INSTALL || ACTION_DOWNLOADONLY:
            isAllUpgrade = true;
            isContinueUpgrade = true;
            versionInformationLab->setText(tr("The progress is installing..."));
            checkUpdateBtn->hide();
            connect(updateMutual->interface,SIGNAL(Cancelable(bool)),this,SLOT(isCancelabled(bool)));
            connect(updateMutual->interface,SIGNAL(UpdateDloadAndInstStaChanged(QStringList,int,QString,QString)),this,SLOT(getAllProgress(QStringList,int,QString,QString)));
            connect(updateMutual->interface,SIGNAL(UpdateInstallFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlot(bool,QStringList,QString,QString)));
            connect(updateMutual->interface,SIGNAL(UpdateDownloadFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlotindownload(bool,QStringList,QString,QString)));
            return;
        break;
        default:
            versionInformationLab->setText(tr("The updater is busy！"));
            return;
        break;
    }
    if(checkUpdateBtn->text() == tr("Retry"))
    {
        widgetList.clear();
        qDebug()<<"access to Retry";
        connect(updateMutual->interface,SIGNAL(UpdateDetectFinished(bool,QStringList,QString,QString)),this,SLOT(slotUpdateCache(bool,QStringList,QString,QString)));
        connect(updateMutual->interface,SIGNAL(UpdateDetectStatusChanged(int,QString)),this,SLOT(slotUpdateCacheProgress(int,QString)));
        updateMutual->failedList.clear();
        QList<AppUpdateWid*> list = this->findChildren<AppUpdateWid*>();
        for(AppUpdateWid* tmp:list)        {
            tmp->deleteLater();
        }
        updateMutual->callDBusUpdateSource();
        versionInformationLab->setText(tr("Updating the software source")+"...");
        checkUpdateBtn->start();
        checkUpdateBtn->setEnabled(false);
    }
    if(checkUpdateBtn->text() == tr("Check Update"))
    {
        widgetList.clear();
        qDebug()<<"access to Check Update";
        connect(updateMutual->interface,SIGNAL(UpdateDetectFinished(bool,QStringList,QString,QString)),this,SLOT(slotUpdateCache(bool,QStringList,QString,QString)));
        connect(updateMutual->interface,SIGNAL(UpdateDetectStatusChanged(int,QString)),this,SLOT(slotUpdateCacheProgress(int,QString)));
        updateMutual->failedList.clear();
        QList<AppUpdateWid*> list = this->findChildren<AppUpdateWid*>();
        for(AppUpdateWid* tmp:list)        {
            tmp->deleteLater();
        }
        updateMutual->callDBusUpdateSource();
        versionInformationLab->setText(tr("Updating the software source")+"...");
        checkUpdateBtn->start();
        checkUpdateBtn->setEnabled(false);
    }
    else if(checkUpdateBtn->text() == tr("UpdateAll"))
    {
        if (!get_battery())
        {
            QMessageBox msgBox;
//            msgBox.setText(("<font size='21'>"+tr("The update stopped because of low battery.")+"</font>")+'\n'+
//                           ("<font size='11'>"+tr("The battery is below 50% and the update cannot be downloaded")+"</font>"));
            msgBox.setText(tr("The update stopped because of low battery."));
            msgBox.setInformativeText(tr("The system update requires that the battery power is not less than 50%"));
            msgBox.setIcon(QMessageBox::Warning);
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setButtonText(QMessageBox::Ok,tr("OK"));
            msgBox.exec();
            return;
        }


        QMessageBox msgBox(qApp->activeModalWidget());
        msgBox.setText(tr("Please back up the system before all updates to avoid unnecessary losses"));
        msgBox.setWindowTitle(tr("Prompt information"));
        msgBox.setIcon(QMessageBox::Icon::Information);

        msgBox.addButton(tr("Cancel"),QMessageBox::NoRole);//QMessageBox::NoRole
        msgBox.addButton(tr("Only Update"),QMessageBox::YesRole); //QMessageBox::YesRole
        //msgBox.addButton(tr("Back And Update"),QMessageBox::AcceptRole);      //先去掉备份按钮
        connect(updateMutual->interface,SIGNAL(InstallDetectStatus(bool,QString)),this,SLOT(InstallStatus(bool,QString)));
        connect(updateMutual->interface,SIGNAL(Cancelable(bool)),this,SLOT(isCancelabled(bool)));
        isAllUpgrade = true;
        int ret = msgBox.exec();
        switch (ret)
        {
            case 0:
//              foreach (AppUpdateWid *wid, widgetList)
//              {
//                  //disconnect(wid, &AppUpdateWid::sendProgress, this, &TabWid::getAllProgress);
//                  wid->updateAPPBtn->show();
//              }
                disconnect(updateMutual->interface,SIGNAL(UpdateDloadAndInstStaChanged(QStringList,int,QString,QString)),this,SLOT(getAllProgress(QStringList,int,QString,QString)));
                isAllUpgrade = false;
                qDebug() << "Close 暂不更新!";
                break;
            case 1:
                qDebug() << "全部更新。。。。。。";
                foreach (AppUpdateWid *wid, widgetList)
                {
//                  if(wid->updateAPPBtn->text() == tr("Update"))
                     wid->updateAPPBtn->setEnabled(false);
                }
                isAutoBackupSBtn->setChecked(false);
                checkUpdateBtn->setEnabled(false);
                checkUpdateBtn->start();
                updateMutual->isPointOutNotBackup = false;   //全部更新时不再弹出单个更新未备份提示
                emit updateAllSignal(false);
                break;
            case 2:
                versionInformationLab->setText(tr("Calculating Capacity..."));
                checkUpdateBtn->hide();
                bacupInit(true);
                backupCore();
                qDebug() << "否，立即备份";
                break;
            default:
//                foreach (AppUpdateWid *wid, widgetList) {
//                    wid->updateAPPBtn->show();
//                }
                disconnect(updateMutual->interface,SIGNAL(UpdateDloadAndInstStaChanged(QStringList,int,QString,QString)),this,SLOT(getAllProgress(QStringList,int,QString,QString)));
                isAllUpgrade = false;
                qDebug() << "Close 暂不更新!";
                break;
         }
    }

}



void TabWid::DownloadLimitValueChanged(const QString &value)
{
    buriedSettings("Upgrade", QString("DownloadHValue"), QString("select"), value);
    if(DownloadHBtn->isChecked()==false)
    {
        updateMutual->SetDownloadLimit(0,false);
        updateMutual->insertInstallStates("download_limit_value","0");
    }
    else if (DownloadHBtn->isChecked()==true)
    {
        QString speedvalue = value;
        QString limit = speedvalue.replace("KB/s","");
        //int dlimit = DownloadLimitValue->value();
        updateMutual->SetDownloadLimit(limit,true);
        updateMutual->insertInstallStates("download_limit_value",limit);
    }
    else
    {
        qDebug()<<"Download Limit Changed";
        updateMutual->SetDownloadLimit(0,false);
        updateMutual->insertInstallStates("download_limit_value","0");
    }
}

void TabWid::DownloadLimitChanged()
{
    if(DownloadHBtn->isChecked() == false)
    {
        qDebug()<<"download limit disabled";
//        DownloadHValue->hide();
        DownloadHValue->setEnabled(false);
        updateMutual->SetDownloadLimit(0,false);
        updateMutual->insertInstallStates("download_limit","false");

    }
    else if(DownloadHBtn->isChecked() == true)
    {
        updateMutual->insertInstallStates("download_limit","true");
        qDebug()<<"download limit enabled";
//        DownloadHValue->show();
        DownloadHValue->setEnabled(true);
        QString dlimit = DownloadHValue->currentText();
        QString limit = dlimit.replace(QString("KB/s"),"");
        updateMutual->SetDownloadLimit(limit,true);
    }
}

void TabWid::isAutoCheckedChanged()     //自动检测按钮绑定的槽函数
{
    if(isAutoCheckSBtn->isChecked() == false)
    {
        updateMutual->insertInstallStates("auto_check","false");

    }
    else if(isAutoCheckSBtn->isChecked() == true)
    {
        updateMutual->insertInstallStates("auto_check","true");
    }
}

void TabWid::isAutoBackupChanged()
{
    if(isAutoBackupSBtn->isChecked() == false)
    {
        updateMutual->insertInstallStates("auto_backup","false");

    }
    else if(isAutoBackupSBtn->isChecked() == true)
    {
        updateMutual->insertInstallStates("auto_backup","true");
    }
}

void TabWid::slotCancelDownload()
{
    checkUpdateBtn->setEnabled(true);
    //    checkUpdateBtn->setText("全部更新");
    checkUpdateBtn->setText(tr("UpdateAll"));
    checkUpdateBtn->adjustSize();
    checkUpdateBtn->setCheckable(true);
//    versionInformationLab->setText(tr("update has been canceled!"));
//    versionInformationLab->setToolTip("");
    foreach (AppUpdateWid *wid, widgetList) {
        wid->updateAPPBtn->setEnabled(true);
    }
}

void TabWid::startoneappupdateslot()
{
    isAllUpgrade=false;
//    connect(updateMutual->interface,SIGNAL(Cancelable(bool)),this,SLOT(isCancelabled(bool)));
}

void TabWid::hideUpdateBtnSlot(bool state,QStringList pkgname,QString error,QString reason)
{
        Q_UNUSED(pkgname);
        bacupInit(false);
        isAllUpgrade = false;
        alldownloadstart = false;
        checkUpdateBtn->setEnabled(true);
        checkUpdateBtn->stop();
        //        checkUpdateBtn->setText(tr("检查更新"));
        checkUpdateBtn->setText(tr("Check Update"));
        if(state) {
            if(isContinueUpgrade)
            {
                versionInformationLab->setText(tr("This update has been completed！"));
            }else
            {
                version = getversion();
                versionInformationLab->setText(tr("Your system is the latest:")+version);
            }
            isContinueUpgrade=false;
//            versionInformationLab->setToolTip("");
            systemPortraitLab->setPixmap(QPixmap(":/img/upgrade/normal.png").scaled(96,96));
            checkUpdateBtn->hide();
            allProgressBar->hide();
//            updatedetaileInfo->hide();
//            detailLabel->hide();
//            updateMutual->fileUnLock();
        }
        else {

            if(isCancel)
            {
                isCancel=false;
                versionInformationLab->setText(tr("update has been canceled!"));
//                versionInformationLab->setToolTip("");
                checkUpdateBtn->hide();
//                return;
            }else{
                versionInformationLab->setText(tr("Part of the update failed!"));
                versionInformationLab->setToolTip(tr("Failure reason:")+ "\r\n"+reason);
                checkUpdateBtn->hide();
            }
//            updateMutual->fileUnLock();
            allProgressBar->hide();
//            updatedetaileInfo->hide();
//            detailLabel->hide();
            lastRefreshTime->setText(error);
        }
        disconnect(updateMutual->interface,SIGNAL(UpdateInstallFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlot(bool,QStringList,QString,QString)));
        disconnect(updateMutual->interface,SIGNAL(UpdateDownloadFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlotindownload(bool,QStringList,QString,QString)));
//        QString updatetime = tr("No Information!");
//        QSqlQuery queryInstall(QSqlDatabase::database("A"));
//        queryInstall.exec("select * from updateinfos order by id desc");
//        while (queryInstall.next())
//        {
//            QString statusType = queryInstall.value("keyword").toString();
//            if(statusType == "" || statusType =="1")
//            {
//                updatetime = queryInstall.value("date").toString();
//                break;
//            }
//        }
        QString checkedtime = tr("No information!");
        QSqlQuery query(QSqlDatabase::database("A"));
        query.exec("select * from display");
        while(query.next())
        {
            checkedtime = query.value("check_time").toString();
        }
        lastRefreshTime->setText(tr("Last Checked:")+checkedtime);
//        lastRefreshTime->setText(tr("Last refresh:")+updatetime);
        lastRefreshTime->show();
        disconnect(updateMutual->interface,SIGNAL(Cancelable(bool)),this,SLOT(isCancelabled(bool)));
        disconnect(updateMutual->interface,SIGNAL(UpdateDependResloveStatus(bool,bool,QStringList,QStringList,QStringList,QString,QString)),this,SLOT(DependResloveResult(bool,bool,QStringList,QStringList,QStringList,QString,QString)));
        disconnect(updateMutual->interface,SIGNAL(UpdateDloadAndInstStaChanged(QStringList,int,QString,QString)),this,SLOT(getAllProgress(QStringList,int,QString,QString)));
        disconnect(updateMutual->interface,SIGNAL(UpdateDownloadInfo(QStringList,int,int,uint,uint,int)),this,SLOT(showDownloadInfo(QStringList,int,int,uint,uint,int)));
        disconnect(updateMutual->interface,SIGNAL(UpdateInstallFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlot(bool,QStringList,QString,QString)));
        disconnect(updateMutual->interface,SIGNAL(UpdateDownloadFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlotindownload(bool,QStringList,QString,QString)));
        /*在安装过程完成后断开安装的链接*/
//        disconnect(m_instance,&updatedeleteprompt::updatepromptsolvecancelsignal,this,&TabWid::closedpendencyfixdialog);
//        disconnect(m_instance,&updatedeleteprompt::updatedeletepromptacceptsignal,this,&TabWid::updatedeletepromptaccept);
//        disconnect(m_instance,&updatedeleteprompt::disupdatedependsolveacceptsignal,this,&TabWid::disupdateallaccept);
}

void TabWid::hideUpdateBtnSlotindownload(bool state,QStringList pkgname,QString error,QString reason)
{
        Q_UNUSED(pkgname);
        bacupInit(false);
        isAllUpgrade = false;
        alldownloadstart = false;
        checkUpdateBtn->setEnabled(true);
        checkUpdateBtn->stop();
        //        checkUpdateBtn->setText(tr("检查更新"));
        checkUpdateBtn->setText(tr("Check Update"));
        lastRefreshTime->setText(tr("Finish the download!"));
        if(state) {
            if(isContinueUpgrade)
            {
                versionInformationLab->setText(tr("This update has been completed！"));
            }
            else
            {
                qDebug()<<"44444444444";

                QDBusInterface upgrade_interface( "com.kylin.systemupgrade",
                                             "/com/kylin/systemupgrade",
                                             "com.kylin.systemupgrade.interface",
                                             QDBusConnection::systemBus() );
                QDBusMessage modelist = upgrade_interface.call("GetConfigValue","InstallMode","shutdown_install");
                QString  mode =modelist.arguments().value(1).toString();
                qDebug()<<mode;
                if(mode =="True")
                {
                    versionInformationLab->setText(tr("The system has download the update!"));
                    QMessageBox msgBox(qApp->activeModalWidget());
                    msgBox.setText(tr("It's need to reboot to make the install avaliable"));
                    msgBox.setWindowTitle(tr("Reboot notification"));
                    msgBox.setIcon(QMessageBox::Icon::Information);
                    msgBox.addButton(tr("Reboot rightnow"), QMessageBox::YesRole);
                    msgBox.addButton(tr("Later"), QMessageBox::NoRole);

                    int ret = msgBox.exec();
                    switch (ret)
                    {
                    case 0:
                        qDebug() << "Reboot rightnow";
                        system("ukui-session-tools --reboot");
                        break;
                    case 1:
                        qDebug() << "Later。。。。。。";
                        break;
                    default:
                        break;
                    }
                }
                else
                {
                    version = getversion();
                    versionInformationLab->setText(tr("Your system is the latest:")+version);
                }
            }
            isContinueUpgrade=false;
//            versionInformationLab->setToolTip("");
            systemPortraitLab->setPixmap(QPixmap(":/img/upgrade/normal.png").scaled(96,96));
            checkUpdateBtn->hide();
            allProgressBar->hide();
//            updatedetaileInfo->hide();
//            detailLabel->hide();
//            updateMutual->fileUnLock();
        }
        else {

            if(isCancel)
            {
                isCancel=false;
                versionInformationLab->setText(tr("update has been canceled!"));
//                versionInformationLab->setToolTip("");
                checkUpdateBtn->hide();
//                return;
            }else{
                versionInformationLab->setText(tr("Part of the update failed!"));
                versionInformationLab->setToolTip(tr("Failure reason:")+ "\r\n"+reason);
                checkUpdateBtn->hide();
            }
//            updateMutual->fileUnLock();
            allProgressBar->hide();
//            updatedetaileInfo->hide();
//            detailLabel->hide();
            lastRefreshTime->setText(error);
        }
        disconnect(updateMutual->interface,SIGNAL(UpdateInstallFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlot(bool,QStringList,QString,QString)));
        disconnect(updateMutual->interface,SIGNAL(UpdateDownloadFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlotindownload(bool,QStringList,QString,QString)));
//        QString updatetime = tr("No Information!");
//        QSqlQuery queryInstall(QSqlDatabase::database("A"));
//        queryInstall.exec("select * from updateinfos order by id desc");
//        while (queryInstall.next()) {
//            QString statusType = queryInstall.value("keyword").toString();
//            if(statusType == "" || statusType =="1") {
//                updatetime = queryInstall.value("date").toString();
//                break;
//            }
//        }
        QString checkedtime = tr("No information!");
        QSqlQuery query(QSqlDatabase::database("A"));
        query.exec("select * from display");
        while(query.next())
        {
            checkedtime = query.value("check_time").toString();
        }
        lastRefreshTime->setText(tr("Last Checked:")+checkedtime);
//        lastRefreshTime->setText(tr("Last refresh:")+updatetime);
        lastRefreshTime->show();
        disconnect(updateMutual->interface,SIGNAL(Cancelable(bool)),this,SLOT(isCancelabled(bool)));
        disconnect(updateMutual->interface,SIGNAL(UpdateDependResloveStatus(bool,bool,QStringList,QStringList,QStringList,QString,QString)),this,SLOT(DependResloveResult(bool,bool,QStringList,QStringList,QStringList,QString,QString)));
        disconnect(updateMutual->interface,SIGNAL(UpdateDloadAndInstStaChanged(QStringList,int,QString,QString)),this,SLOT(getAllProgress(QStringList,int,QString,QString)));
        disconnect(updateMutual->interface,SIGNAL(UpdateDownloadInfo(QStringList,int,int,uint,uint,int)),this,SLOT(showDownloadInfo(QStringList,int,int,uint,uint,int)));
        disconnect(updateMutual->interface,SIGNAL(UpdateInstallFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlot(bool,QStringList,QString,QString)));
        disconnect(updateMutual->interface,SIGNAL(UpdateDownloadFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlotindownload(bool,QStringList,QString,QString)));
 }


void TabWid::oneappUpdateresultSlot(bool state,QStringList pkgname,QString error,QString reason)
{
    Q_UNUSED(pkgname);
        checkUpdateBtn->setEnabled(true);
        checkUpdateBtn->stop();
        //        checkUpdateBtn->setText(tr("检查更新"));
        checkUpdateBtn->setText(tr("UpdateAll"));
        checkUpdateBtn->adjustSize();
        foreach (AppUpdateWid *wid, widgetList) {
            wid->updateAPPBtn->setEnabled(true);
        }
        if(state) {
            versionInformationLab->setText(tr("Part of the update success!"));
//            versionInformationLab->setToolTip("");
            systemPortraitLab->setPixmap(QPixmap(":/img/upgrade/normal.png").scaled(96,96));
            checkUpdateBtn->show();
            allProgressBar->hide();
            lastRefreshTime->hide();
//            updatedetaileInfo->hide();
//            detailLabel->hide();
//            updateMutual->fileUnLock();
        }
        else {

            if(isCancel)
            {
                isCancel=false;
                versionInformationLab->setText(tr("update has been canceled!"));
//                versionInformationLab->setToolTip("");
                checkUpdateBtn->show();
                return;
            }else{
                versionInformationLab->setText(tr("Part of the update failed!"));
                versionInformationLab->setToolTip(tr("Failure reason:")+ "\r\n"+reason);
                checkUpdateBtn->show();
            }
//            updateMutual->fileUnLock();
            allProgressBar->hide();
//            updatedetaileInfo->hide();
//            detailLabel->hide();
            lastRefreshTime->setText(error);
        }
        if(updateMutual->importantList.size() == 0 && updateMutual->failedList.size()==0)
        {
            checkUpdateBtn->setEnabled(true);
            checkUpdateBtn->stop();
            //        checkUpdateBtn->setText(tr("检查更新"));
            checkUpdateBtn->setText(tr("Check Update"));
            QDBusInterface upgrade_interface( "com.kylin.systemupgrade",
                                                     "/com/kylin/systemupgrade",
                                                     "com.kylin.systemupgrade.interface",
                                                     QDBusConnection::systemBus() );
                        QDBusMessage modelist = upgrade_interface.call("GetConfigValue","InstallMode","shutdown_install");
                        QString  mode =modelist.arguments().value(1).toString();
                        qDebug()<<mode;
                        if(mode =="True")
                        {
                            versionInformationLab->setText(tr("All the update has been downloaded."));
                        }
                        else
                        {
                            version = getversion();
                            versionInformationLab->setText(tr("Your system is the latest:")+version);
                        }

//            versionInformationLab->setToolTip("");
            systemPortraitLab->setPixmap(QPixmap(":/img/upgrade/normal.png").scaled(96,96));
            checkUpdateBtn->hide();
            allProgressBar->hide();
//            updatedetaileInfo->hide();
//            detailLabel->hide();
//            updateMutual->fileUnLock();
        }
        disconnect(updateMutual->interface,SIGNAL(UpdateInstallFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlot(bool,QStringList,QString,QString)));
        disconnect(updateMutual->interface,SIGNAL(UpdateDownloadFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlotindownload(bool,QStringList,QString,QString)));
//        QString updatetime = tr("No Information!");
//        QSqlQuery queryInstall(QSqlDatabase::database("A"));
//        queryInstall.exec("select * from updateinfos order by id desc");
//        while (queryInstall.next()) {
//            QString statusType = queryInstall.value("keyword").toString();
//            if(statusType == "" || statusType =="1") {
//                updatetime = queryInstall.value("date").toString();
//                break;
//            }
//        }
//        lastRefreshTime->setText(tr("Last refresh:")+updatetime);
        QString checkedtime = tr("No information!");
        QSqlQuery query(QSqlDatabase::database("A"));
        query.exec("select * from display");
        while(query.next())
        {
            checkedtime = query.value("check_time").toString();
        }
        lastRefreshTime->setText(tr("Last Checked:")+checkedtime);
        lastRefreshTime->show();
        disconnect(updateMutual->interface,SIGNAL(Cancelable(bool)),this,SLOT(isCancelabled(bool)));

}

void TabWid::shutdowninstallfinishslot()
{
    versionInformationLab->setText(tr("Part of the update success!"));
    checkUpdateBtn->setEnabled(true);

    qDebug()<<updateMutual->importantList;
    qDebug()<<appAllMsg.name;
    updateMutual->importantList.removeOne(appAllMsg.name);
    updateMutual->failedList.removeOne(appAllMsg.name);

    foreach (AppUpdateWid *wid, widgetList)
    {
//                if(wid->updateAPPBtn->text() == tr("Update"))
            wid->updateAPPBtn->setEnabled(true);
    }
    if(updateMutual->importantList.size() == 0 && updateMutual->failedList.size()==0)
    {
        qDebug()<<"access to the allllllllllllllllllllll";
        checkUpdateBtn->setEnabled(true);
        checkUpdateBtn->stop();
        //        checkUpdateBtn->setText(tr("检查更新"));
        checkUpdateBtn->setText(tr("Check Update"));
        version = getversion();
        versionInformationLab->setText(tr("Your system is the latest:")+version);
//            versionInformationLab->setToolTip("");
        systemPortraitLab->setPixmap(QPixmap(":/img/upgrade/normal.png").scaled(96,96));
        checkUpdateBtn->hide();
        allProgressBar->hide();
//        updatedetaileInfo->hide();
//        detailLabel->hide();
//            updateMutual->fileUnLock();
    }
//    else
//    {
//        int size1 = updateMutual->importantList.size();
//        qDebug()<<"!!!!!!!!!!!!!!!!!!!!!!!"<<size1;
//    }
 }


void TabWid::updatecancel(void)
{
    qDebug()<<"get the signal";
    isAllUpgrade=false;
    checkUpdateBtn->setEnabled(true);
    checkUpdateBtn->stop();
    //        checkUpdateBtn->setText(tr("检查更新"));
    checkUpdateBtn->setText(tr("UpdateAll"));
    checkUpdateBtn->adjustSize();
    versionInformationLab->setText(tr("update has been canceled!"));
//    versionInformationLab->setToolTip("");
    checkUpdateBtn->show();

    foreach (AppUpdateWid *wid, widgetList) {
//        if(wid->updateAPPBtn->text() == tr("Update"))
            wid->updateAPPBtn->setEnabled(true);
    }

    disconnect(updateMutual->interface,SIGNAL(UpdateDependResloveStatus(bool,bool,QStringList,QStringList,QStringList,QString,QString)),this,SLOT(DependResloveResult(bool,bool,QStringList,QStringList,QStringList,QString,QString)));
    disconnect(updateMutual->interface,SIGNAL(UpdateDloadAndInstStaChanged(QStringList,int,QString,QString)),this,SLOT(getAllProgress(QStringList,int,QString,QString)));
    disconnect(updateMutual->interface,SIGNAL(UpdateDownloadInfo(QStringList,int,int,uint,uint,int)),this,SLOT(showDownloadInfo(QStringList,int,int,uint,uint,int)));
    disconnect(updateMutual->interface,SIGNAL(UpdateInstallFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlotindownload(bool,QStringList,QString,QString)));
    disconnect(updateMutual->interface,SIGNAL(UpdateDownloadFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlot(bool,QStringList,QString,QString)));
//    disconnect(dependfixdialog,&dependencyfixdialog::updatedependsolvecancelsignal,this,&TabWid::updatecancel);
//    disconnect(dependfixdialog,&dependencyfixdialog::updatealldependsolveacceptsignal,this,&TabWid::updateAllApp);
//    disconnect(dependfixdialog,&dependencyfixdialog::disupdatedependsolveacceptsignal,this,&TabWid::disupdateallaccept);
                //    connect(dependfixdialog,&dependencyfixdialog::updatedependshowdetailssignal,this,&TabWid::showdetaillist);
//    disconnect(dependfixdialog,SIGNAL(updatedependshowdetailssignal(QStringList,QStringList,QStringList,int)),this,SLOT(showdetaillist(QStringList,QStringList,QStringList,int)));
//    disconnect(dependfixdialog,&dependencyfixdialog::closedpendencyfixdialog,this,&TabWid::closedpendencyfixdialog);
}

void TabWid::changeUpdateAllSlot(bool isUpdate)
{
    if (isUpdate) {
        isAllUpgrade=false;
        checkUpdateBtn->setEnabled(false);
//        versionInformationLab->setText(tr("Being updated..."));
//        versionInformationLab->setToolTip("");
        foreach (AppUpdateWid *wid, widgetList) {
                wid->updateAPPBtn->setEnabled(false);
        }
    } else  {
        if(checkUpdateBtn->isEnabled() == false)
        {
            checkUpdateBtn->setText(tr("UpdateAll"));
            checkUpdateBtn->setEnabled(true);
            checkUpdateBtn->adjustSize();
        }
    }
}


void TabWid::waitCrucialInstalled()
{
    if(fileLockedStatus == false)
    {
        QString msg = tr("An important update is in progress, please wait.");
        //        QString msg = tr("正在进行一项重要更新，请等待。");
        updateMutual->onRequestSendDesktopNotify(msg);
        versionInformationLab->setText(msg);
//        versionInformationLab->setToolTip("");
        fileLockedStatus = true;
    }
}

void TabWid::getReplyFalseSlot()
{
    isConnectSourceSignal = true;
}

void TabWid::receiveBackupStartResult(int result)
{
    switch (result)
    { 
    case 20:
    {
        QMessageBox msgBox;
        msgBox.setText(tr("insufficient backup space"));
        msgBox.setWindowTitle(tr("Prompt information"));
        msgBox.setIcon(QMessageBox::Icon::Information);
        msgBox.addButton(tr("OK"), QMessageBox::AcceptRole);
        msgBox.addButton(tr("Cancel"), QMessageBox::RejectRole);
        msgBox.setInformativeText(tr("plase clean up your disk or expand the backup space"));
        msgBox.exec();
        versionInformationLab->setText(tr("insufficient backup space"));
        checkUpdateBtn->show();
//      versionInformationLab->setToolTip("");

    }
        break;
    case 30:
    case 31:
        break;
    default:
        versionInformationLab->setText(tr("backup failed"));
        checkUpdateBtn->show();
        break;
    /*case int(backuptools::backup_result::BACKUP_START_SUCCESS):
        //        versionInformationLab->setText(tr("开始备份，正在获取进度")+"...");
        versionInformationLab->setText(tr("Start backup,getting progress")+"...");
//        checkUpdateBtn->start();
//        checkUpdateBtn->setEnabled(false);
//        checkUpdateBtn->hide();
        foreach (AppUpdateWid *wid, widgetList)
        {
//            if(wid->updateAPPBtn->text() == tr("Update"))
                wid->updateAPPBtn->setEnabled(false);
        }
        return;
    case int(backuptools::backup_result::WRITE_STORAGEINFO_ADD_ITEM_FAIL):
    case int(backuptools::backup_result::WRITE_STORAGEINFO_UPDATE_ITEM_FAIL):
        //        backupMessageBox(tr("写入配置文件失败，本次更新不会备份系统！"));
        backupMessageBox(tr("Failed to write configuration file, this update will not back up the system!"));
        break;
    case int(backuptools::backup_result::BACKUP_CAPACITY_IS_NOT_ENOUGH):
        //        backupMessageBox(tr("备份空间不足，本次更新不会备份系统！"));
//        backupMessageBox(tr("Insufficient backup space, this update will not backup your system!"));

        break;
    case int(backuptools::backup_result::INC_NOT_FOUND_UUID):
        //        backupMessageBox(tr("麒麟备份还原工具无法找到UUID，本次更新不会备份系统!"));
        backupMessageBox(tr("Kylin backup restore tool could not find the UUID, this update will not backup the system!"));
        break;
    default:
//        backupMessageBox(tr("The system backup partition is not detected. Do you want to continue updating?"));
        break;*/
    }
//    disconnect(backup, &BackUp::backupStartRestult, this, &TabWid::receiveBackupStartResult);
}

void TabWid::whenStateIsDuing()
{
    versionInformationLab->setText(tr("Calculating Capacity..."));
//    checkUpdateBtn->start();
//    checkUpdateBtn->setEnabled(false);
    checkUpdateBtn->hide();
}

bool TabWid::get_battery()
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

void TabWid::fileLock()
{
    QDir dir("/tmp/auto-upgrade/");
    if(! dir.exists()) {
        dir.mkdir("/tmp/auto-upgrade/");//只创建一级子目录，即必须保证上级目录存在
        chmod("/tmp/auto-upgrade/",0777);
    }
    umask(0000);
    int fd = open("/tmp/auto-upgrade/ukui-control-center.lock", O_RDONLY | O_CREAT,0666);
    if (fd < 0) {
        qDebug()<<"解锁时文件锁打开异常";
        return;
    }
    flock(fd, LOCK_EX | LOCK_NB);
}

void TabWid::fileUnLock()
{
    QDir dir("/tmp/auto-upgrade/");
    if(! dir.exists()) {
        dir.mkdir("/tmp/auto-upgrade/");//只创建一级子目录，即必须保证上级目录存在
        chmod("/tmp/auto-upgrade/",0777);
    }
    umask(0000);
    int fd = open("/tmp/auto-upgrade/ukui-control-center.lock", O_RDONLY | O_CREAT,0666);
    if (fd < 0) {
        qDebug()<<"解锁时文件锁打开异常";
        return;
    }
    flock(fd, LOCK_UN);
}

void TabWid::dependencyupdateAll(bool status)
{
    allProgressBar->setState(NormalProgress);
/*        if(!status){
            checkUpdateBtn->setText(tr("Calculating"));
            connect(updateMutual->interface,SIGNAL(UpdateDependResloveStatus(bool,bool,QStringList,QStringList,QStringList,QString,QString)),this,SLOT(DependResloveResult(bool,bool,QStringList,QStringList,QStringList,QString,QString)));

        }else*/{
            foreach (AppUpdateWid *wid, widgetList) {
                wid->updateAPPBtn->hide();
                wid->hide();
            }
            connect(updateMutual->interface,SIGNAL(UpdateDloadAndInstStaChanged(QStringList,int,QString,QString)),this,SLOT(getAllProgress(QStringList,int,QString,QString)));
            connect(updateMutual->interface,SIGNAL(UpdateDownloadInfo(QStringList,int,int,uint,uint,int)),this,SLOT(showDownloadInfo(QStringList,int,int,uint,uint,int)));
            connect(updateMutual->interface,SIGNAL(UpdateInstallFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlot(bool,QStringList,QString,QString)));
            connect(updateMutual->interface,SIGNAL(UpdateDownloadFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlotindownload(bool,QStringList,QString,QString)));
        }
        lastRefreshTime->setText(tr("The system is updating..."));
        qDebug()<<"dependencyupdateAll";
        updateMutual->DistUpgradeAll(true);

}

void TabWid::updateAllApp(bool status)
{
    allProgressBar->setState(NormalProgress);
        if(!status){
            checkUpdateBtn->setText(tr("Calculating"));
            connect(updateMutual->interface,SIGNAL(UpdateDependResloveStatus(bool,bool,QStringList,QStringList,QStringList,QString,QString)),this,SLOT(DependResloveResult(bool,bool,QStringList,QStringList,QStringList,QString,QString)));

        }else{
            foreach (AppUpdateWid *wid, widgetList) {
                wid->updateAPPBtn->hide();
                wid->hide();
            }
            connect(updateMutual->interface,SIGNAL(UpdateDloadAndInstStaChanged(QStringList,int,QString,QString)),this,SLOT(getAllProgress(QStringList,int,QString,QString)));
            connect(updateMutual->interface,SIGNAL(UpdateDownloadInfo(QStringList,int,int,uint,uint,int)),this,SLOT(showDownloadInfo(QStringList,int,int,uint,uint,int)));
            connect(updateMutual->interface,SIGNAL(UpdateInstallFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlot(bool,QStringList,QString,QString)));
            connect(updateMutual->interface,SIGNAL(UpdateDownloadFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlotindownload(bool,QStringList,QString,QString)));
        }
        lastRefreshTime->setText(tr("The system is updating..."));
        qDebug()<<"updateAllApp";
        updateMutual->DistUpgradeAll(status);

}

void TabWid::disupdateallaccept()
{
    qDebug()<<"disupdateallaccept";
    foreach (AppUpdateWid *wid, widgetList) {
        wid->hide();
    }
    connect(updateMutual->interface,SIGNAL(UpdateDloadAndInstStaChanged(QStringList,int,QString,QString)),this,SLOT(getAllProgress(QStringList,int,QString,QString)));
    connect(updateMutual->interface,SIGNAL(UpdateDownloadInfo(QStringList,int,int,uint,uint,int)),this,SLOT(showDownloadInfo(QStringList,int,int,uint,uint,int)));
    connect(updateMutual->interface,SIGNAL(UpdateInstallFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlot(bool,QStringList,QString,QString)));
    connect(updateMutual->interface,SIGNAL(UpdateDownloadFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlotindownload(bool,QStringList,QString,QString)));
    updateMutual->DistUpgradeSystem(true);
//    distUpgradeAllApp(true);
}

bool TabWid::distUpgradeAllApp(bool status)
{
    if(!status){
        connect(updateMutual->interface,SIGNAL(DistupgradeDependResloveStatus(bool,bool,QStringList,QStringList,QStringList,QString,QString)),this,SLOT(DistupgradeDependResloveResult(bool,bool,QStringList,QStringList,QStringList,QString,QString)));
    }else{

        foreach (AppUpdateWid *wid, widgetList) {
            wid->hide();
        }
        connect(updateMutual->interface,SIGNAL(UpdateDloadAndInstStaChanged(QStringList,int,QString,QString)),this,SLOT(getAllProgress(QStringList,int,QString,QString)));
        connect(updateMutual->interface,SIGNAL(UpdateDownloadInfo(QStringList,int,int,uint,uint,int)),this,SLOT(showDownloadInfo(QStringList,int,int,uint,uint,int)));
        connect(updateMutual->interface,SIGNAL(UpdateInstallFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlot(bool,QStringList,QString,QString)));
        connect(updateMutual->interface,SIGNAL(UpdateDownloadFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlotindownload(bool,QStringList,QString,QString)));
    }
    updateMutual->DistUpgradeSystem(status);
}

void TabWid::autoupgradestatusshow()
{
    qDebug()<<"接收到备份完成信号";
    filewatcher = new QFileSystemWatcher();
    filewatcher->addPath("/var/run/unattended-upgrades.progress");
    connect(filewatcher,SIGNAL(fileChanged(QString)),this,SLOT(progresschanged()));
    QStringList path = filewatcher->files();
    qDebug()<<path;
//    return;
}

void TabWid::progresschanged()
{
    qDebug()<<"进度变化";
    QFile progressfile("/var/run/unattended-upgrades.progress");
    QFile lockfile("/tmp/auto-upgrade/ukui-control-center.lock");
    QFile pidfile("/var/run/unattended-upgrades.pid");
    progressfile.open(QIODevice::ReadOnly);
    QString prgs= progressfile.readAll();
//    versionInformationLab->setText("系统自动更新功能正在安装新文件："+prgs);
    versionInformationLab->setText(tr("Auto-Update progress is installing new file：")+prgs+"%");
    checkUpdateBtn->hide();
    filewatcher->addPath("/var/run/unattended-upgrades.progress");//加此句的意义：电脑的修改文件的方式不一样，有可能会删除该文件导致只能够监测一次文件的变化
    QDBusConnection::systemBus().connect(QString(),QString("/"),"com.kylin.install.notification","InstallFinish",this,SLOT(autoinstallfinish()));
    return;


    /*不能通过判断文件是否还存在判断安装结束，这只能作为一种异常处理，可以让沈亚峰引入信号*/
//    if(!pidfile.exists())//文件pid不存在就是程序
//    {
//        versionInformationLab->setText("系统更新完成！");
//        qDebug()<<"断开开关链接";
//        disconnect(filewatcher,SIGNAL(fileChanged(QString)),this,SLOT(progresschanged()));
//        return;
//    }
//    return;
}

void TabWid::autoinstallfinish()
{
//    versionInformationLab->setText("系统自动更新完成！");
    versionInformationLab->setText(tr("Auto-Update progress finished!"));
    disconnect(filewatcher,SIGNAL(fileChanged(QString)),this,SLOT(progresschanged()));
    return;
}

void TabWid::backupfailure()
{
//    versionInformationLab->setText("自动更新安装时备份失败！");
    versionInformationLab->setText(tr("Auto-Update progress fail in backup!"));
    return;
}

QString TabWid::getLanguageEnv()
{
    QStringList environment = QProcess::systemEnvironment();
    QString language_default="zh_CN.UTF-8";
    QString str="";
    foreach(str,environment)
    {
        if (str.startsWith("LANG="))
        {
            return str.mid(5);
        }
    }
    return language_default;
}

void TabWid::fixbrokencancel()
{
//    versionInformationLab->setText("自动更新安装时备份失败！");
    qDebug()<<"recieve the signal";
    versionInformationLab->setText(tr("Failed in updating because of broken environment."));
    checkUpdateBtn->setText(tr("Check Update"));
    checkUpdateBtn->stop();
    checkUpdateBtn->setEnabled(true);
    checkUpdateBtn->show();
//    disconnect(fixbrokeninstall,&fixbrokeninstalldialog::fixbrokenpkgkeepbtnclickedsignal,this,&TabWid::fixbrokencancel);
//    disconnect(fixbrokeninstall,&fixbrokeninstalldialog::fixbrokenpkgremovebtnclickedsignal,this,&TabWid::fixbrokenremove);
    return;
}

void TabWid::fixbrokenremove()
{
    qDebug()<<"fixup the environment";
    updateMutual->interface->asyncCall("FixBrokenDepends");
    versionInformationLab->setText(tr("It's fixing up the environment..."));
//    disconnect(fixbrokeninstall,&fixbrokeninstalldialog::fixbrokenpkgkeepbtnclickedsignal,this,&TabWid::fixbrokencancel);
//    disconnect(fixbrokeninstall,&fixbrokeninstalldialog::fixbrokenpkgremovebtnclickedsignal,this,&TabWid::fixbrokenremove);
    return;
}

void TabWid::showdetaillist(QStringList pkgname,QStringList description,QStringList deletereason,int mode)
{
    qDebug()<<"get the siganl";
    qDebug()<<"the install mode is "<<mode;
//    m_instance = fixupdetaillist::GetInstance(this);
    m_instance = updatedeleteprompt::GetInstance(this);
    int pkgnum = m_instance->updatedeletepkglist(pkgname,description,deletereason);
    m_instance->pkgname=pkgname;
    m_instance->description=description;
    m_instance->deletereason=deletereason;
    m_instance->updatemode=mode;

    /*疑问：该处的connect应该在此处，还是在弹出备份并更新那里呢？*/
    connect(m_instance,&updatedeleteprompt::updatepromptsolvecancelsignal,this,&TabWid::closedpendencyfixdialog);
    connect(m_instance,&updatedeleteprompt::updatedeletepromptacceptsignal,this,&TabWid::updatedeletepromptaccept);
    connect(m_instance,&updatedeleteprompt::disupdatedependsolveacceptsignal,this,&TabWid::disupdateallaccept);
    m_instance->exec();
}

QString TabWid::getversion()
{
    QDBusInterface upgrade( "com.kylin.systemupgrade",
                            "/com/kylin/systemupgrade",
                            "com.kylin.systemupgrade.interface",
                            QDBusConnection::systemBus() );
    QDBusMessage upgradever = upgrade.call("GetSystemUpdateVersion");
    QString upgradeversion1 = upgradever.arguments().value(0).toString().replace("\"","");
    QString upgradeversion2 = upgradever.arguments().value(1).toString().replace("\"","");
    if(!upgradeversion1.isNull())
        qInfo()<<"upgradeversion1 is "<<upgradeversion1;
    if(!upgradeversion2.isNull())
        qInfo()<<"upgradeversion2 is "<<upgradeversion2;
    QString upgradeversion = QString("openKylin %1 (%2)").arg(upgradeversion1).arg(upgradeversion2);
    if(upgradeversion1.isEmpty()||upgradeversion1.isNull())
    {
        upgradeversion = QString("openKylin 2.0");       
    }
    else if(upgradeversion2.isEmpty()||upgradeversion2.isNull())
    {
        upgradeversion = QString("openKylin %1").arg(upgradeversion1);
    }
    return upgradeversion;
}

void TabWid::closedpendencyfixdialog()
{
    qDebug()<<"get the closedpendencyfixdialog signal";
//    isAllUpgrade=false;
    checkUpdateBtn->setEnabled(true);
    checkUpdateBtn->stop();
    //        checkUpdateBtn->setText(tr("检查更新"));
    checkUpdateBtn->setText(tr("UpdateAll"));
    checkUpdateBtn->adjustSize();
//    versionInformationLab->setText(tr("update has been canceled!"));
    checkUpdateBtn->show();

    foreach (AppUpdateWid *wid, widgetList) {
//        if(wid->updateAPPBtn->text() == tr("Update"))
            wid->updateAPPBtn->setEnabled(true);
    }

//    disconnect(updateMutual->interface,SIGNAL(UpdateDependResloveStatus(bool,bool,QStringList,QStringList,QStringList,QString,QString)),this,SLOT(DependResloveResult(bool,bool,QStringList,QStringList,QStringList,QString,QString)));
//    disconnect(updateMutual->interface,SIGNAL(UpdateDloadAndInstStaChanged(QStringList,int,QString,QString)),this,SLOT(getAllProgress(QStringList,int,QString,QString)));
//    disconnect(updateMutual->interface,SIGNAL(UpdateDownloadInfo(QStringList,int,int,uint,uint,int)),this,SLOT(showDownloadInfo(QStringList,int,int,uint,uint,int)));
//    disconnect(updateMutual->interface,SIGNAL(UpdateInstallFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlotindownload(bool,QStringList,QString,QString)));
//    disconnect(updateMutual->interface,SIGNAL(UpdateDownloadFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlot(bool,QStringList,QString,QString)));
    disconnect(dependfixdialog,&dependencyfixdialog::updatedependsolvecancelsignal,this,&TabWid::updatecancel);
//    disconnect(dependfixdialog,&dependencyfixdialog::updatealldependsolveacceptsignal,this,&TabWid::updateAllApp);
    disconnect(dependfixdialog,&dependencyfixdialog::disupdatedependsolveacceptsignal,this,&TabWid::disupdateallaccept);
//    connect(dependfixdialog,&dependencyfixdialog::updatedependshowdetailssignal,this,&TabWid::showdetaillist);
    disconnect(dependfixdialog,SIGNAL(updatedependshowdetailssignal(QStringList,QStringList,QStringList,int)),this,SLOT(showdetaillist(QStringList,QStringList,QStringList,int)));


    disconnect(dependfixdialog,&dependencyfixdialog::closedpendencyfixdialog,this,&TabWid::closedpendencyfixdialog);

    /*该三个为列表弹窗的信号链接*/
//    disconnect(m_instance,&updatedeleteprompt::updatepromptsolvecancelsignal,this,&TabWid::closedpendencyfixdialog);
//    disconnect(m_instance,&updatedeleteprompt::updatedeletepromptacceptsignal,this,&TabWid::updatedeletepromptaccept);
//    disconnect(m_instance,&updatedeleteprompt::disupdatedependsolveacceptsignal,this,&TabWid::disupdateallaccept);
}

//void TabWid::updateAllbyprompt()
//{
//    allProgressBar->setState(NormalProgress);
//        if(!status){
//            checkUpdateBtn->setText(tr("Calculating"));
//            connect(updateMutual->interface,SIGNAL(UpdateDependResloveStatus(bool,bool,QStringList,QStringList,QStringList,QString,QString)),this,SLOT(DependResloveResult(bool,bool,QStringList,QStringList,QStringList,QString,QString)));

//        }else{
//            foreach (AppUpdateWid *wid, widgetList) {
//                wid->updateAPPBtn->hide();
//                wid->hide();
//            }
//            connect(updateMutual->interface,SIGNAL(UpdateDloadAndInstStaChanged(QStringList,int,QString,QString)),this,SLOT(getAllProgress(QStringList,int,QString,QString)));
//            connect(updateMutual->interface,SIGNAL(UpdateDownloadInfo(QStringList,int,int,uint,uint,int)),this,SLOT(showDownloadInfo(QStringList,int,int,uint,uint,int)));
//            connect(updateMutual->interface,SIGNAL(UpdateInstallFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlot(bool,QStringList,QString,QString)));
//            connect(updateMutual->interface,SIGNAL(UpdateDownloadFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlotindownload(bool,QStringList,QString,QString)));
//        }
//        qDebug()<<"updateAllApp";
//        updateMutual->DistUpgradeAll(status);
//}

void TabWid::updatedeletepromptaccept()
{
    allProgressBar->setState(NormalProgress);
        {
            foreach (AppUpdateWid *wid, widgetList) {
                wid->updateAPPBtn->hide();
                wid->hide();
            }
            connect(updateMutual->interface,SIGNAL(UpdateDloadAndInstStaChanged(QStringList,int,QString,QString)),this,SLOT(getAllProgress(QStringList,int,QString,QString)));
            connect(updateMutual->interface,SIGNAL(UpdateDownloadInfo(QStringList,int,int,uint,uint,int)),this,SLOT(showDownloadInfo(QStringList,int,int,uint,uint,int)));
            connect(updateMutual->interface,SIGNAL(UpdateInstallFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlot(bool,QStringList,QString,QString)));
            connect(updateMutual->interface,SIGNAL(UpdateDownloadFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlotindownload(bool,QStringList,QString,QString)));
        }
        qDebug()<<"updateAllApp";
        updateMutual->DistUpgradeAll(true);

}
