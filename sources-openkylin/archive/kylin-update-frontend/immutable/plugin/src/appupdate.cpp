#include "appupdate.h"
#include <QPixmap>
#include <stdio.h>
#include <QLocale>
#include <QScrollBar>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
using namespace  std;

#define JSON_FILE_PATH "/var/cache/kylin-system-updater/json/"

AppUpdateWid::AppUpdateWid(AppAllMsg msg,QWidget *parent):QWidget(parent)
{
    m_updateMutual = UpdateDbus::getInstance(this);
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
    appname.clear();
    appname = name;
    frameLayout = new QHBoxLayout();
    frameLayout->setContentsMargins(0,0,0,0);   //更新信息之间的间距
    frameLayout->setSpacing(0);
    this->setLayout(frameLayout);

    AppFrame = new QFrame(this);  //最外层窗口适配主题样式
    AppFrame->setFrameShape(QFrame::Box);
    AppFrame->setMaximumSize(QSize(16777215, 16777215));
    frameLayout->addWidget(AppFrame);
    mainHLayout = new QHBoxLayout();
    appTitleWid = new QWidget();
    iconNameLayout = new QHBoxLayout();
    appIconName = new QLabel(this);
    appIconName->setFixedWidth(200);
    appIcon = new QLabel(appIconName);
    appNameLab = new MyLabel(appIconName);
    appIconName->setLayout(iconNameLayout);
    appIcon->setFixedSize(32,32);
    iconNameLayout->addSpacing(6);
    iconNameLayout->addWidget(appIcon,1);
    iconNameLayout->addSpacing(8);
    iconNameLayout->addWidget(appNameLab,10);
    appVersion = new FixLabel(this);
    appVersion ->setFixedWidth(250);
    detaileInfo = new KBorderlessButton(this);
    //    detaileInfo->setText(tr("详情"));
    detaileInfo->setText(tr("details"));
    QPalette palette_datailInfo = detaileInfo->palette();
    palette_datailInfo.setColor(QPalette::ButtonText,QColor(55,144,250));
    detaileInfo->setPalette(palette_datailInfo);
    updateAPPBtn = new QPushButton(this);
    //    updateAPPBtn->setText(tr("更新"));
    updateAPPBtn->setText(tr("Update"));
    updateAPPBtn->setFixedWidth(95);
    QSizePolicy appupdatebutton_policy = updateAPPBtn->sizePolicy();
    appupdatebutton_policy.setRetainSizeWhenHidden(true);//保持按钮被隐藏时的布局稳定

    mainHLayout->addWidget(appIconName,1,Qt::AlignLeft);
    mainHLayout->addWidget(appVersion,1,Qt::AlignLeft);
    mainHLayout->addWidget(detaileInfo,1,Qt::AlignHCenter);
    mainHLayout->addSpacing(60);
    mainHLayout->addWidget(updateAPPBtn,0,Qt::AlignRight);
    mainHLayout->setContentsMargins(0,0,16,0);
    mainHLayout->setSpacing(0);

    AppFrame->setLayout(mainHLayout);
    QMap<QString, QString> map = getNameAndIconFromJson(name);
    if (!map.value("update-name").isNull())
    {
        dispalyName = map.value("update-name");
        qInfo()<<"map.value.isNull()"<<dispalyName;
    }
    appNameLab->setText(dispalyName);
    /*判断图标，优先级: JSON文件指定 > qrc资源文件中 > 主题 > 默认*/
    if (!map.value("icon").isNull()) {
        haveThemeIcon = true;
        appIcon->setPixmap(QPixmap(map.value("icon")));
    } else if (name.contains("kylin-update-desktop-")||name == "linux-generic") {
        qInfo()<<"app exsist icon";
        haveThemeIcon = true;
        pkgIconPath = QString(":/img/upgrade/%1.png").arg(name);
        appIcon->setPixmap(QPixmap(pkgIconPath));
    } else if (QIcon::fromTheme(name).hasThemeIcon(name)) {    //判断是否有主题图标并输出
        haveThemeIcon = true;
        qInfo()<<"theme exsist icon";
        QIcon icon = QIcon::fromTheme(name);
        QPixmap pixmap = icon.pixmap(icon.actualSize(QSize(32, 32)));
        appIcon->setPixmap(pixmap);
    } else {
        qInfo()<<"else exsist icon";
        QIcon icon = QIcon::fromTheme("application-x-desktop");
        QPixmap pixmap = icon.pixmap(icon.actualSize(QSize(32, 32)));
        appIcon->setPixmap(pixmap);
    }
    QString newStrMsg = appAllMsg.availableVersion;
    if(newStrMsg.size()>16){
        appVersion->setText(tr("Version:")+newStrMsg);
        appVersion->setToolTip(tr("Version:")+newStrMsg);
    }
    else{
        appVersion->setText(tr("Version:")+newStrMsg);
        appVersion->setToolTip("");
    }
}


AppUpdateWid::~AppUpdateWid()
{
}

void AppUpdateWid::initConnect()
{
    connect(detaileInfo,&QPushButton::clicked,this,&AppUpdateWid::showDetails);
    connect(updateAPPBtn,&QPushButton::clicked,this,&AppUpdateWid::cancelOrUpdate);
}

//void AppUpdateWid::startPreUpdate()
//{
//    qInfo()<<"get in start preupdate";
//    connect(m_updateMutual->interface,SIGNAL(UpdateDependResloveStatus(bool,bool,QStringList,QStringList,QStringList,QString,QString)),
//            this,SLOT(oneAppDependResloveResult(bool,bool,QStringList,QStringList,QStringList,QString,QString)));
//    m_updateMutual->interface->asyncCall("DistUpgradePartial",false,appAllMsg.name);//计算依赖
//}

//void AppUpdateWid::oneAppDependResloveResult(bool ResloveStatus,bool ResloveResult,QStringList DeletePkgList, QStringList DeletePkgDeslist,QStringList DeletePkgReasonlist,QString Error,QString Reason)
//{
//    qInfo()<<"oneAppDependResloveResult";
//    disconnect(m_updateMutual->interface,SIGNAL(UpdateDependResloveStatus(bool,bool,QStringList,QStringList,QStringList,QString,QString)),
//               this,SLOT(oneAppDependResloveResult(bool,bool,QStringList,QStringList,QStringList,QString,QString)));
//    if(!ResloveStatus){
//        qInfo()<<"处理依赖有问题："<<Error<<Reason;
//        QMessageBox msgBox(qApp->activeModalWidget());
//        msgBox.setText(tr("There are unresolved dependency conflicts in this update，Please select update all"));
//        msgBox.setWindowTitle(tr("Prompt information"));
//        msgBox.setIcon(QMessageBox::Icon::Information);
//        msgBox.addButton(tr("Update All"), QMessageBox::YesRole);
//        msgBox.addButton(tr("Cancel"), QMessageBox::NoRole);
//        int ret = msgBox.exec();
//        switch (ret) {
//        case 0:
//            qInfo() << "全部更新，该处逻辑暂未处理（涉及到全部更新）";
//            break;
//        case 1:
//            qInfo() << "取消更新";
//            break;
//        default:
//            qInfo() << "取消更新";
//            break;
//        }
//    }
//    else{
//        if (!ResloveResult){
//            qInfo()<<"无需卸载包，可以直接更新，常规场景";
//            startBackup();
//        }
//        else{
//            qInfo()<<"需卸载包后，再更新";
////            oneappshowDependSlovePtompt(DeletePkgList,DeletePkgDeslist,DeletePkgReasonlist);（做界面展示）
//        }
//    }
//}

void AppUpdateWid::startBackup()/*暂时无进度，后续要加进度，把结果信号嵌套进去*/
{
    qInfo()<<"start backup";
    backup_interface = new QDBusInterface( "com.kylin.systembackup",
                               "/update",
                               "com.kylin.systembackup.manager",
                               QDBusConnection::systemBus() );
    QString userName = qgetenv("USER");
    int uid = getuid();
    connect(backup_interface,SIGNAL(sendAutoBackupResult(bool)),this,SLOT(backupResult(bool)));
    QDBusMessage backupResult = backup_interface->call("autoBackUpForSystemUpdate","backup for v11 upgrade",userName,uid);
    if (backupResult.type() == QDBusMessage::ErrorMessage){
        qInfo() << "Call dbus method error:" << backupResult.errorMessage();
        disconnect(backup_interface,SIGNAL(sendAutoBackupResult(bool)),this,SLOT(backupResult(bool)));
    }
}

void AppUpdateWid::backupResult(bool result)
{
    qInfo()<<"get in backupResult,disconnect sendAutoBackupResult";
    disconnect(backup_interface,SIGNAL(sendAutoBackupResult(bool)),this,SLOT(backupResult(bool)));
    if(result){
        qInfo()<<"备份成功";
        startUpdate();
    }
    else{
        qInfo()<<"备份失败";
        QMessageBox msgBox(qApp->activeModalWidget());
        msgBox.setText(tr("The backup is abnormal. Do you want to continue updating?"));
        msgBox.setWindowTitle(tr("Prompt information"));
        msgBox.setIcon(QMessageBox::Icon::Information);
        msgBox.addButton(tr("Continue"), QMessageBox::YesRole);
        msgBox.addButton(tr("Cancel"), QMessageBox::NoRole);
        int ret = msgBox.exec();
        switch (ret) {
        case 0:
            qInfo() << "继续更新";
            startUpdate();
            break;
        case 1:
            qInfo() << "取消更新";
//            appVersion->setText(tr("Update has been canceled."));
//            updateAPPBtn->setText(tr("Update"));
//            updateAPPBtn->setEnabled(true);
//            updateAPPBtn->show();
            break;
        default:
            qInfo() << "取消更新";
//            appVersion->setText(tr("Update has been canceled."));
//            updateAPPBtn->setText(tr("Update"));
//            updateAPPBtn->setEnabled(true);
//            updateAPPBtn->show();
            break;
        }
    }
}

void AppUpdateWid::startUpdate()
{

    connect(m_updateMutual->interface,SIGNAL(UpdateDownloadInfo(int,int,quint64,quint64,int)),this,SLOT(showDownloadStatues(int,int,quint64,quint64,int)));
    connect(m_updateMutual->interface,SIGNAL(UpdateDownloadFinished(bool,QStringList,QString,QString)),this,SLOT(showDownloadFinsih(bool,QStringList,QString,QString)));
    /*旧版本逻辑
    m_updateMutual->interface->asyncCall("DistUpgradePartial",true,appAllMsg.name);//计算依赖
    updateAPPBtn->setText(tr("Cancel"));
    updateAPPBtn->setEnabled(false);
    appVersion->setText(tr("Ready to update"));
    */
    m_updateMutual->interface->call("UpdateDownloadAll");
    updateAPPBtn->setText(tr("Cancel"));
    updateAPPBtn->setEnabled(true);
    appVersion->setText(tr("Ready to update"));
}

void AppUpdateWid::showDownloadStatues(int currentserial,int total,quint64 downloadedsize,quint64 totalsize,int downloadSpeed)
{
    qInfo()<<"get in showDownloadStatues:"<<currentserial<<total<<downloadedsize<<totalsize<<downloadSpeed;
    QString downloadspeed = modifySpeedUnit(downloadSpeed, 1);
    appVersion->setText(tr("downloading")+":"+"("+downloadspeed+")"+modifySizeUnit(downloadedsize)+"/"+modifySizeUnit(totalsize));
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
            return QString("%1%2").arg((int)(size/1024)).arg("kB/S");
    }
    else
        return QString("%1%2").arg(size).arg("B/S");
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
            return QString("%1%2").arg((int)(tmpSize*100 + 0.5)/100.0).arg("kB");//保留小数点后两位，并四舍五入
    }
    else
        return QString("%1%2").arg(size).arg("B");
}

void AppUpdateWid::showDownloadFinsih(bool state,QStringList pkgname,QString error,QString reason)
{
    qInfo()<<"get in showDownloadFinsih:"<<state<<pkgname<<error<<reason;
    if(state){
        qInfo()<<"完成下载";
        updateAPPBtn->show();
        updateAPPBtn->setText(tr("Reboot"));
        updateAPPBtn->setEnabled(true);
        appVersion->setText(tr("Please reboot to install the update."));
    }
    else if(!state && !CancelBtnClicked){
        qInfo()<<"下载失败";
        updateAPPBtn->show();
        updateAPPBtn->setText(tr("Update"));
        updateAPPBtn->setEnabled(true);
        appVersion->setText(tr("Update failed!"));
    }
    else if(!state && CancelBtnClicked){
        qInfo()<<"取消下载";
        CancelBtnClicked = false;
        updateAPPBtn->show();
        updateAPPBtn->setText(tr("Update"));
        updateAPPBtn->setEnabled(true);
        appVersion->setText(tr("Update Canceled!"));
    }
}

void AppUpdateWid::cancelOrUpdate()
{
    qInfo()<<"cancel or update";
    if(updateAPPBtn->text() == tr("Reboot"))
    {
        qInfo()<<"appupdate reboot";
        /*
        QDBusInterface upgrade_interface( "com.kylin.systemupgrade",
                                   "/com/kylin/systemupgrade",
                                   "com.kylin.systemupgrade.interface",
                                   QDBusConnection::systemBus() );
        QDBusMessage rebootresult = upgrade_interface.call("TriggerInstallOnShutdown","reboot");
        if(!rebootresult.errorMessage().isEmpty())
        {
            qInfo()<<"dbus call failed,"<<rebootresult.errorMessage();
        }
        int  reboot =rebootresult.arguments().value(0).toInt();
        QString reason = rebootresult.arguments().value(1).toString();
        if(!reboot)
        {
            qInfo()<<"reboot success!";
        }
        */
        qInfo()<<"reboot rightnow";
        QDBusMessage rebootresult = m_updateMutual->interface->call("DeployLatestUpdate","reboot");
        int  reboot =rebootresult.arguments().value(0).toInt();
        QString reason = rebootresult.arguments().value(1).toString();
        qInfo()<<"reboot result and reason"<<reboot<<reason;
        if(reboot)
        {
            QMessageBox msgBox(qApp->activeModalWidget());
            msgBox.setText(reason);
            msgBox.setWindowTitle(tr("Reboot failed!"));
            msgBox.setIcon(QMessageBox::Icon::Information);
            msgBox.setStandardButtons(QMessageBox::Ok);
            msgBox.setButtonText(QMessageBox::Ok,tr("OK"));
            msgBox.exec();
        }
    }
    else if(updateAPPBtn->text() == tr("Update"))
    {
        emit changeTabWidText();
//        startPreUpdate();
        startBackup();
//        startUpdate();
//        startBackup();
    }
    if(updateAPPBtn->text() == tr("Cancel")){
        qInfo()<<"the text of btn is"<<updateAPPBtn->text();
        qInfo()<<"cancel download";
        CancelBtnClicked = true;
        updateAPPBtn->setText(tr("Update"));
        detaileInfo->show();
        QDBusReply<int> reply = m_updateMutual->interface->call("CancelDownload");
        if(!reply.isValid())
        {
            qInfo()<<"取消下载接口异常";
            return ;
        }
        int cancelreply = reply.value();
        qInfo()<<"cancelDownload value:"<<reply.value();
        if(cancelreply == 0)
        {
            qInfo()<<"取消成功，进入Tabwid";
            isCancel = true;
            emit appupdateiscancel();
        }
        else
        {
            qInfo()<<"取消失败!!!!!";
        }
    }
}

QMap<QString, QString> AppUpdateWid::getNameAndIconFromJson(QString pkgname)
{
    QMap <QString, QString> nameIconList;
    /*判断json文件是否存在*/
    QString filename = QString(JSON_FILE_PATH) +pkgname +".json";
    qInfo()<<"getNameAndIconFromJson's filename is "<<filename;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)){
        qInfo() << "JSON file open failed! ";
        file.close();
        return nameIconList;
    }
    QByteArray jsonData = file.readAll();

    QJsonParseError err_rpt;
    QJsonDocument  root_Doc = QJsonDocument::fromJson(jsonData, &err_rpt); // 字符串格式化为JSON

    if (!root_Doc.isNull() && (err_rpt.error == QJsonParseError::NoError)) {  // 解析未发生错误
        if (root_Doc.isObject()) { // JSON 文档为对象
            QJsonObject object = root_Doc.object();  // 转化为对象
            if (QLocale::system().name() == "zh_CN"){
                QString name  = object.value("update-name").toObject().value("zh_CN").toString();
                if (!name.isNull()) {
                    nameIconList.insert("update-name", name);
                }
            }else {
                QString name  = object.value("update-name").toObject().value("en_US").toString();
                if (!name.isNull()) {
                    nameIconList.insert("update-name", name);
                }
            }
            QString iconPath = object.value("icon").toString();
            if (!iconPath.isNull())
                nameIconList.insert("icon", iconPath);
        }
    }else{
        qInfo() << "JSON文件格式错误！";
        file.close();
        return nameIconList;
    }
    file.close();
    return nameIconList;
}

void AppUpdateWid::showDetails()
{
    qInfo()<<"show details in app";
    Detaildialog *detaillog=new Detaildialog();
    detaillog->setFixedSize(420,314);
    detaillog->logContent->clear();
    detaillog->setContentsMargins(24,24,24,24);
    QPoint detail_pos = detaileInfo->mapToGlobal(detaileInfo->pos());
    detaillog->move(QCursor().pos().x()-detaillog->width(),detail_pos.y()+detaileInfo->height());
    QString appname_lab = appNameLab->text();
    qInfo()<<"the appname_lab is"<< appname_lab;
    QString filename = "/var/cache/kylin-system-updater/json/"+appname+".json";
    qInfo()<<"file name is: "<<filename;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)){
        qInfo() << "JSON file open failed! "<<file.errorString();
        return ;
    }
    QByteArray jsonData = file.readAll();
    QJsonParseError err_rpt;
    QJsonDocument  root_Doc = QJsonDocument::fromJson(jsonData, &err_rpt); // 字符串格式化为JSON
    if (!root_Doc.isNull() && (err_rpt.error == QJsonParseError::NoError))
    {  // 解析未发生错误
        if (root_Doc.isObject())
        { // JSON 文档为对象
            QJsonObject object = root_Doc.object();  // 转化为对象
            if (QLocale::system().name() == "zh_CN")
            {
                QString name  = object.value("update-name").toObject().value("zh_CN").toString();
                QString description  = object.value("description").toObject().value("zh_CN").toString();
                if (!description.isNull()) {
                    detaillog->logContent->append(description);
//                    msg->shortDescription=description;
                }
                else
                {
                    detaillog->logContent->append(tr("Cumulative updates"));
                }
            }
            else
            {
                QString name  = object.value("update-name").toObject().value("en_US").toString();
                QString description  = object.value("description").toObject().value("en_US").toString();
                if (!description.isNull()) {
//                    msg->shortDescription=description;
                    detaillog->logContent->append(description);
                }
                else
                {
                    detaillog->logContent->append(tr("Cumulative updates"));
                }
            }
        }
    }
    detaillog->installContent->setText(tr("version:")+appAllMsg.availableVersion+"  "+QString(modifySizeUnit(appAllMsg.msg.allSize)));
    detaillog->installContent->setStyleSheet("QLabel{color: rgb(129, 129, 129);}");
    file.close();
    detaillog->show();
}
