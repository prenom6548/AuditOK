#include "updatehandle.h"
#include <iostream>
#include <QProcess>

updatehandle::updatehandle(QString getnum1,QWidget *parent) : QWidget(parent)
{
    timerDownload = new QTimer(this);
    connect(timerDownload, SIGNAL(timeout()), this, SLOT(downloadTimeout()));
    timerDownload->start(20*60*1000);
    qDebug()<<"timer start";
    argnum = getnum1;

    initTranslator();      //加载翻译文件
    getpower();            //获取电量值
    readconf();            //读取配置文件设定值
    initDbus();            //初始化dbus
    initUI(argnum);              //初始化托盘程序
}

updatehandle::~updatehandle()
{
    qDebug() << Q_FUNC_INFO << __LINE__;
}

/**
 * @brief updatehandle::initTranslator
 * 加载翻译文件
 */
void updatehandle::initTranslator()
{
    QTranslator *translator = new QTranslator;
    if (translator->load(QLocale(), QLatin1String("kylin-background-upgrade"), QLatin1String("_"), QLatin1String("/usr/share/kylin-background-upgrade"))) {
        QApplication::installTranslator(translator);
    } else {
        qDebug() << "cannot load translator kylin-background-upgrade_" << QLocale::system().name() << ".qm!";
    }

}

/**
 * @brief updatehandle::getpower
 * 获取电量值
 */
void updatehandle::getpower()
{
    //获取电量信息
    Power_dbus = new power_dbus(this);
    powerState = Power_dbus->initDbusaddrest();
    if (powerState) {
        powerNum = Power_dbus->getPower_Dbus();
        qDebug()<<"电脑电量值为:"<<powerNum;
    }
}

/**
 * @brief updatehandle::initDbus
 * 初始化dbus
 */
void updatehandle::initDbus()
{
    UserIdStatus *arg = new UserIdStatus(); //查看当前是否为用户操作的用户
    if(arg->UIDStatus == "online") {
        qDebug()<<"非活跃窗口，禁止启动";
        exit(0);
    }
    m_updateMutual = update_dbus::getInstance(); //初始化更新管理器d-bus
    m_sourceMutual = source_dbus::getInstance(); //初始化源管理器d-bus

}


/**
 * @brief updatehandle::initUI
 * 初始化界面类
 */
void updatehandle::initUI(QString argnum)
{

    if(argnum.contains("--check-immediately")){
        qDebug()<<"exec --check-immediately";
    } else if(argnum.contains("--check-upgrade")) {
        qDebug()<<"exec --check-upgrade";
    } else {
        qDebug()<<"exec slient.";
    }

    trayIcon = new trayicon(this);
    bool ignore_or_delay = ignoreOrDelay(argnum.contains("--check-immediately",Qt::CaseInsensitive));
    choice_window = new updatewidget(ignore_or_delay);//更新选择页面
    tip_window = new updateTip();      //静默升级提示

    if (argnum == "") {
        //监听连接源管理器d-bus
        //监听托盘点击事件，3：单击，2：双击，1：右键
        connect(trayIcon,&trayicon::activated, this,&updatehandle::onActivated);
        connect(m_sourceMutual,&source_dbus::ready,this,&updatehandle::getTemplateList);
        m_sourceMutual->connectDbus();
    } else if (argnum.contains("--check-upgrade",Qt::CaseInsensitive)||ignore_or_delay) {
        //连接关闭功能槽函数
        connect(choice_window,&updatewidget::disAgreeBtnSignals,this,&updatehandle::execslots);
        UpdateDectCheck(ignore_or_delay);
    } else {
        qDebug()<<"No update，exit!";
        exit(0);
    }
}

/**
 * @brief updatehandle::tray_Show
 * 启动托盘选择页面
 */
void updatehandle::tray_Show()
{
    if(checktime()){
        if (!m_updateMutual->listenState()) {
            if (m_getsql()) {
                qDebug()<<"m_getsql 的值为true";
                choice_window->update_lab->setText(tr("System update"));
                if (argnum != "update") {
                    choice_window->updateWidgetShow();
                }
            } else {
                qDebug()<<"更新管理器关闭检测更新，托盘程序结束";
                exit(0);
            }
        } else {
            qDebug()<<"更新管理器已运行，程序退出";
            exit(0);
        }
    } else {
        qDebug()<<"配置文件设定时间未到，程序退出";
        exit(0);
    }
}

/**
 * @brief updatehandle::checktime
 * 进行当前时间与配置文件时间比对
 * @return
 * true: 当前时间大于配置文件时间
 * false: 当前时间小于配置文件时间
 */
bool updatehandle::checktime()
{
    timersetting->beginGroup(QString::fromLocal8Bit("Timer"));
    CheckUpgradeTime = timersetting->value("CheckUpgradeTime").toString();
    QDateTime datetime = QDateTime::fromString(CheckUpgradeTime, "yyyy-MM-dd hh:mm:ss");
    //结束组
    timersetting->endGroup();
    QDateTime Nowtime = QDateTime::currentDateTime();  //获取系统现在的时间
    qint64 nSecs = Nowtime.secsTo(datetime); //对比
    qDebug()<<"Get timing time: "<<CheckUpgradeTime;
    qDebug()<<"Current system time: "<<Nowtime;
    qDebug()<<"nSecs"<<nSecs;
    if(nSecs == 0){
        qDebug()<<"The timing configuration file is abnormal，ckeck upgrade ...";
        return true;
    }
    if(nSecs < 0){
        qDebug()<<"Check upgrade starts when the scheduled time is reached.";
        return true;
    }
    return false;
}

/**
 * @brief updatehandle::m_getsql
 * 获取更新管理器是否自动检测的数据
 * @return
 * true:自动检测
 * false:关闭自动检测
 */
bool updatehandle::m_getsql()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("/var/cache/kylin-system-updater/kylin-system-updater.db");
    if (!db.open()) {
           qDebug()<<"open sql error";
           return false;
    }
    QSqlQuery query;
    query.exec("SELECT * FROM display");
    while(query.next()){
         QString tmp=query.value(3).toString();
         if (tmp == "true") {
             return true;
         } else if (tmp == "false") {
             return false;
         }
    }
    qDebug()<<"从数据库未读到正确的标志信息";
    return false;
}

/**
 * @brief updatehandle::onActivated
 * 接收点击时间的槽函数
 * @param reason
 * reason:传入点击事件信息
 */
void updatehandle::onActivated(trayicon::ActivationReason reason)
{
    if(tip_window->isHidden()){
        QPoint pt=cursor().pos();
        tip_window->move(pt.x(),pt.y()-45);
        tip_window->show();
    } else {
        tip_window->hide();
    }
}

/**
 * @brief updatehandle::getTemplateList
 * 获取源管理器文件中的可选更新与强制更新列表
 */
void updatehandle::getTemplateList()
{
    qDebug()<<"Parses the optional update list and the mandatory update list ...";
    QMap<QString,QStringList> updatedata;
    QString str1;
    QString str2;
    QStringList m_crucial;
    QFile file1("/var/lib/kylin-software-properties/template/crucial.list");
    if (!file1.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug()<<"cat not open file crucial.list";
        exit(0);
    }
    while (!file1.atEnd()) {
        QByteArray line = file1.readLine();
        str1 += QString(line);
    }
    str1.replace(QString("\n"), QString(""));
    m_crucial = str1.split(QRegExp("[ ]"));
    m_crucial.removeAll(QString(""));
    qDebug()<<"The crucial lists: "<<m_crucial;

    QStringList m_important;
    QFile file2("/var/lib/kylin-software-properties/template/important.list");
    if (!file2.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug()<<"cat not open file important.list";
        exit(0);
    }
    while (!file2.atEnd()) {
        QByteArray line = file2.readLine();
        str2 += QString(line);
    }
    str2.replace(QString("\n"), QString(""));
    m_important = str2.split(QRegExp("[ ]"));
    m_important.removeAll(QString(""));
    qDebug()<<"The important lists: "<<m_important;

    updatedata.insert("crucial",m_crucial);
    updatedata.insert("important",m_important);

    processData(updatedata);
}

/**
 * @brief updatehandle::processData
 * 处理获取的更新数据
 * @param data
 * 传入可选更新与强制更新列表
 */
void updatehandle::processData(QMap<QString,QStringList> data)
{
    qDebug()<<"Monitor the installation progress and status signal ...";
    //ksu获取进度，状态
    connect(m_updateMutual,&update_dbus::UpdateInstallFinished,this,&updatehandle::InstallFinsih);
    QStringList crucial;
    QMap<QString,QStringList>::iterator it;
    for (it = data.begin();it != data.end();it++) {
        if (it.key() == "crucial") {
            crucial = it.value();
        } else if (it.key() == "important") {
            important = it.value();
        }
    }
    qDebug()<<"Checking silent upgrade list: "<<crucial;
    QStringList       checkCrucialAll;

    checkCrucialAll = m_updateMutual->checkForUpdates(crucial);
    if (!checkCrucialAll.isEmpty()) {
            handleCrucial(checkCrucialAll);
     } else {
        qDebug() << "Silent upgrade list is empty, exit!";
        exit(0);
    }
}

/**
 * @brief updatehandle::handleCrucial
 * 处理静默升级包
 * @param checkCrucialAll
 * 传入包列表
 */
void updatehandle::handleCrucial(QStringList checkCrucialAll)
{
    QStringList crucialPackageSize;
    QStringList crucialPackage;

    if (checkCrucialAll.count()%2 != 0) {
        qDebug()<<"The format of the silent upgrade list is incorrect, exit!";
        exit(0);
    }
    if (argnum != "update") {
        //提取包名与大小信息
        for (int i=0;i<checkCrucialAll.count();i+=2) {
            crucialPackage.append(checkCrucialAll.at(i));
        }

        for (int i=1;i<checkCrucialAll.count();i+=2) {
            crucialPackageSize.append(checkCrucialAll.at(i));
        }
        checkCrucial = crucialPackage;
        qDebug()<<"Checking for inactivity in silent upgrading ...";

        if (powerState) { //判断是否获取到电量
            if (powerNum.toInt()<powerSize) {  //判断电量是否达到警戒值
                qDebug()<<"超出设定电源警戒值，判断包大小是否符合继续下载条件";
                if (packageDownOrNot(crucialPackageSize)) {
                    notify_send(tr("The system is updating silently"));
                    trayIcon->setVisible(true);
                    qDebug()<<"The system is updating silently ...";
                    m_updateMutual->installAndUpgrade(crucialPackage);
                } else {
                    exit(0);
                }
            } else {
                notify_send(tr("The system is updating silently"));
                trayIcon->setVisible(true);
                qDebug()<<"The system is updating silently ...";
                m_updateMutual->installAndUpgrade(crucialPackage);
            }
        } else {
            notify_send(tr("The system is updating silently"));
            trayIcon->setVisible(true);
            qDebug()<<"The system is updating silently ...";
            m_updateMutual->installAndUpgrade(crucialPackage);
        }
    }
}

/**
 * @brief updatehandle::UpdateDectSlot
 * 处理可选升级包
 */
void updatehandle::UpdateDectSlot(bool status,QStringList list,QString errorstring,QString errorcode)
{
    if (!status) {
        qDebug() << "Update Detect failed: " << QString("%1 %2").arg(errorstring).arg(errorcode);
        exit(0);
    } else {
        if (list.length() == 0) {
            qDebug() << "The software on this computer is up to date, exit!";
            exit(0);
        } else {
            choice_window->update_lab->setText(tr("System update"));
            if (argnum != "update") {
                choice_window->updateWidgetShow();
            }
        }
    }
    //完成检测，刷新下次更新标志位
    timersetting->beginGroup(QString::fromLocal8Bit("Timer"));
    timersetting->setValue("GeneratRandomTime",true);
    timersetting->setValue("CheckCompleted",true);
    timersetting->sync();
    timersetting->endGroup();
}

/**
 * @brief updatehandle::UpdateDectCheck
 * 处理可选升级包
 */
void updatehandle::UpdateDectCheck(bool _check_immediately)
{
    //判断是否有系统更新
        if(checktime() || !RandomizedExecute || _check_immediately){
            if (m_getsql() || _check_immediately) {
                qDebug()<<"Detection update has been opened ...";
                connect(m_updateMutual,&update_dbus::UpdateDectFinished,this,&updatehandle::UpdateDectSlot);
                bool ret= m_updateMutual->checkForUpdateDect();
            } else {
                qDebug()<<"Detection updates have been disabled, exit!";
                exit(0);
            }
        } else {
            qDebug()<<"The scheduled time is not reached, exit.";
            exit(0);
        }
}


/**
 * @brief updatehandle::ignoreOrDelays
 * 处理可选升级包
 */
bool updatehandle::ignoreOrDelay(bool mode)
{
    static char flagPath[64] = {0};
    sprintf(flagPath, "/var/cache/kylin-update-manager/ignoreOrDelay");
    bool ignore_or_delay = false;
    QFile file(flagPath);

    //判断是否完成第一步更新
    if (access(flagPath, F_OK) == 0) { // flag文件存在且可写
        qDebug()<<flagPath<<"is found!";

        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            ignore_or_delay = false;
        QByteArray line = file.readLine();
        if(QString(line).contains("2107",Qt::CaseInsensitive)) {
            ignore_or_delay = true;
            qDebug()<<"The first step of the upgrade has been completed, Detection of the second step of the upgrade! [2107]";
//            //需要清空文件
//            file.close();
//            file.open(QIODevice::Truncate);
//            file.close();
//            file.open(QIODevice::WriteOnly);
        } else if (QString(line).contains("2203",Qt::CaseInsensitive)) {
            ignore_or_delay = true;
            qDebug()<<"The first step of the upgrade has been completed, Detection of the second step of the upgrade! [2203]";
        }
    } else {
        qDebug()<<flagPath<<"isn't found!";
        ignore_or_delay = false;
    }
    file.close();
    return ignore_or_delay&&mode;
}

/**
 * @brief updatehandle::packageDownOrNot
 * 判断是否超出设定大小
 * @param arg
 * 传入包列表
 * @return
 * true:符合预定大小
 * false:超出设定大小
 */
bool updatehandle::packageDownOrNot(QStringList arg)
{
    qDebug()<<"开始检测包大小是否超出设定大小";
    for (int i = 0 ;i<arg.count();i++) {
         QString pagesize = arg.at(i);
         if (pagesize.toFloat() > setPackageSize) {
             qDebug()<<"包大小超出设定大小，程序退出";
             return false;
         }
    }
    qDebug()<<"包大小符合预定大小";
    return true;
}

/**
 * @brief updatehandle::register_dbus
 */
void updatehandle::register_dbus()
{
    //建立到session bus的连接
    QDBusConnection connection = QDBusConnection::sessionBus();

    //在session bus上注册名为com.scorpio.test的服务
    if (!connection.registerService("com.scorpio.test")) {
        qDebug() << "error:" << connection.lastError().message();
        exit(-1);
    }
    object = new traydbusservice();

   //注册名为/test/objects的对象，把类Object所有槽函数导出为object的method
   connection.registerObject("/test/objectsd", object,QDBusConnection::ExportAllSlots| QDBusConnection :: ExportAllSignals);

   connect(object,&traydbusservice::quitsignal,[=](){
       emit execSignal();
   });

   connect(object,&traydbusservice::connectSuccessSignal,[=](){
        dbusConnectStatus = true;
        emit object->ready("success");
   });

}

/**
 * @brief updatehandle::d_busStatus
 * 判断下载状态的槽函数
 * @param arg : Appname
 * @param args : aptpercent
 * @param state : state
 */
void updatehandle::d_busStatus(QString arg,int32_t args,QString state)
{
    if (!checkCrucial.isEmpty() && messstate2) {
        messstate2=false;
        trayIcon->setVisible(true);
    }

//    qDebug()<<"下载进度：";
//    qDebug()<<"Appname:"<<arg;
//    qDebug()<<"aptpercent:"<<args;
}

void updatehandle::InstallFinsih(bool state,QStringList pkgname,QString error,QString reason)
{
    bool _check_status;
    timerDownload->stop();
    //apt返回失败时判断是否安装成功
    if (!state)
        _check_status = package_installation_status(checkCrucial);

    if(state || _check_status){
        qDebug() << "Silent upgrade is complete, exit.";
        notify_send(tr("The system has completed an important update, It is recommended that you restart immediately."));
    }else{
        QString errorstate = tr("update error");
        QString errorName;
        errorName = QString("%1 %2").arg(error).arg(errorstate);
        qDebug() << "Silent update failed: "<<errorName;
        notify_send(errorName);
    }
    trayIcon->setVisible(false);
    tip_window->hide();
    exit(0);
}

bool updatehandle::package_installation_status(QStringList pkgs)
{
    QString pkgname;
    QString cmd;
    QProcess process;
    bool status,status_single;
    for (QStringList::iterator it = pkgs.begin(); it != pkgs.end(); ++it) {
        pkgname = *it;
        status_single = false;
        cmd = QString("dpkg -l %1").arg(pkgname);
        process.start(cmd);
        process.waitForFinished();
        QByteArray result_ = process.readAllStandardOutput();
        if (QString::fromLocal8Bit(result_) != "") {
            QStringList rets = QString::fromLocal8Bit(result_).split(QRegExp("[\n]"),QString::SkipEmptyParts);
            for (QStringList::iterator it_ret = rets.begin(); it_ret != rets.end(); ++it_ret) {
                QString ret;
                ret = *it_ret;
                if (ret.contains(pkgname) && ret.startsWith("ii")) {
                    status_single = true;
                    qDebug() << "pkg "<<pkgname<<"，Installation succeeded.";
                }
            }
            if (!status_single) { //install failed
                status = false;
                break;
            }
        } else {
            status = false;
            break;
        }
        if (status_single)
            status = true;
    }
    return status;
}

/**
 * @brief updatehandle::execslots
 * 关闭界面槽函数
 */
void updatehandle::execslots()
{
    //设置推迟检测，更新标志位
    timersetting->beginGroup(QString::fromLocal8Bit("Timer"));
    timersetting->setValue("CheckCompleted",false);
    timersetting->setValue("GeneratRandomTime",false);
    timersetting->sync();
    timersetting->endGroup();
    emit execSignal();
}

/**
 * @brief updatehandle::notify_send
 * 发送消息通知函数
 * @param arg
 * 发送的正文
 */
void updatehandle::notify_send(QString arg)
{
    QDBusInterface iface("org.freedesktop.Notifications",
                         "/org/freedesktop/Notifications",
                         "org.freedesktop.Notifications",
                         QDBusConnection::sessionBus());
    QList<QVariant> args;
    args<<tr("kylin-background-upgrade")
       <<((unsigned int) 0)
      <<QString("kylin-update-manager")
     <<tr("update") //显示的是什么类型的信息//系统升级
    <<arg //显示的具体信息
    <<QStringList()
    <<QVariantMap()
    <<(int)-1;
    iface.callWithArgumentList(QDBus::AutoDetect,"Notify",args);
}

/**
 * @brief updatehandle::downloadTimeout
 * 下载时间超时函数
 */
void updatehandle::downloadTimeout()
{
    qDebug() << "Download the timeout, exit!";
    exit(0);
}


/**
 * @brief updatehandle::_secsTo
 */
int updatehandle::_secsTo(QString time1, QString time2)
{
    QStringList time1Slot,time2Slot;
    // **:**
    if (time1.contains(":")){
        time1Slot = time1.split(":");
    }else{return -1;}
    if (time2.contains(":")){
        time2Slot = time2.split(":");
    }else{return -1;}
    if (time1Slot.at(0) > time2Slot.at(0)) {
        return 1;
    } else if (time1Slot.at(0) == time2Slot.at(0)) {
        if (time1Slot.at(1) > time2Slot.at(1)) {
            return 1;
        } else if (time1Slot.at(1) == time2Slot.at(1)) {
            return 0;
        } else if (time1Slot.at(1) < time2Slot.at(1)) {
            return -1;
        }
    } else if (time1Slot.at(0) < time2Slot.at(0)) {
        return -1;
    }
}

/**
 * @brief updatehandle::readconf
 * 读取配置文件
 */
void updatehandle::readconf()
{
    QMap<QString, QVariant> Timer;
    Timer.insert("CheckCompleted",       bool(false));
    Timer.insert("CheckUpgradeTime",     " ");
    Timer.insert("CheckUpgradeTimeSlot", "14:30:00-16:30:00");
    Timer.insert("GeneratRandomTime",    bool(true));
    Timer.insert("RandomizedExecute",    bool(true));
    QMap<QString, QVariant> Settings;
    Settings.insert("powersize",      int(1));
    Settings.insert("setpackagesize", "999999999");
//    qDebug()<<"Settings "<<QString::number(float(999999999),'f',2);

    confPath = QDir::homePath()+"/.config/kylin-background-upgrade/kylin-background-upgrade-conf.ini";
    QFileInfo fileInfo(confPath);
    if (!fileInfo.exists()) {
        qDebug()<<"The background-upgrade configuration file isn't exists.";
        QMapIterator<QString, QVariant> ir_timer(Timer);
        QMapIterator<QString, QVariant> ir_settings(Settings);
        QSettings *code_conf = new QSettings(confPath,QSettings::IniFormat);
        code_conf->beginGroup(QString::fromLocal8Bit("Timer"));
        while(ir_timer.hasNext()){
            ir_timer.next();
            code_conf->setValue(ir_timer.key(), ir_timer.value());
        }
        code_conf->sync();
        code_conf->endGroup();
        code_conf->beginGroup(QString::fromLocal8Bit("Settings"));
        while(ir_settings.hasNext()){
            ir_settings.next();
            code_conf->setValue(ir_settings.key(), ir_settings.value());
        }
        code_conf->sync();
        code_conf->endGroup();
    }

    timersetting = new QSettings(confPath,QSettings::IniFormat);
    timersetting->beginGroup(QString::fromLocal8Bit("Timer"));
    CheckUpgradeTime = timersetting->value("CheckUpgradeTime").toString();
    CheckUpgradeTimeSlot = timersetting->value("CheckUpgradeTimeSlot").toString();
    RandomizedExecute = timersetting->value("RandomizedExecute").toBool();
    GeneratRandomTime = timersetting->value("GeneratRandomTime").toBool();
    CheckCompleted = timersetting->value("CheckCompleted").toBool();
    qDebug()<<"[ CheckUpgradeTime: "<<CheckUpgradeTime<<", CheckUpgradeTimeSlot: "<<CheckUpgradeTimeSlot \
    <<", RandomizedExecute: "<<RandomizedExecute<<", GeneratRandomTime: "<<GeneratRandomTime<<", CheckUpgradeTime: "<<CheckUpgradeTime\
    <<", CheckCompleted: "<<CheckCompleted<<" ]";
    timersetting->endGroup();

    timersetting->beginGroup(QString::fromLocal8Bit("Settings"));
    powerSize = timersetting->value("powersize").toInt();
    setPackageSize = timersetting->value("setpackagesize").toFloat();
    timersetting->endGroup();

    QStringList timeSlot = CheckUpgradeTimeSlot.split("-");
    qDebug()<<"timeSlot "<<timeSlot;
    //需要刷新检测时间
    if (GeneratRandomTime) {
        QDateTime time = QDateTime::currentDateTime();                     //获取系统现在的时间
        QString current_datetime = time.toString("yyyy-MM-dd hh:mm:ss");                //设置显示格式
        QString current_date= time.toString("yyyy-MM-dd");                //设置显示格式
        QString current_time= time.toString("hh:mm:ss");                //设置显示格式
        QString nowtime;
        QByteArray str2char2 = current_time.toLatin1(); // must
        nowtime = str2char2.data();

        qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
        int a = 0;
        timersetting->beginGroup(QString::fromLocal8Bit("Timer"));
        QString timing =  current_date+' '+timeSlot.at(0);
        QString start_timing = timeSlot.at(0);
        QString end_timing = timeSlot.at(1);
        QStringList start_timingSlot,end_timingSlot;
        QString time0,time1,time2,time3;
        if (start_timing.contains(":")){
            start_timingSlot = start_timing.split(":");
            time0 = start_timingSlot.at(0);
            time1 = start_timingSlot.at(1);
        }
        if (end_timing.contains(":")){
            end_timingSlot = end_timing.split(":");
            time2 = end_timingSlot.at(0);
            time3 = end_timingSlot.at(1);
        }
        int strayParameter = (time2.toInt()*60+time3.toInt())-(time0.toInt()*60+time1.toInt());
        a = 0;
        if (strayParameter<0){
            qDebug() << "Time format error.";
        }else if (strayParameter==0) {
            qDebug() << "strayParameter: "<< strayParameter << ", a "<<a;
        } else {
            a = qrand()%(strayParameter);
            qDebug() << "strayParameter: "<< strayParameter << ", a "<<a;
        }
        QDateTime bojb = QDateTime::fromString(timing,"yyyy-MM-dd hh:mm:ss").addSecs(a*60);
        if (CheckCompleted) {
            bojb = bojb.addDays(1);
            timersetting->setValue("CheckCompleted",false);
        }
        QString timing_time = bojb.toString("yyyy-MM-dd hh:mm:ss").toLatin1().data();

        timersetting->setValue("CheckUpgradeTime",timing_time);
        timersetting->setValue("GeneratRandomTime",false);
        qDebug()<< "CheckUpgradeTime "<<timing_time;
        timersetting->sync();
        timersetting->endGroup();
    }
}

