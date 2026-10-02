#include "tabwidget.h"
#ifdef signals
#undef signals
#endif
extern "C" {
#include <gio/gdesktopappinfo.h>
#include <glib.h>
}


/*全部更新中的更新流程也得保留，避免续接出现问题*/
#define JSON_FILE_PATH "/var/cache/kylin-system-updater/json/"

TabWid::TabWid(QWidget *parent):QWidget(parent)
{
    qInfo()<<"!!!!!!!!! get in v11 ostree";
    allComponents();
}

void TabWid::initDbus()
{
    /* 更新管理器dbus接口 */
    updateMutual = UpdateDbus::getInstance();
    datetimeutils = DateTimeUtils::get_instance();
    /*sdk 时间格式接口*/
    m_dateSessionDbus = new QDBusInterface("com.kylin.kysdk.DateServer",
                                             "/com/kylin/kysdk/Date",
                                             "com.kylin.kysdk.DateInterface",
                                             QDBusConnection::sessionBus(),
                                             this);
    if (m_dateSessionDbus->isValid()) {
        connect(m_dateSessionDbus,SIGNAL(ShortDateSignal(QString)),this,SLOT(UpdateSdkTime(QString)));
        connect(m_dateSessionDbus,SIGNAL(TimeSignal(QString)), this, SLOT(UpdateSdkTime(QString)));
    }
    /*sdk声效*/
    soundplayeriface = new QDBusInterface("org.ukui.sound.theme.player",
                                          "/org/ukui/sound/theme/player",
                                          "org.ukui.sound.theme.player",
                                          QDBusConnection::sessionBus(),
                                          this);
    connect(checkUpdateBtn,&QPushButton::clicked,this,&TabWid::checkUpdateBtnClicked);
    connect(checkUpdateBtn,&QPushButton::clicked,this,[=] (bool status){
        ukcc::UkccCommon::buriedSettings("Upgrade",checkUpdateBtn->text(),QString("clicked"));
    });
    connect(historyUpdateLog,&QPushButton::clicked,this,&TabWid::showHistoryWidget);
    connect(isAutoCheckSBtn,&KSwitchButton::stateChanged,this,&TabWid::isAutoCheckedChanged);
    connect(isAutoCheckSBtn,&KSwitchButton::stateChanged,this,[=] (bool status){
        ukcc::UkccCommon::buriedSettings(QString("Upgrade"), QString("isAutoCheckSBtn"), QString("settings"),  status ? "true":"false");
    });
    connect(isAutoUpgradeSBtn,&KSwitchButton::stateChanged,this, &TabWid::isAutoUpgradeChanged);
    connect(isAutoUpgradeSBtn,&KSwitchButton::stateChanged,this,[=] (bool status){
        ukcc::UkccCommon::buriedSettings(QString("Upgrade"), QString("isAutoUpgradeSBtn"), QString("settings"),  status ? "true":"false");
    });
    connect(DownloadHBtn,&KSwitchButton::stateChanged,this,&TabWid::DownloadLimitChanged);
    connect(DownloadHBtn,&KSwitchButton::stateChanged,this,[=] (bool status){
        ukcc::UkccCommon::buriedSettings(QString("Upgrade"), QString("DownloadHBtn"), QString("settings"),  status ? "true":"false");
    });
    connect(DownloadHValue,&QComboBox::currentTextChanged,this,&TabWid::DownloadLimitValueChanged);
    /*Beta版本反馈机制*/
    connect(TrailBorderlessButton,&KBorderlessButton::clicked,this,&TabWid::ShowFeedback);
    connect(TrailCheckBtn,&KSwitchButton::stateChanged,this,&TabWid::FeedbackStatusChanged);
    connect(RestoreButton,&QPushButton::clicked,this,&TabWid::OpenBackupTool);
    /*策略下发功能*/
    connect(updateMutual->interface,SIGNAL(ButtonStatusChange(QString,QString)),this,SLOT(SecurityDownloadChange(QString,QString)));
    /*分步更新安装过程提示*/
    connect(AdvancedOption,&QPushButton::clicked,this,&TabWid::ShowAdvancedOption);
    checkUpdateBtn->stop();
    checkUpdateBtn->setText(tr("Check Update"));
    checkUpdateBtnClicked();
}

void TabWid::allComponents()
{
    AppMessage = new QVBoxLayout();
    AppMessage->setAlignment(Qt::AlignTop);
    this->setLayout(AppMessage);
    AppMessage->setContentsMargins(0, 0, 0, 0);
    this->setMinimumSize(QSize(550,0));
    this->setMaximumSize(QSize(16777215, 16777215));

    /*系统更新第一部分*/
    /*update一级标题*/
    labUpdate = new TitleLabel(this);
    //    labUpdate->setText(tr("更新"));
    labUpdate->setText(tr("Update"));
    /*检查更细框内布局设置*/
    systemWidget = new QFrame(this);
    systemWidget->setFrameShape(QFrame::Box);
    /*appwidget初始布局*/
    allUpdateWid = new QWidget(this);
    allUpdateLayout = new QVBoxLayout();
    allUpdateLayout->setAlignment(Qt::AlignTop);
    allUpdateWid->setLayout(allUpdateLayout);
    allUpdateLayout->setSpacing(0);
    allUpdateLayout->setMargin(0);

    tab1HLayout = new QHBoxLayout();
    systemWidget->setLayout(tab1HLayout);
    tab1HLayout->setContentsMargins(16,0,16,0);
    systemWidget->setFixedHeight(132);

    /*左侧图片*/
    systemPortraitLab = new QLabel(systemWidget);
    systemPortraitLab->setFixedSize(96,96);
    systemPortraitLab->setPixmap(QPixmap(":/img/upgrade/normal.png").scaled(QSize(96,96)));

    /*中间布局*/
    QWidget *inforWidget = new QWidget();
    inforLayout = new QVBoxLayout(inforWidget);
    inforWidget->setLayout(inforLayout);
    versionErrorcodeLab= new QWidget(inforWidget);
    versionErrorcodeLablayout = new QHBoxLayout(versionErrorcodeLab);
    versionInformationLab = new QLabel(versionErrorcodeLab);
    versionErrorcodeLab->setLayout(versionErrorcodeLablayout);
    versionErrorcodeLablayout->addWidget(versionInformationLab);
    versionErrorcodeLablayout->setSpacing(0);
    versionErrorcodeLablayout->addStretch(0);
    versionErrorcodeLablayout->setContentsMargins(0,0,0,0);
    versionErrorcodeLab->setFixedHeight(versionInformationLab->height()+4);
    allProgressBar = new KProgressBar(systemWidget);
    allProgressBar->setFixedHeight(20);
    allProgressBar->setBodyWidth(12);
    allProgressBar->hide();
    allProgressBar->setValue(0);
    lastRefreshTime = new LightLabel(systemWidget);
    inforLayout->addStretch();
    inforLayout->addWidget(versionErrorcodeLab);
    inforLayout->addWidget(allProgressBar,Qt::AlignVCenter);
    inforLayout->addWidget(lastRefreshTime);
    inforLayout->addStretch();
    allProgressBar->hide();

    /*右侧按键布局*/
    checkUpdateBtn = new m_button(systemWidget);
    checkUpdateBtn->setDefault(true);
    checkUpdateBtn->start();

    tab1HLayout->addWidget(systemPortraitLab,0,Qt::AlignHCenter|Qt::AlignLeft);
    tab1HLayout->addWidget(inforWidget,Qt::AlignLeft|Qt::AlignTop);
    tab1HLayout->addWidget(checkUpdateBtn);
    tab1HLayout->setContentsMargins(16,16,16,16);
    tab1HLayout->setSpacing(0);



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
    isAutoCheckSBtn = new KSwitchButton();
    isAutoCheckSBtn->setFixedWidth(50);
    isAutoCheckWidget->setLayout(isAutoCheckLayout);
    isAutoCheckLayout->setContentsMargins(8, 0, 14, 0);
    isAutoCheckLayout->addSpacing(6);
    isAutoCheckLayout->addWidget(isAutoCheckedLab);
    isAutoCheckLayout->addWidget(isAutoCheckSBtn);

    /*switchbutton2：自动下载和安装更新*/
    isAutoUpgradeWidget = new QFrame();
    isAutoUpgradeWidget->setFrameShape(QFrame::Box);
    isAutoUpgradeWidget->setFixedHeight(60);
    isAutoUpgradeWidget->hide();
    isAutoUpgradeLayout = new QVBoxLayout();
    isAutoUpgradeLayout->setContentsMargins(8, 0, 14, 0);
    isAutoUpgradeLab = new QLabel();
    isAutoUpgradeLab->setText(tr("Automatically updates"));
    autoUpgradeLab = new LightLabel(tr("The system will automatically updates when there is an available network and backup."));
    autoUpgradeLab->setWordWrap(true);
    autoUpgradeLab->setAlignment(Qt::AlignTop);
    autoUpgradeLab->hide();
    isAutoUpgradeSBtn = new KSwitchButton();
    isAutoUpgradeSBtn->setFixedWidth(50);
    autoUpgradeBtnLayout = new QHBoxLayout();
    autoUpgradeBtnLayout->addSpacing(6);
    autoUpgradeBtnLayout->addWidget(isAutoUpgradeLab);
    autoUpgradeBtnLayout->addWidget(isAutoUpgradeSBtn);

    /*switchbutton3：下载限速*/
    DownloadVWidget = new QFrame();
    DownloadVWidget->setFrameShape(QFrame::Box);
    DownloadVWidget->setFixedHeight(60);
    DownloadVWidget->hide();
    DownloadVLayout = new QVBoxLayout();
    DownloadVLayout->setContentsMargins(8, 0, 14, 0);
    DownloadHLayout = new QHBoxLayout();
    DownloadVLabLayout = new QHBoxLayout();
    DownloadHLab = new QLabel();
    DownloadHLab->setText(tr("Download Limit"));
    DownloadHBtn = new KSwitchButton();
    DownloadHBtn->setFixedWidth(50);
    DownloadHValue = new QComboBox();
    QStringList strList;
    strList<<"50 kB/s"<<"100 kB/s"<<"200 kB/s"<<"300 kB/s"<<"600 kB/s";
    DownloadHValue->addItems(strList);
    DownloadVLab = new LightLabel(tr("It will be avaliable in the next download."));
    DownloadVLab->setWordWrap(true);
    DownloadVLab->setAlignment(Qt::AlignTop);
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

    /*switchbutton4：提供反馈*/
    TrailWidget = new QFrame();
    TrailWidget->setFrameShape(QFrame::Box);
//    TrailWidget->setFixedHeight(60);
    TrailWidget->setMinimumHeight(60);
    TrailLayout = new QHBoxLayout();
    TrailLayout->setContentsMargins(8, 0, 14, 0);
    TrailLabel1 = new FixLabel();
    TrailLabel1->setText(tr("Accept beta version and "));
    TrailLabel2 = new FixLabel();
    TrailLabel2->setText(tr("to help improving the system"));
    TrailBorderlessButton = new KBorderlessButton();
    TrailBorderlessButton->setText(tr("provide feedback"));
    TrailCheckBtn = new KSwitchButton();
    TrailCheckBtn->setFixedWidth(50);
    TrailLayout->addSpacing(6);
    TrailLayout->addWidget(TrailLabel1);
    TrailLayout->addWidget(TrailBorderlessButton);
    TrailLayout->addWidget(TrailLabel2);
    TrailLayout->addStretch();
    TrailLayout->addWidget(TrailCheckBtn);
    TrailWidget->setLayout(TrailLayout);

    /*switchbutton5：还原路径*/
    RestoreWidget = new QFrame();
    RestoreWidget->setFrameShape(QFrame::Box);
    RestoreWidget->setFixedHeight(60);
    RestoreLayout = new QHBoxLayout();
    RestoreLayout->setContentsMargins(8, 0, 14, 0);
    RestoreLabel = new QLabel();
    RestoreLabel->setText(tr("Rollback to previous version"));
    RestoreButton = new QPushButton();
    RestoreButton->setText(tr("start"));
    RestoreButton->setFixedWidth(120);
    RestoreLayout->addSpacing(6);
    RestoreLayout->addWidget(RestoreLabel);
    RestoreLayout->addWidget(RestoreButton);
    RestoreWidget->setLayout(RestoreLayout);

    isAutoUpgradeLayout->addLayout(autoUpgradeBtnLayout);
    autoUpgradeLayout = new QHBoxLayout();
    autoUpgradeLayout->addSpacing(6);
    autoUpgradeLayout->addWidget(autoUpgradeLab);
    isAutoUpgradeLayout->addLayout(autoUpgradeLayout);
    isAutoUpgradeWidget->setLayout(isAutoUpgradeLayout);
    /*临时加入的分割线，在后期可以化成一个模块*/
    line12 = new QFrame(this);
    line12->setMinimumSize(QSize(0, 1));
    line12->setMaximumSize(QSize(16777215, 1));
    line12->setLineWidth(0);
    line12->setFrameShape(QFrame::HLine);
    line12->setFrameShadow(QFrame::Sunken);
    line12->hide();
    line23 = new QFrame(this);
    line23->setMinimumSize(QSize(0, 1));
    line23->setMaximumSize(QSize(16777215, 1));
    line23->setLineWidth(0);
    line23->setFrameShape(QFrame::HLine);
    line23->setFrameShadow(QFrame::Sunken);
    line23->hide();
    line34 = new QFrame(this);
    line34->setMinimumSize(QSize(0, 1));
    line34->setMaximumSize(QSize(16777215, 1));
    line34->setLineWidth(0);
    line34->setFrameShape(QFrame::HLine);
    line34->setFrameShadow(QFrame::Sunken);
    line45= new QFrame();
    line45->setMinimumSize(QSize(0, 1));
    line45->setMaximumSize(QSize(16777215, 1));
    line45->setLineWidth(0);
    line45->setFrameShape(QFrame::HLine);
    line45->setFrameShadow(QFrame::Sunken);

    QWidget *historyUpdateLogWid = new QWidget(this);
    QHBoxLayout *historyUpdateLogLayout = new QHBoxLayout();
    historyUpdateLog = new QPushButton(this); // 历史日志弹出窗口控制按钮
    //    historyUpdateLog->setText(tr("更新历史"));
    historyUpdateLog->setText(tr("View history"));
    historyUpdateLog->setFixedHeight(36);
    AdvancedOption = new QPushButton(this);
    AdvancedOption->setText(tr("Advanced"));
    historyUpdateLog->setFixedHeight(36);
    historyUpdateLog->setFixedWidth(132);
    AdvancedOption->setFixedWidth(132);
    AdvancedOption->hide();
    historyUpdateLogWid->setContentsMargins(2,0,0,0);
    historyUpdateLogWid->setLayout(historyUpdateLogLayout);
    historyUpdateLogLayout->setAlignment(Qt::AlignLeft);
    historyUpdateLogLayout->addWidget(historyUpdateLog);
    historyUpdateLogLayout->setSpacing(5);
    historyUpdateLogLayout->addWidget(AdvancedOption);
    historyUpdateLogLayout->setMargin(0);

    /*第二个titlelabel的布局*/
    updatesettingLayout->setAlignment(Qt::AlignTop);
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
    updatesettingLayout->addWidget(line34);
    updatesettingLayout->addWidget(TrailWidget);
    updatesettingLayout->setSpacing(2);
    updatesettingLayout->setMargin(0);
    updatesettingLayout->addWidget(line45);
    updatesettingLayout->addWidget(RestoreWidget);
    updatesettingLayout->setSpacing(2);
    updatesettingLayout->setMargin(0);
    AppMessage->addWidget(labUpdate);
    AppMessage->addWidget(systemWidget);
    AppMessage->addWidget(allUpdateWid);
    AppMessage->addSpacing(30);
    AppMessage->addWidget(updateSettingLab);
    AppMessage->addWidget(updateSettingWidget);
    AppMessage->addSpacing(6);
    AppMessage->addWidget(historyUpdateLogWid);
    AppMessage->addStretch();
    AppMessage->setContentsMargins(0,0,0,0);

    getAllDisplayInformation();
    setVisibleBySecurity();
    QDBusConnection::sessionBus().connect(QString(), QString("/"), "org.ukui.ukcc.session.interface", "configChanged", this, SLOT(setVisibleBySecurity()));

}

void TabWid::checkUpdateBtnClicked()
{
    qInfo()<<"get in checkUpdateBtnClicked";
    if(checkUpdateBtn->text() == tr("Cancel"))
    {
        qInfo()<<"cancel download";
        updateMutual->interface->call("CancelDownload");
        versionInformationLab->setText(tr("update has been canceled!"));
        disconnect(updateMutual->interface,SIGNAL(UpdateDownloadInfo(int,int,quint64,quint64,int)),this,SLOT(showDownloadInfo(int,int,quint64,quint64,int)));
        disconnect(updateMutual->interface,SIGNAL(UpdateDownloadFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlotindownload(bool,QStringList,QString,QString)));
        checkUpdateBtn->hide();
        allProgressBar->hide();
        lastRefreshTime->hide();
        return;
    }
    QDBusMessage msg = updateMutual->interface->call("GetBackendStatus",getLanguageEnv());
    backstatus = msg.errorMessage().isEmpty();
    if(!backstatus)
    {
        qInfo()<<"the dbus is not start";
        versionInformationLab->setText(tr("The updater is NOT start"));
        checkUpdateBtn->hide();
        return;
    }
    int backendStatus = msg.arguments().value(0).toInt();
    switch(backendStatus)
    {
        qInfo()<<"backendStatus is "<<backendStatus;
        case ACTION_DEFUALT_STATUS:
        break;
        case ACTION_UPDATE:
            qInfo()<<"access to ACTION_UPDATE";
            versionInformationLab->setText(tr("The progress is updating..."));
            checkUpdateBtn->hide();
            connect(updateMutual->interface,SIGNAL(UpdateDetectFinished(bool,QStringList,QString,QString)),this,SLOT(slotUpdateCache(bool,QStringList,QString,QString)));
            return;
        break;
        case ACTION_DOWNLOADONLY:
            qInfo()<<"access to ACTION_DOWNLOADONLY";
            versionInformationLab->setText(tr("The progress is downloading..."));
            checkUpdateBtn->setEnabled(false);
            allProgressBar->show();
            checkUpdateBtn->setText(tr("Cancel"));
            lastRefreshTime->setText(tr("In download"));
            connect(updateMutual->interface,SIGNAL(UpdateDownloadInfo(int,int,quint64,quint64,int)),this,SLOT(showDownloadInfo(int,int,quint64,quint64,int)));
            connect(updateMutual->interface,SIGNAL(UpdateDownloadFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlotindownload(bool,QStringList,QString,QString)));
            return;
        break;
        default:
            versionInformationLab->setText(tr("The updater is busy！"));
            return;
        break;
    }
    //后续需要添加判断接口是否生效的接口
    if(checkUpdateBtn->text() == tr("Retry"))
    {
        qInfo()<<"access to Retry";
        connect(updateMutual->interface,SIGNAL(UpdateDetectFinished(bool,QStringList,QString,QString)),this,SLOT(slotUpdateCache(bool,QStringList,QString,QString)));
        updateMutual->interface->call("UpdateDetect");
        versionInformationLab->setText(tr("Updating the software source")+"...");
        checkUpdateBtn->start();
        checkUpdateBtn->setEnabled(false);
        return;
    }
    else if(checkUpdateBtn->text() == tr("Check Update"))
    {
        qInfo()<<"access to Check Update";
        connect(updateMutual->interface,SIGNAL(UpdateDetectFinished(bool,QStringList,QString,QString)),this,SLOT(slotUpdateCache(bool,QStringList,QString,QString)));
        updateMutual->interface->call("UpdateDetect");
        versionInformationLab->setText(tr("Updating the software source")+"...");
        checkUpdateBtn->start();
        checkUpdateBtn->show();
        checkUpdateBtn->setEnabled(false);
        return;
    }
    /*
    else if(checkUpdateBtn->text() == tr("reboot rightnow"))
    {
        qInfo()<<"reboot rightnow";
        QDBusMessage rebootresult = updateMutual->interface->call("DeployLatestUpdate","reboot");
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
        return;
    }
    else if(checkUpdateBtn->text() == tr("UpdateAll"))
    {
        system("systemctl --user start kylin-stepinstall-notify.service");
        system("systemctl --user enable kylin-stepinstall-notify.service");
        connect(updateMutual->interface,SIGNAL(UpdateDownloadInfo(int,int,uint,uint,int)),this,SLOT(showDownloadInfo(int,int,uint,uint,int)));
        connect(updateMutual->interface,SIGNAL(UpdateDownloadFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlotindownload(bool,QStringList,QString,QString)));
        qInfo() << "updateAllApp";
        checkUpdateBtn->setEnabled(true);
        lastRefreshTime->setText(tr("The system is updating..."));
        updateMutual->interface->call("UpdateDownloadAll");
        return;
    }
    */
}

QString TabWid::getLanguageEnv()
{
    qInfo()<<"getLanguageEnv";
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

void TabWid::slotUpdateCache(bool result,QStringList pkgname, QString error ,QString reason)
{
    qInfo()<<"get into slotUpdateCache";
    AppAllMsg msg;
    lastRefreshTime->show();
    checkUpdateBtn->show();
    qInfo() <<"更新成功结果:" << result;
    if (result){
        versionInformationLab->setText(tr("Getting update list")+"...");
        QList<QString>::Iterator it = pkgname.begin(),itend = pkgname.end();
        int i = 0;
        for (;it != itend; it++,i++)
        {
            qInfo() << "slotUpdateCache函数：获取到的包列表：" << pkgname[i];
            getpkginfoFromJson(&msg,pkgname[i]);
            msg.name=pkgname[i];
            qInfo()<<"msg'name is" <<msg.name;
            loadingOneUpdateMsgSlot(msg);
        }
        loadingFinishedSlot();
     }
     else{
        qInfo() << "更新失败 error_code:" <<error_code;
        checkUpdateBtn->setEnabled(true);
        checkUpdateBtn->stop();
        checkUpdateBtn->setText(tr("Retry"));
        if(error_code == "#0102"){
            qInfo()<<"Internet error";
            versionInformationLab->setText(tr("Network exception, unable to check for updates!"));
        }
        else if(error_code == "#0201")
        {
            qInfo()<<"disk space is insufficiense";
            versionInformationLab->setText(tr("Disk space is insufficiense clean the disk and then upgrade."));
            checkUpdateBtn->hide();
        }
        else if(error_code == "#0204"){
            qInfo()<<"no room to backup";
            versionInformationLab->setText(tr("No room, upgrade failed."));
            checkUpdateBtn->show();
        }
        else if(error_code == "#0208"){
            qInfo()<<"low power";
            versionInformationLab->setText(tr("Battery level is below 50%,and upgrade failed."));
            checkUpdateBtn->show();
        }
        else{
            qInfo()<<"show error code";
            versionInformationLab->setText(tr("Checking update failed! "));
            checkedtime = tr("No information!");
            QSqlQuery query(QSqlDatabase::database("A"));
            query.exec("select * from display");
            while(query.next()){
                checkedtime = datetimeutils->TranslationTime(query.value("check_time").toString());
            }
            lastRefreshTime->setText(tr("Last Checked:")+checkedtime);
        }
        checkUpdateBtn->show();
        disconnect(updateMutual->interface,SIGNAL(UpdateDetectFinished(bool,QStringList,QString,QString)),this,SLOT(slotUpdateCache(bool,QStringList,QString,QString)));
    }
}

void TabWid::getpkginfoFromJson(AppAllMsg *msg,QString pkgname)
{
    qInfo()<<"getpkginfoFromJson";
    /*判断json文件是否存在*/
    QString filename = "/var/cache/kylin-system-updater/json/"+pkgname +".json";
    qInfo() << filename;
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)){
        qInfo() << "JSON file open failed! ";
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
                QString name  = object.value("update-name").toObject().value("zh_CN").toString();
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
                QString name  = object.value("update-name").toObject().value("en_US").toString();
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
            QString new_version = object.value("version").toString();
            if (!new_version.isNull())
                msg->availableVersion=new_version;
            QString cur_version = object.value("version").toString();
            if (!cur_version.isNull())
                msg->version=cur_version;
            QString changelog = object.value("changelog").toString();
            if (!changelog.isNull())
                msg->longDescription=changelog;
        }
    }
    else{
        qInfo() << "JSON文件格式错误！";
        return ;
    }
    file.close();
}

void TabWid::loadingOneUpdateMsgSlot(AppAllMsg msg)
{
    qInfo()<<"loadingOneUpdateMsgSlot";
    ukscConnect = new UKSCConn();
    if(updateMutual->importantList.indexOf(msg.name) == -1){
        qInfo()<<"index = -1";
        struct pkgProgress pkg;
        pkg.name = msg.name;
        pkgList << pkg;
        qInfo()<<"the msg.name is "<< msg.name;
        updateMutual->importantList.append(msg.name);   //重要更新列表中添加appname
        AppUpdateWid *appWidget = new AppUpdateWid(msg, this);
        widgetList << appWidget;
        connect(appWidget, &AppUpdateWid::appupdateiscancel, this, &TabWid::slotCancelDownload);
        connect(appWidget, &AppUpdateWid::changeTabWidText , this, &TabWid::changeTabWidTextSlot);
        if(ukscConnect->isConnectUskc == true)
        {
            QStringList list = ukscConnect->getInfoByName(msg.name);
            if(list[2] != "")
            {
                qInfo()<<"ukscconnect->appnamelab is"<<list[2];
                appWidget->appNameLab->setText(list[2]);
                appWidget->dispalyName = list[2];

            }
            if(list[1] != "" && QLocale::system().name()=="zh_CN")
            {
                qInfo()<<"ukscconnect->appnamelab is"<<list[1];
                appWidget->appNameLab->setText(list[1]);
                appWidget->dispalyName = list[1];
            }
            if(list[0] != "" && !appWidget->haveThemeIcon)
            {
                appWidget->appIcon->setPixmap(QPixmap(list[0]).scaled(32, 32));
            }
        }
        allUpdateLayout->addWidget(appWidget);
        qInfo() <<"loadingOneUpdateMsgSlot:" << appWidget->dispalyName;
    }
}

void TabWid::loadingFinishedSlot()
{
    qInfo()<<"loadingFinishedSlot";
    disconnect(updateMutual->interface,SIGNAL(UpdateDetectFinished(bool,QStringList,QString,QString)),this,SLOT(slotUpdateCache(bool,QStringList,QString,QString)));
//    disconnect(updateMutual->interface,SIGNAL(UpdateDetectStatusChanged(int,QString)),this,SLOT(slotUpdateCacheProgress(int,QString)));
    if(updateMutual->importantList.size() == 0)
    {
        qInfo()<<"updateMutual software num is 0";
        checkUpdateBtn->setEnabled(true);
        checkUpdateBtn->stop();
        //        checkUpdateBtn->setText(tr("检查更新"));
        checkUpdateBtn->setText(tr("Check Update"));
        version = getversion();
        versionInformationLab->setText(tr("Your system is the latest:")+version);
        checkUpdateBtn->hide();
        checkedtime = tr("No information!");
        QSqlQuery query(QSqlDatabase::database("A"));
        query.exec("select * from display");
        while(query.next())
        {
            checkedtime = datetimeutils->TranslationTime(query.value("check_time").toString());
        }
        lastRefreshTime->setText(tr("Last Checked:")+checkedtime);
        lastRefreshTime->show();
        allProgressBar->hide();
    }
    else
    {
        updateMutual->importantSize = updateMutual->importantList.size();   //此次检测结果的更新数量
        qInfo()<<"updateMutual software num is "<<updateMutual->importantSize;
        checkUpdateBtn->stop();
        checkUpdateBtn->setEnabled(true);
        //        checkUpdateBtn->setText(tr("全部更新"));
        checkUpdateBtn->setText(tr("UpdateAll"));
        checkUpdateBtn->hide();
        checkUpdateBtn->adjustSize();
        //        versionInformationLab->setText(tr("检测到你的系统有可更新的应用！"));
        versionInformationLab->setText(tr("Updatable app detected on your system!"));
        checkedtime = tr("No information!");
        QSqlQuery query(QSqlDatabase::database("A"));
        query.exec("select * from display");
        while(query.next())
        {
            checkedtime = datetimeutils->TranslationTime(query.value("check_time").toString());
        }
        lastRefreshTime->setText(tr("Last Checked:")+checkedtime);
        systemPortraitLab->setPixmap(QPixmap(":/img/upgrade/update.png").scaled(96,96));
    }
}

void TabWid::isCancelabled(bool status)
{
    if(status){
        checkUpdateBtn->setEnabled(true);
    }
    else{
        checkUpdateBtn->setEnabled(false);
    }
}

void TabWid::showDownloadInfo(int currentserial,int total,quint64 downloadedsize,quint64 totalsize,int speed)
{
    qInfo()<<"showDownloadinfo";
    qInfo()<<currentserial<<total<<downloadedsize<<totalsize<<speed;
    int Progress = 100*currentserial/total;
    qInfo() << Progress ;
    versionInformationLab->setText(tr("The system is downloading the update!"));
    if (allProgress<Progress)
    {
        allProgress=Progress;
    }
    allProgressBar->setValue(allProgress);
    allProgressBar->show();
    lastRefreshTime->show();
    if(speed > 0)
        alldownloadstart=true;
    QString downloadspeed = modifySpeedUnit(speed, 1);
    if(speed == 0 && !alldownloadstart)
    {
        lastRefreshTime->setText(tr("In the download")+":"+modifySizeUnit(downloadedsize)+"/"+modifySizeUnit(totalsize)+"("+tr("calculating")+")");
        return;
    }
    lastRefreshTime->setText(tr("In the download")+":"+modifySizeUnit(downloadedsize)+"/"+modifySizeUnit(totalsize)+"("+downloadspeed+")");
    checkUpdateBtn->setText(tr("Cancel"));
    checkUpdateBtn->setEnabled(true);
}

void TabWid::hideUpdateBtnSlotindownload(bool state,QStringList pkgname,QString error,QString reason)
{
    qInfo()<<"[INFO]get in the hideUpdateBtnSlotindownload fuction";
    Q_UNUSED(pkgname);
    checkUpdateBtn->setEnabled(true);
    checkUpdateBtn->stop();
    checkUpdateBtn->setText(tr("Check Update"));
    lastRefreshTime->setText(tr("Finish the download!"));
    if(state)
    {
        qInfo()<<"[INFO]the state is "<<state;
        versionInformationLab->setText(tr("The system has download the update，and you are suggested to reboot to use the new version."));
        checkUpdateBtn->setText(tr("reboot rightnow"));
        checkUpdateBtn->show();
        allProgressBar->hide();
        systemPortraitLab->setPixmap(QPixmap(":/img/upgrade/normal.png").scaled(96,96));
    }
    else
    {
        if(error_code == "#0102")
        {
            qInfo()<<"Internet error";
            versionInformationLab->setText(tr("Network exception, unable to check for updates!"));
        }
        else if(error_code == "#0201")
        {
            qInfo()<<"disk space is insufficiense";
            versionInformationLab->setText(tr("Disk space is insufficiense clean the disk and then upgrade."));
            checkUpdateBtn->hide();
        }
        else if(error_code == "#0204")
        {
            qInfo()<<"no room to backup";
            versionInformationLab->setText(tr("No room to backup,upgrade failed."));
            checkUpdateBtn->hide();
        }
        else if(error_code == "#0208")
        {
            qInfo()<<"low power";
            versionInformationLab->setText(tr("Battery level is below 50%,and upgrade failed."));
            checkUpdateBtn->hide();
        }
        else
        {
            versionInformationLab->setText(tr("Update failed! "));
            qInfo()<<"show error code of update1";
            checkUpdateBtn->hide();
        }
        allProgressBar->hide();
        lastRefreshTime->setText(error);
    }
    disconnect(updateMutual->interface,SIGNAL(UpdateDownloadInfo(int,int,uint,uint,int)),this,SLOT(showDownloadInfo(int,int,quint64,quint64,int)));
    disconnect(updateMutual->interface,SIGNAL(UpdateDownloadFinished(bool,QStringList,QString,QString)),this,SLOT(hideUpdateBtnSlotindownload(bool,QStringList,QString,QString)));
    checkedtime = tr("No information!");
    QSqlQuery query(QSqlDatabase::database("A"));
    query.exec("select * from display");
    while(query.next())
    {
        checkedtime = datetimeutils->TranslationTime(query.value("check_time").toString());
    }
    lastRefreshTime->setText(tr("Last Checked:")+checkedtime);
    lastRefreshTime->show();
}

/*静态流程函数*/
void TabWid::ShowAdvancedOption()
{
    SetWidget *setWidget = new SetWidget(this);
    setWidget->setFixedSize(setWidget->width(),setWidget->height());
    setWidget->show();
}

void TabWid::SecurityDownloadChange(QString button, QString content)
{
    qInfo()<<"get in SecurityDownloadChange"<<button<<content;
    if(button == "speed")
    {
        disconnect(DownloadHBtn,&KSwitchButton::stateChanged,this,&TabWid::DownloadLimitChanged);
        disconnect(DownloadHValue,&QComboBox::currentTextChanged,this,&TabWid::DownloadLimitValueChanged);
        if(content == "0")
        {
            DownloadHBtn->setChecked(false);
            DownloadHValue->setEnabled(false);
        }
        else
        {
            DownloadHBtn->setChecked(true);
            DownloadHValue->setEnabled(true);
            DownloadHValue->setCurrentText(content+" kB/s");
        }
        connect(DownloadHBtn,&KSwitchButton::stateChanged,this,&TabWid::DownloadLimitChanged);
        connect(DownloadHValue,&QComboBox::currentTextChanged,this,&TabWid::DownloadLimitValueChanged);
    }
}

void TabWid::ShowFeedback()
{
    qInfo()<<"ShowFeedback";
    QProcess process;
    process.startDetached("kylin-service-support",QStringList());
    if (process.waitForStarted())
    {
        if (process.waitForFinished())
        {
            QProcess::ExitStatus status = process.exitStatus();
            if (status == QProcess::NormalExit)
            {
                // 进程执行成功
                qInfo() << "Process executed successfully";
            }
            else
            {
                // 进程执行失败
                qInfo() << "Process execution failed";
                QDBusInterface iface("org.freedesktop.Notifications",
                                     "/org/freedesktop/Notifications",
                                     "org.freedesktop.Notifications",
                                     QDBusConnection::sessionBus());
                QList<QVariant> args;
                QVariantMap hints;
                QString message = tr("feedback UI end failed!");
                args<<(tr("System-Upgrade"))
                    <<((unsigned int) 0)
                    <<("ukui-control-center")
                    <<tr("ukui-control-center-update")//显示的是什么类型的信息  控制面板-更新提示
                    <<message //显示的具体信息
                    <<QStringList()
                    <<hints
                    <<(int)-1;
                iface.callWithArgumentList(QDBus::AutoDetect,"Notify",args);
            }
        }
    }
    else
    {
        QProcess::ProcessError error = process.error();
        if (error == QProcess::FailedToStart)
        {
            qInfo() << "Failed to start process";
            QDBusInterface iface("org.freedesktop.Notifications",
                                 "/org/freedesktop/Notifications",
                                 "org.freedesktop.Notifications",
                                 QDBusConnection::sessionBus());
            QList<QVariant> args;
            QVariantMap hints;
            QString message = tr("feedback UI start failed!");
            args<<(tr("System-Upgrade"))
                <<((unsigned int) 0)
                <<("ukui-control-center")
                <<tr("ukui-control-center-update")//显示的是什么类型的信息  控制面板-更新提示
                <<message //显示的具体信息
                <<QStringList()
                <<hints
                <<(int)-1;
            iface.callWithArgumentList(QDBus::AutoDetect,"Notify",args);
        }
    }
    return ;
}

void TabWid::FeedbackStatusChanged()
{
    qInfo()<<"FeedbackStatusChanged";
    QDBusInterface properties_interface( "com.kylin.systemupgrade",
                                 "/com/kylin/systemupgrade",
                                 "com.kylin.systemupgrade.interface",
                                 QDBusConnection::systemBus() );
    QDBusMessage earlyAccessStatus_message;
    if(TrailCheckBtn->isChecked() == false){
        earlyAccessStatus_message = properties_interface.call("SetEarlyaccessStatus",false);
        if (earlyAccessStatus_message.type() == QDBusMessage::ErrorMessage) {
            qInfo() << "D-Bus call failed:" << earlyAccessStatus_message.errorMessage();
        }
    }
    else if(TrailCheckBtn->isChecked() == true){
        earlyAccessStatus_message = properties_interface.call("SetEarlyaccessStatus",true);
        if (earlyAccessStatus_message.type() == QDBusMessage::ErrorMessage) {
            qInfo() << "D-Bus call failed:" << earlyAccessStatus_message.errorMessage();
        }
    }
    int result = earlyAccessStatus_message.arguments().value(0).toInt();
    qInfo()<<"isEarlyaccessStatus:"<<result;
    if(result){
        qInfo()<<"failed to isEarlyaccessStatus";
    }
    if(!result){
        qInfo()<<"success to isEarlyaccessStatus";
    }
}

void TabWid::OpenBackupTool()
{
    qInfo()<<"OpenBackupTool";
    int ret=0;
    QMessageBox msgBox(qApp->activeWindow());
    msgBox.setText(tr("Please make sure your important files are saved before rollback"));
    msgBox.setWindowTitle(tr("Prompt information"));
    msgBox.setIcon(QMessageBox::Icon::Warning);
    msgBox.addButton(tr("Cancel"),QMessageBox::NoRole);//QMessageBox::NoRole
    msgBox.addButton(tr("Continue to Rollback"),QMessageBox::YesRole); //QMessageBox::YesRole
    ret = msgBox.exec();
    qDebug()<<"you choose:"<<ret;
    if(ret == 1)
    {
//        updateMutual->interface->call("RollbackSysVersion",1,"");
        QStringList backupList = result.split(';');
        BackupEntry latestBackup;
        qInfo()<<"backuplist is "<<backupList;
        for (const QString &backup : backupList) {
            qInfo()<<"OpenBackupTool backup is "<<backup;
            BackupEntry currentBackup = parseBackupEntry(backup);
            if (currentBackup.timestamp.isValid() && (latestBackup.timestamp.isNull() || currentBackup.timestamp > latestBackup.timestamp)) {
                latestBackup = currentBackup;
                qInfo()<<"get latestBackup:"<<latestBackup.timestamp;
            }
        }
        if (latestBackup.timestamp.isValid()) {
            qInfo() << "Latest Backup:";
            qInfo() << "Description:" << latestBackup.description;
            qInfo() << "UUID:" << latestBackup.uuid;
            qInfo() << "Timestamp:" << latestBackup.timestamp;//.toString(Qt::ISODate)
        } else {
            qInfo() << "No valid backups found.";
        }
        QDBusInterface dbusInterface( "com.kylin.systembackup",
                                      "/update",
                                      "com.kylin.systembackup.manager",
                                      QDBusConnection::systemBus());
        if (!dbusInterface.isValid()) {
            qInfo() << "Failed to connect to DBus service.";
            return ;
        }
        QString userName = qgetenv("USER");
        int uid = getuid();
        QDBusReply<QString> reply =
                dbusInterface.call("autoRestoreForSystemUpdate",latestBackup.uuid,userName,uid);
    }
    else
    {
        ;
    }
}

BackupEntry TabWid::parseBackupEntry(const QString &entry) {
    BackupEntry backup;
    QStringList fields = entry.split(',');
    if (fields.size() == 3) {
        backup.description = fields.at(0);
        backup.uuid = fields.at(1);
        backup.timestamp = QDateTime::fromString(fields.at(2), "yyyy-MM-dd HH:mm:ss");
        qInfo()<<"parseBackupEntry get "<<backup.timestamp;
    }
    return backup;
}

void TabWid::DownloadLimitChanged()
{
    qInfo()<<"DownloadLimitChanged";
    if(DownloadHBtn->isChecked() == false)
    {
        qInfo()<<"download limit disabled";
        DownloadHValue->setEnabled(false);
        updateMutual->SetDownloadLimit(0,false);
        updateMutual->insertInstallStates("download_limit","false");

    }
    else if(DownloadHBtn->isChecked() == true)
    {
        updateMutual->insertInstallStates("download_limit","true");
        qInfo()<<"download limit enabled";
        DownloadHValue->setEnabled(true);
        QString dlimit = DownloadHValue->currentText();
        QString limit = dlimit.replace(QString(" kB/s"),"");
        updateMutual->SetDownloadLimit(limit,true);
    }
}

void TabWid::isAutoUpgradeChanged()
{
    qInfo()<<"isAutoUpgradeChanged";
    bool list;
    if (isAutoUpgradeSBtn->isChecked())
    {
        list = true;
    }
    else
    {
        list = false;
    }
    updateMutual->SetAutoUpgradeState(list);
}

void TabWid::showHistoryWidget()
{
    qInfo() << "showHistoryWidget";
    ukcc::UkccCommon::buriedSettings("Upgrade","historyLog",QString("clicked"));
    historyLog = m_updatelog::GetInstance(this);
    historyLog->show();
}

void TabWid::UpdateSdkTime(QString dateformat)
{
    qInfo()<<"UpdateSdkTime";
    QSqlQuery query(QSqlDatabase::database("A"));
    query.exec("select * from display");
    while(query.next())
    {
        checkedtime = datetimeutils->TranslationTime(query.value("check_time").toString());
    }
    lastRefreshTime->setText(tr("Last Checked:")+checkedtime);
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
    QString upgradeversion = QString("KylinOS V11 %1 (%2)").arg(upgradeversion1).arg(upgradeversion2);
    if(upgradeversion1.isEmpty()||upgradeversion1.isNull())
    {
        upgradeversion = QString("KylinOS V11");
    }
    else if(upgradeversion2.isEmpty()||upgradeversion2.isNull())
    {
        upgradeversion = QString("KylinOS V11").arg(upgradeversion1);
    }
    return upgradeversion;
}

void TabWid::showUserGuide(QString programName)
{
    QString dbusName = "com.kylinUserGuide.hotel_" + QString::number(getuid());

    QDBusInterface *serviceInterface =  new QDBusInterface(dbusName,"/","com.guide.hotel",QDBusConnection::sessionBus());
    if(!serviceInterface->isValid())
    {
        qDebug() << "用户手册" <<"Service Interface: " << qPrintable(QDBusConnection::sessionBus().lastError().message());
        return;
    }
    serviceInterface->call(QDBus::Block, "showGuide", QVariant::fromValue(QString(programName)));
}

//转换包大小的单位
QString TabWid::modifySizeUnit(long size)
{
    qInfo()<<"modifySizeUnit";
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


QString TabWid::caltime(uint downloadedsize,uint totalsize,int speed)
{
    int cha = totalsize-downloadedsize;
    qInfo()<<"cha is"<<cha;
    if(cha<0)
        return QString("%1").arg("----");
    else
    {
        if(speed <= 0)
            return QString("%1").arg("----");
        else
        {
            uint time = cha/speed;
            qInfo()<<"time is"<<cha;
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
            return QString("%1%2").arg((int)(size/1024)).arg("kB/S");
    }
    else
        return QString("%1%2").arg(size).arg("B/S");
}

void TabWid::isAutoCheckedChanged()     //自动检测按钮绑定的槽函数
{
    qInfo()<<"isAutoCheckedChanged";
    if(isAutoCheckSBtn->isChecked() == false)
    {
        updateMutual->insertInstallStates("auto_check","false");

    }
    else if(isAutoCheckSBtn->isChecked() == true)
    {
        updateMutual->insertInstallStates("auto_check","true");
    }
}

TabWid::~TabWid()
{
    qInfo() << "~TabWid" ;
    delete updateMutual;
    updateMutual = nullptr;
}

void TabWid::slotUpdateCacheProgress(int progress, QString status)
{
    qInfo()<<"slotUpdateCacheProgress";
    checkUpdateBtn->hide();
    isConnectSourceSignal = true;
    QString updatetime =  tr("No Information!");
    if (progress > 100) {
        return;
    }
    if  (lastprogress > progress)
        return;
    lastprogress = progress;
    lastRefreshTime->hide();
    if(lastprogress==92)
    {
        lastprogress=0;
        checkedtime = tr("No information!");
        QSqlQuery query(QSqlDatabase::database("A"));
        query.exec("select * from display");
        while(query.next())
        {
            checkedtime = datetimeutils->TranslationTime(query.value("check_time").toString());
        }
        lastRefreshTime->setText(tr("Last Checked:")+checkedtime);
    }
    qInfo() << "update cache progress :" << progress;
    versionInformationLab->setText(tr("The system is checking update :") + QString::number(progress)+"%");
}


void TabWid::getAllDisplayInformation()
{
    qInfo()<<"get in getAllDisplayInformation";
    QSqlQuery query(QSqlDatabase::database("A"));
    QString updatetime;
    QString checkedstatues;
    QString downloadlimitstatus;
    QString downloadlimitvalue;
    query.exec("select * from display");
    while(query.next())
    {
        checkedtime = datetimeutils->TranslationTime(query.value("check_time").toString());
        checkedstatues = query.value("auto_check").toString();
        downloadlimitstatus = query.value("download_limit").toString();
        downloadlimitvalue = query.value("download_limit_value").toString();
    }
    qInfo()<<"downloadlimitstatus:"<<downloadlimitstatus;
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
    if(checkedstatues == "false")
    {
        isAutoCheckSBtn->setChecked(false);
    }
    else
    {
        isAutoCheckSBtn->setChecked(true);
    }
    isAutoCheckSBtn->setEnabled(true);
    if(downloadlimitstatus == "false")
    {
        qInfo()<<downloadlimitstatus<<"and the value is"<<downloadlimitvalue;
        DownloadHBtn->setChecked(false);
        DownloadHValue->setCurrentText(downloadlimitvalue+" kB/s");
        DownloadHValue->setEnabled(false);
    }
    else
    {
        qInfo()<<downloadlimitstatus<<"and the value is"<<downloadlimitvalue;
        DownloadHBtn->setChecked(true);
        DownloadHValue->setEnabled(true);
        DownloadHValue->setCurrentText(downloadlimitvalue+" kB/s");
    }
    QDBusInterface dbusInterface( "com.kylin.systembackup",
                                  "/update",
                                  "com.kylin.systembackup.manager",
                                  QDBusConnection::systemBus());
    if (!dbusInterface.isValid()) {
        qInfo() << "Failed to connect to DBus service.";
        return ;
    }
//    QVariant value = dbusInterface.property("autoBackUpList");
    QDBusReply<QString> reply = dbusInterface.call("autoBackUpList");
    if (reply.isValid()) {
        result = reply.value();
        qInfo() << "Result from autoBackUpList:" << result;
    } else {
        qInfo() << "Failed to call D-Bus method:" << reply.error().message();
    }
    if(result.isEmpty())
    {
        line34->hide();
        RestoreWidget->hide();
    }
    QDBusInterface upgrade_interface( "com.kylin.systemupgrade",
                                 "/com/kylin/systemupgrade",
                                 "com.kylin.systemupgrade.interface",
                                 QDBusConnection::systemBus() );
    QDBusMessage earlyAccessStatus_message;
    earlyAccessStatus_message = upgrade_interface.call("GetEarlyaccessStatus");
    bool AccessStatusResult = earlyAccessStatus_message.arguments().value(0).toBool();
    if(AccessStatusResult){
        TrailCheckBtn->setChecked(true);
    }
    else{
        TrailCheckBtn->setChecked(false);
    }
}


void TabWid::DownloadLimitValueChanged(const QString &value)
{
    qInfo()<<"DownloadLimitValueChanged";
    ukcc::UkccCommon::buriedSettings("Upgrade", QString("DownloadHValue"), QString("select"), value);
    if(DownloadHBtn->isChecked()==false)
    {
        updateMutual->SetDownloadLimit(0,false);
        updateMutual->insertInstallStates("download_limit_value","0");
    }
    else if (DownloadHBtn->isChecked()==true)
    {
        QString speedvalue = value;
        QString limit = speedvalue.replace(" kB/s","");
        updateMutual->SetDownloadLimit(limit,true);
        updateMutual->insertInstallStates("download_limit_value",limit);
    }
    else
    {
        qInfo()<<"Download Limit Changed";
        updateMutual->SetDownloadLimit(0,false);
        updateMutual->insertInstallStates("download_limit_value","0");
    }
}

void TabWid::setVisibleBySecurity()
{
    qInfo()<<"get in setVisibleBySecurity";
    // 安全管控 自动更新设置项的显示与隐藏
    QVariantMap ModuleMap = getModuleHideStatus();
    qInfo()<<"ModuleMap is "<< ModuleMap;
    if(ModuleMap.isEmpty())
        return ;
    QString moduleSettings = ModuleMap.value("upgradeSettings").toString();
    qInfo()<<"moduleSettings is "<< moduleSettings;
    QStringList setItems = moduleSettings.split(",");
    qInfo()<<"setItems is "<< setItems;

    foreach (QString setItem, setItems) {
        QStringList item = setItem.split(":");
        qInfo() << "set item Name: " << item.at(0);
        if (item.at(0) == "NotifyFrame") {
            isAutoCheckWidget->setVisible(item.at(1) == "true");
            line12->setVisible(item.at(1) == "true");
        }
        if (item.at(0) == "AutoUpdateFrame") {
            isAutoUpgradeWidget->setVisible(item.at(1) == "true");
            line23->setVisible(item.at(1) == "true");
        }
        if (item.at(0) == "DownloadLimitFrame") {
            DownloadVWidget->setVisible(item.at(1) == "true");
        }
        if (item.at(0) == "AdvancedOptionsBtn") {
            AdvancedOption->setVisible(item.at(1) == "true");
        }
    }
}

QVariantMap TabWid::getModuleHideStatus()
{
    qInfo()<<"getModuleHideStatus";
    QDBusInterface m_interface( "org.ukui.ukcc.session",
                                "/",
                                "org.ukui.ukcc.session.interface",
                                QDBusConnection::sessionBus());

    QDBusReply<QVariantMap> obj_reply = m_interface.call("getModuleHideStatus");
    if (!obj_reply.isValid()) {
        qInfo()<<"execute dbus method getModuleHideStatus failed";
    }
    return obj_reply.value();
}

void TabWid::slotCancelDownload()
{
    versionInformationLab->setText(tr("Update has been canceled!"));
    versionInformationLab->setToolTip("");
    foreach (AppUpdateWid *wid, widgetList) {
        wid->updateAPPBtn->setEnabled(true);
    }
}

void TabWid::changeTabWidTextSlot(){
    versionInformationLab->setText(tr("The system is being updated..."));
}
