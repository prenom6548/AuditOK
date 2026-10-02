/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include <QWidget>

#include "about.h"
#include "trialdialog.h"

#include <unistd.h>
#include <QFile>
#include <QGridLayout>
#include <QPluginLoader>
#include <QEvent>
#include <QMessageBox>

#ifdef Q_OS_LINUX
#include <sys/sysinfo.h>
#elif defined(Q_OS_FREEBSD)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#include <QProcess>
#include <QFile>
#include <QDebug>
#include <QStorageInfo>
#include <QtMath>
#include <QSvgRenderer>
#include <QElapsedTimer>
#include "kysdk/kysdk-system/libkycpu.h"

using namespace kdk;

#define CLICK_TIME_DIFF 3000
#define PLUGINS_SCHEMA       "org.ukui.control-center.plugins"
#define PLUGINS_PATH         "/org/ukui/control-center/plugins/"
#define PLUGIN_NAME          "plugin-name"
#define SHOW_KEY             "show"

const QString kAboutFile = "/usr/share/applications/kylin-user-guide.desktop";
const QString kHPFile = "/usr/share/applications/hp-document.desktop";
QStringList mIpList;

About::About() : mFirstLoad(true)
{
#ifdef MAVIS
    pluginName = tr("About and Support");
#else
    pluginName = tr("About");
#endif
    pluginType = SYSTEM;
}

About::~About()
{
    if (!mFirstLoad) {

    }
}

QString About::plugini18nName()
{
    return pluginName;
}

int About::pluginTypes()
{
    return pluginType;
}

void About::preInitialize() {
    mAboutDBus = new QDBusInterface("org.ukui.ukcc.session",
                                    "/About",
                                    "org.ukui.ukcc.session.About",
                                    QDBusConnection::sessionBus(), this);
    if (!mAboutDBus->isValid()) {
        qCritical() << "org.ukui.ukcc.session.About DBus error:" << mAboutDBus->lastError();
    }
}

QWidget *About::pluginUi()
{
    if (mFirstLoad) {
        mFirstLoad = false;

        mAboutWidget = new AboutUi;
        // 重命名弹窗改由 clicked 触发（见 setConnect），不再依赖鼠标 Press 过滤
        mAboutWidget->getSequenceContent()->installEventFilter(this);
        preInitialize();
        setConnect();
        setupVersionCompenent();
        setVersionNumCompenent();
        setupDesktopComponent();
        setHostNameCompenet();
        setupKernelCompenent();
        initActiveDbus();
        setupSerialComponent();
        setPrivacyCompent();
        setupDiskComponent();
        securityControl();
        setupSysInstallComponent();
        setupUpgradeComponent();
    }
    return mAboutWidget;
}

const QString About::name() const
{
    return QStringLiteral("About");
}

bool About::isShowOnHomePage() const
{
    return true;
}

QIcon About::icon() const
{
    if (QIcon::hasThemeIcon("preferences-system-details-symbolic")) {
        return QIcon::fromTheme("preferences-system-details-symbolic");
    } else {
        return QIcon();
    }
}

bool About::isEnable() const
{
    return true;
}

/* 初始化DBus对象 */
void About::initActiveDbus()
{
    /*activeInterface = QSharedPointer<QDBusInterface>(
        new QDBusInterface("org.freedesktop.activation",
                           "/org/freedesktop/activation",
                           "org.freedesktop.activation.interface",
                           QDBusConnection::systemBus()));
    if (activeInterface.get()->isValid()) {
        connect(activeInterface.get(), SIGNAL(activation_result(int)), this, SLOT(activeSlot(int)));
    }*/
}

void About::setConnect()
{
    QDBusConnection::sessionBus().connect("org.ukui.ukcc.session",
                                          "/About",
                                          "org.ukui.ukcc.session.About",
                                          "changed",
                                          this,
                                          SLOT(keyChangedSlot(QString)));
    activeInterface = QSharedPointer<QDBusInterface>(
        new QDBusInterface("org.freedesktop.activation",
                           "/org/freedesktop/activation",
                           "org.freedesktop.activation.interface",
                           QDBusConnection::systemBus()));
    if (activeInterface.get()->isValid()) {
        connect(activeInterface.get(), SIGNAL(activation_result(int)), this, SLOT(activeSlot(int)));
    }
    QString pluginName = "MaintainMode";
    QByteArray ba = (QString("%1%2").arg(pluginName).arg("/")).toUtf8();
    char* path = ba.data();
    const QByteArray id(PLUGINS_SCHEMA);
    if (QGSettings::isSchemaInstalled(id)) {
        QString plugin = QString("%1%2%3").arg(PLUGINS_PATH).arg(pluginName).arg("/");
        m_maintainSettings = new QGSettings(id, plugin.toUtf8().data(), this);
    }
    connect(mAboutWidget->getHpBtn(),&QPushButton::clicked,this,[=](){
        openIntelSlot(kHPFile);
    });

    connect(mAboutWidget->getEducateBtn(), &QPushButton::clicked, this, [=]{
        openIntelSlot(kAboutFile);
    });

    connect(mAboutWidget->getActivationBtn(), &QPushButton::clicked, this, &About::runActiveWindow);

    connect(mAboutWidget->getEditHost(), &QPushButton::clicked, this, &About::openHostNameDialog);

    connect(mAboutWidget->getTrialBtn(), &KBorderlessButton::clicked, this, [=]() {
        UkccCommon::buriedSettings(name(), "show trial exemption agreement", QString("clicked"));
        TrialDialog *dialog = new TrialDialog(mAboutWidget);
        dialog->exec();
    });

    connect(mAboutWidget->getAgreeBtn(), &KBorderlessButton::clicked, this, [=]() {
        UkccCommon::buriedSettings(name(), "show user privacy agreement", QString("clicked"));
        QString cmd = "kylin-user-guide -A kylin-os";
        QProcess process(this);
        process.startDetached(cmd);
    });

    connect(mAboutWidget, &AboutUi::onLabelClicked, this, [=]() {
        logoClickSlot();
    });
}

void About::logoClickSlot() {
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    qDebug() << Q_FUNC_INFO << "click logo" << currentTime;

    // 将当前点击时间存入数组
    m_clickTimes[m_currentIndex] = currentTime;
    m_currentIndex = (m_currentIndex + 1) % 5;

    // 如果数组已填满，检查时间差
    if (m_currentIndex == 0 && !m_isFilled) {
        m_isFilled = true;
    }

    if (!m_isFilled) {
        qDebug() << Q_FUNC_INFO << "not fill m_currentIndex" << m_currentIndex;
        return;
    }
    // 已点击5次，计算最早一次点击和当前点击的时间差
    qint64 earliestTime = m_clickTimes[m_currentIndex]; // 当前索引指向的是最早的时间
    qint64 timeDiff = currentTime - earliestTime;
    qDebug() << Q_FUNC_INFO << "earliestTime" << earliestTime << "currentTime" << currentTime << "timeDiff" << timeDiff;
    if (timeDiff < CLICK_TIME_DIFF) {
        qDebug() << Q_FUNC_INFO << "3 second click 5 times";
        m_isFilled = false;
        m_currentIndex = 0;
        for (int i = 0; i < 5; i++) {
            m_clickTimes[i] = 0;
        }
        if (m_maintainSettings) {
            QDBusInterface maintainModeInterface("com.kylin.MaintainMode",
                                                 "/com/kylin/MaintainMode/Object",
                                                 "com.kylin.MaintainMode.interface",
                                                 QDBusConnection::systemBus());
            QDBusReply<bool> ret = maintainModeInterface.call("maintainShowKey");
            bool shouldShowKey = true;
            if (!ret.isValid()) {
                qWarning() << Q_FUNC_INFO << "maintainShowKey dbus call failed" << ret.error().message();
            } else {
                shouldShowKey = ret.value();
                qDebug() << Q_FUNC_INFO << "shouldShowKey" << shouldShowKey;
            }
            if (shouldShowKey) {
                m_maintainSettings->set(SHOW_KEY, true);
            }
            qDebug() << Q_FUNC_INFO << "MaintainMode plugin set show shouldShowKey" << shouldShowKey;
        }
    } else {
        qDebug() << Q_FUNC_INFO << "3 second not click 5 times" << timeDiff;
    }
}

int About::loadLibrary() {
    // 设置库路径
    m_library.setFileName(LIB_KYLIN_ACTIVATION_PATH);

    // 尝试加载库
    bool libraryLoaded = m_library.load();

    if (!libraryLoaded) {
        qDebug() << Q_FUNC_INFO << "Failed to load library:" << LIB_KYLIN_ACTIVATION_PATH
                    << "Error:" << m_library.errorString();
        return -2;
    }

    qDebug() << Q_FUNC_INFO << "Successfully loaded library:" << LIB_KYLIN_ACTIVATION_PATH;

    // 解析函数符号
    m_rsaCheckFunc = reinterpret_cast<KylinActivationRsaCheckFunc>(
        m_library.resolve("kylin_activation_rsa_check")
    );

    if (!m_rsaCheckFunc) {
        qDebug() << Q_FUNC_INFO << "Failed to resolve function: kylin_activation_rsa_check";
        return 1;
    } else {
        qDebug() << Q_FUNC_INFO << "Successfully resolved function: kylin_activation_rsa_check";
        return 0;
    }
}

/* 获取激活信息 */
void About::setupSerialComponent()
{
    if (!activeInterface.get()->isValid()) {
        qDebug() << "Create active Interface Failed When Get active info: " <<
            QDBusConnection::systemBus().lastError();
        return;
    }

    QString serial;
    QDBusReply<QString> serialReply;
    serialReply = activeInterface.get()->call("serial_number");
    if (!serialReply.isValid()) {
        qDebug()<<"serialReply is invalid"<<endl;
    } else {
        serial = serialReply.value();
    }

    bool isActive = false;
    int libStatus = loadLibrary();
    qDebug() << Q_FUNC_INFO << "libStatus" << libStatus << "dateRes" << dateRes;
    if (libStatus == 1) { // 没有rsa函数
        //直接用date的返回值即可
        QDBusMessage dateReply = activeInterface.get()->call("date");
        if (dateReply.type() == QDBusMessage::ReplyMessage) {
            dateRes = dateReply.arguments().at(0).toString();
        }
        isActive = !dateRes.isEmpty();
        qDebug() << Q_FUNC_INFO << "use date" << dateRes << "isActive" << isActive;
    } else if (libStatus == 0) {
        int random = QRandomGenerator::global()->bounded(1000);
        QString randomStr = QString::number(random);
        qDebug() << Q_FUNC_INFO << "random" << random << "randomStr" << randomStr;
        QDBusMessage dateAuthReply = activeInterface.get()->call("date_auth", randomStr);
        QString randomRet;
        if (dateAuthReply.type() == QDBusMessage::ReplyMessage) {
            if (dateAuthReply.arguments().size() == 3) {
                randomRet = dateAuthReply.arguments().at(0).toString();
                dateRes = dateAuthReply.arguments().at(1).toString();
            }
            qDebug() << Q_FUNC_INFO << "date_auth result size" << dateAuthReply.arguments().size() << "dateRes" << dateRes;
        }
        bool rsaRet = m_rsaCheckFunc(randomStr.toStdString().c_str(), randomRet.toStdString().c_str());
        isActive = (rsaRet && !dateRes.isEmpty());
        qDebug() << Q_FUNC_INFO << "use date_auth" << dateRes << "rsaRet" << rsaRet << "isActive" << isActive;
    }

    QDBusMessage trial_dateReply = activeInterface.get()->call("trial_date");
    QString trial_dateRes;
    if (trial_dateReply.type() == QDBusMessage::ReplyMessage) {
        trial_dateRes = trial_dateReply.arguments().at(0).toString();
    }
    QString customerInfo;
    QDBusReply<QString> customerInfoReply;
    customerInfoReply = activeInterface.get()->call("get_license_to_info");
    if (!customerInfoReply.isValid()) {
        mAboutWidget->getCustomerInfoLabel()->hide();
        mAboutWidget->getCustomerInfoValueLabel()->hide();
        qDebug() << Q_FUNC_INFO << "get_license_to_info interface invalid";
    } else {
        customerInfo = customerInfoReply.value();
        if (customerInfo.isEmpty()) {
            mAboutWidget->getCustomerInfoLabel()->hide();
            mAboutWidget->getCustomerInfoValueLabel()->hide();
            qDebug() << Q_FUNC_INFO << "get_license_to_info is empty";
        } else {
            mAboutWidget->getCustomerInfoLabel()->setVisible(true);
            mAboutWidget->getCustomerInfoValueLabel()->setVisible(true);
            mAboutWidget->getCustomerInfoValueLabel()->setText(customerInfo);
            qDebug() << Q_FUNC_INFO << "get customerInfo" << customerInfo;
        }
    }
    mAboutWidget->getSequenceContent()->setText(serial);
    // 未激活时序列号为空：无点击内容，勿进 Tab 链，避免空焦点框
    mAboutWidget->getSequenceContent()->setFocusPolicy(
        serial.isEmpty() ? Qt::NoFocus : Qt::StrongFocus);
    mTimeText = tr("DateRes");
    if (!isActive) {  //未激活
         if (!trial_dateRes.isEmpty()) {  //试用期
            mAboutWidget->getActiveStatus()->setText(tr("Inactivated"));
            mAboutWidget->getActiveStatus()->setFontColor(Qt::red);
            mTimeText = tr("Trial expiration time");
            mAboutWidget->getActivationBtn()->setText(tr("Active"));
        } else {
             mAboutWidget->getActiveStatus()->setText(tr("Inactivated"));
             mAboutWidget->getActiveStatus()->setFontColor(Qt::red);
             mAboutWidget->getActivationBtn()->setText(tr("Active"));
         }
         mAboutWidget->getActivationBtn()->setVisible(true);
         activestatus = false;
    }  else {    //已激活
        mAboutWidget->getActivationBtn()->hide();
        mAboutWidget->getTrialBtn()->hide();
        mAboutWidget->getAndLabel()->hide();
        mAboutWidget->getActiveStatus()->setFontColor(Qt::gray);
        mAboutWidget->getActiveStatus()->setText(tr("Activated"));
        mAboutWidget->getActivationBtn()->setText(tr("Extend"));
        activestatus = true;
    }

    for (QObject* obj : mAboutWidget->children()) {
        if (obj->objectName() != "StatusDialog") {
            continue;
        }
        StatusDialog *statusDialog = static_cast<StatusDialog*>(obj);
        if (nullptr == statusDialog) {
            continue;
        }
        statusDialog->mTimeLabel_1->setText(mTimeText);
        statusDialog->mVersionLabel_2->setText(mAboutWidget->getVersion()->text());
        statusDialog->mStatusLabel_2->setText(mAboutWidget->getActiveStatus()->text());
        statusDialog->mSerialLabel_2->setText(mAboutWidget->getSequenceContent()->text());
        statusDialog->mTimeLabel_2->setText(dateRes);
        if (statusDialog->mTimeLabel_2->text().contains(tr("expired"))) {
            statusDialog->mTimeLabel_2->setFontColor(Qt::red);
        }

        if (!activestatus) {
            statusDialog->mTimeLabel_1->parentWidget()->hide();
            qDebug() << Q_FUNC_INFO << "activestatus is" << activestatus << statusDialog->mTimeLabel_2->text();
        } else {
            statusDialog->mTimeLabel_1->parentWidget()->setVisible(true);
            qDebug() << Q_FUNC_INFO << "activestatus is" << activestatus << statusDialog->mTimeLabel_2->text();
        }
        if (!activestatus && UkccCommon::isCloudVirtualDesktop()) {
            statusDialog->mExtentBtn->hide();
        }
        statusDialog->mExtentBtn->setText(mAboutWidget->getActivationBtn()->text());
        statusDialog->getActivationCode();
        statusDialog->update();
        qDebug() << Q_FUNC_INFO << "set serial" << statusDialog->mSerialLabel_2->text() << statusDialog->mActivationCodeLabel_2->text() << mTimeText;
        break;
    }

}

/* 获取内部版本号 */
void About::setVersionNumCompenent()
{
    QStringList list = mAboutDBus->property("build").toStringList();
    if (list.size() < 2) {
        qDebug() << Q_FUNC_INFO << "get dbus property build size error";
        return;
    }
    QString build = list.at(0);
    QString pversion = list.at(1);
    if (build.isEmpty() || build.contains("null"))
        mAboutWidget->getBuildFrame()->setHidden(true);
    else
        mAboutWidget->getBuild()->setText(build);
    if (pversion.isEmpty() || pversion.contains("null"))
        mAboutWidget->getPatchFrame()->setHidden(true);
    else
        mAboutWidget->getPatchVersion()->setText(pversion);
}

/* 获取logo图片和版本名称 */
void About::setupVersionCompenent()
{
    QString versionID;
    QString version;

    QStringList list = mAboutDBus->property("versionInfo").toStringList();
    if (list.count() < 2) {
        return;
    } else {
        versionID = list.at(0);
        version = list.at(1);
    }

    if (QString(kdk_system_get_systemCategory()).compare("MaxTablet") == 0) {
        version = tr("Kylin Linux Desktop (Touch Screen) V10 (SP1)");
    } else if (QString(kdk_system_get_systemCategory()).compare("Tablet") == 0) {
        version = tr("Kylin Linux Desktop (Tablet) V10 (SP1)");
    }

    if (!version.isEmpty()) {
        mAboutWidget->getVersion()->setText(version);
    } else {
        mAboutWidget->getVersion()->setText(tr("Kylin Linux Desktop V10 (SP1)"));
    }

    if (!versionID.compare(vTen, Qt::CaseInsensitive) ||
        !versionID.compare(vTenEnhance, Qt::CaseInsensitive) ||
        !versionID.compare(vFour, Qt::CaseInsensitive) ||
        !versionID.compare(v11, Qt::CaseInsensitive)) {

        if (mAboutDBus->property("themeMode").toString() == UKUI_DARK) {
            mThemePixmap = loadSvg("://img/plugins/about/logo-dark.svg", 130, 50);
        } else {
            mThemePixmap = loadSvg("://img/plugins/about/logo-light.svg", 130, 50);
        }
    } else {
        mAboutWidget->getActivaFrame()->setVisible(false);
        mAboutWidget->getTrialBtn()->setVisible(false);
        mAboutWidget->getAndLabel()->setVisible(false);
        mThemePixmap = loadSvg("://img/plugins/about/logoukui.svg", 130, 50);
    }
    mAboutWidget->getLogo()->setPixmap(mThemePixmap);
}

/* 获取桌面信息 */
void About::setupDesktopComponent()
{
    QString desktop = mAboutDBus->property("desktop").toString();
    mAboutWidget->getDesktopEnv()->setText(desktop);
    changedSlot();
    QDBusConnection::systemBus().connect(QString(), QString("/org/freedesktop/Accounts/User1000"),
                                         QString("org.freedesktop.Accounts.User"), "Changed",this,
                                         SLOT(changedSlot()));
}

/* 获取CPU信息 */
void About::setupKernelCompenent()
{
    QString memorySize("0GB");
    QString cpuType;
    QString kernal = QSysInfo::kernelType() + " " + QSysInfo::kernelVersion();
    mUkccDbus = new QDBusInterface("com.control.center.qt.systemdbus",
                                   "/",
                                   "com.control.center.interface",
                                   QDBusConnection::systemBus(), this);
    QString availableMemory = mAboutDBus->property("availableMemory").toString();
    qDebug() << Q_FUNC_INFO << "property availableMemory" << availableMemory;
    availableMemory = QString("%1%2%3%4%5").arg("(").arg(availableMemory).arg("GB").arg(tr("Available")).arg(")");
    qDebug() << Q_FUNC_INFO << "format availableMemory" << availableMemory;
    if (mUkccDbus->isValid()) {
       QDBusReply<QString>  result = mUkccDbus->call("getMemory");
       qDebug()<<"memory :"<<result;
       if (result != "0") {
            memorySize.clear();
            memorySize.append(result + "GB" + availableMemory);
            qDebug() << Q_FUNC_INFO << "memorySize" << memorySize << "availableMemory" << availableMemory;
       }
   }
   if (memorySize == "0GB") {
       memorySize = mAboutDBus->property("memory").toString() + availableMemory;
       qDebug() << Q_FUNC_INFO << "memorySize from memory" << memorySize;
   }

    mAboutWidget->getKerner()->setText(kernal);
    mAboutWidget->getMemeory()->setText(memorySize);

    cpuType = kdk_cpu_get_model();
    qDebug() << Q_FUNC_INFO << "cpu info" << cpuType;
    mAboutWidget->getCpuInfo()->setText(cpuType);
}

void About::diskInfoReadySlot(QString diskInfo) {
    qDebug() << Q_FUNC_INFO << "diskInfoReadySlot" << diskInfo;
    if (!diskInfo.isEmpty()) {
        mAboutWidget->getDiskContent()->setText(diskInfo);
        mAboutWidget->getDiskFrame()->setVisible(true);
        qDebug() << Q_FUNC_INFO << "update diskLabel" << diskInfo;
    } else {
        mAboutWidget->getDiskFrame()->setVisible(false);
    }
}

/* 获取硬盘信息 */
void About::setupDiskComponent()
{
    if (mAboutWidget->getDiskFrame()->isHidden()) {
        qDebug() << Q_FUNC_INFO << "diskFrame hidden, skip";
        return;
    }
    QDBusConnection::systemBus().connect("com.control.center.qt.systemdbus",
                                         "/",
                                         "com.control.center.interface",
                                         "diskInfoReady",
                                         this,
                                         SLOT(diskInfoReadySlot(QString)));
    QDBusInterface ifc("com.control.center.qt.systemdbus",
                       "/",
                       "com.control.center.interface",
                       QDBusConnection::systemBus());
    QDBusReply<QString> res = ifc.call("getDiskInfo");
    if (!res.isValid()) {
        qDebug() << Q_FUNC_INFO << "getDiskInfo is invalid";
        mAboutWidget->getDiskFrame()->setVisible(false);
        return;
    }
    QString diskInfo = res.value().trimmed();
    if (diskInfo.isEmpty()) {
        qDebug() << Q_FUNC_INFO << "getDiskInfo is empty";
        mAboutWidget->getDiskFrame()->setVisible(false);
        return;
    }
    mAboutWidget->getDiskContent()->setText(diskInfo);
    qDebug() << Q_FUNC_INFO << "get disk size suc" << diskInfo;
}

void About::setHostNameCompenet()
{
    const QString& hostName = UkccCommon::getHostName();
    if (hostName.isEmpty()) {
        mAboutWidget->getHostName()->setHidden(true);
    } else {
        mAboutWidget->getShowHostName()->setText(hostName);
        mAboutWidget->getHostName()->setText(hostName);
    }
    mAboutWidget->refreshHostNameRenameAccessibleName();
}

void About::uploadStateChangedSlot(int state) {
    qDebug() << Q_FUNC_INFO << "UploadStateChanged state" << state;
    mAboutWidget->getPriBtn()->blockSignals(true);
    mAboutWidget->getPriBtn()->setChecked(state == 0 ? false : true);
    mAboutWidget->getPriBtn()->blockSignals(false);
}

void About::setPrivacyCompent()
{
    QDBusInterface *PriDBus = new QDBusInterface("com.kylin.daq",
                                                 "/com/kylin/daq",
                                                 "com.kylin.daq.interface",
                                                 QDBusConnection::systemBus(), this);
    QDBusConnection::systemBus().connect("com.kylin.daq",
                                         "/com/kylin/daq",
                                         "com.kylin.daq.interface",
                                         "UploadStateChanged",
                                         this,
                                         SLOT(uploadStateChangedSlot(int)));
    if (!PriDBus->isValid()) {
        return;
    }
    QDBusReply<int> reply = PriDBus->call("GetUploadState");
    mAboutWidget->getPriBtn()->blockSignals(true);
    mAboutWidget->getPriBtn()->setChecked(reply == 0 ? false : true);
    mAboutWidget->getPriBtn()->blockSignals(false);

    connect(mAboutWidget->getPriBtn(),&KSwitchButton::stateChanged ,this ,[=](bool status){
        if (status) {
             PriDBus->call("SetUploadState" , (status ? 1 : 0));
             UkccCommon::buriedSettings(name(), "Send optional diagnostic data", QString("settings"), status ? "true": "false");
         } else {
             UkccCommon::buriedSettings(name(), "Send optional diagnostic data", QString("settings"), status ? "true": "false");
             PriDBus->call("SetUploadState" , (status ? 1 : 0));
         }
    } );
}

void About::showExtend(QString dateres)
{
    if (!dateRes.contains(tr("expired"))) {
        dateRes = dateres + QString("(%1)").arg(tr("expired"));
    }
    mAboutWidget->getActivationBtn()->setText(tr("Extend"));
}

int About::getMonth(QString month)
{
    if (month == "Jan") {
        return 1;
    } else if (month == "Feb") {
        return 2;
    } else if (month == "Mar") {
        return 3;
    } else if (month == "Apr") {
        return 4;
    } else if (month == "May") {
        return 5;
    } else if (month == "Jun") {
        return 6;
    } else if (month == "Jul") {
        return 7;
    } else if (month == "Aug") {
        return 8;
    } else if (month == "Sep" || month == "Sept") {
        return 9;
    } else if (month == "Oct") {
        return 10;
    } else if (month == "Nov") {
        return 11;
    } else if (month == "Dec") {
        return 12;
    }else {
        return 0;
    }
}

void About::reboot()
{
    QDBusInterface *rebootDbus = new QDBusInterface("org.gnome.SessionManager",
                                                    "/org/gnome/SessionManager",
                                                    "org.gnome.SessionManager",
                                                    QDBusConnection::sessionBus());

    rebootDbus->call("reboot");
    delete rebootDbus;
    rebootDbus = nullptr;
}

bool About::isActivationVersionAtLeast(const QString &requiredVersion) {
    QProcess versionProcess;
    versionProcess.start("dpkg-query", QStringList() << "--showformat=${Version}" << "--show" << "kylin-activation");
    versionProcess.waitForFinished();

    if (versionProcess.exitCode() != 0) {
        qDebug() << "kylin-activation not find";
        return false;
    }

    QString installedVersion = versionProcess.readAllStandardOutput().trimmed();
    if (installedVersion.isEmpty()) {
        qDebug() << "kylin-activation not find";
        return false;
    }

    qDebug() << "install version" << installedVersion << "required version" << requiredVersion;
    QProcess compareProcess;
    compareProcess.start("dpkg", QStringList() << "--compare-versions" << installedVersion << ">=" << requiredVersion);
    compareProcess.waitForFinished();

    bool satisfied = (compareProcess.exitCode() == 0);
    qDebug() << "version satisfied" << satisfied;

    return satisfied;
}

void About::openHostNameDialog()
{
    if (!mAboutWidget || !mAboutWidget->isDomain()) {
        return;
    }

    QString str = UkccCommon::getHostName();
    HostNameDialog *dialog = new HostNameDialog(mAboutWidget);
    QWidget *widget = qApp->activeWindow(); // 记录 mainwindow，exec 后 activeWindow 可能为空
    dialog->exec();
    if (str != UkccCommon::getHostName()) {
        QMessageBox *mReboot = new QMessageBox(widget);
        mReboot->setIcon(QMessageBox::Warning);
        mReboot->setText(tr("The system needs to be restarted to set the HostName, whether to reboot"));
        mReboot->addButton(tr("Reboot Now"), QMessageBox::AcceptRole);
        mReboot->addButton(tr("Reboot Later"), QMessageBox::RejectRole);
        int ret = mReboot->exec();
        switch (ret) {
        case QMessageBox::AcceptRole:
            sleep(1);
            reboot();
            break;
        }
        mAboutWidget->getHostName()->setText(UkccCommon::getHostName());
        mAboutWidget->getShowHostName()->setText(UkccCommon::getHostName());
        mAboutWidget->refreshHostNameRenameAccessibleName();
        UkccCommon::buriedSettings(name(), "change hostname", QString("settings"), UkccCommon::getHostName());
    }
}

/* 处理窗口缩放时的文本显示 */
bool About::eventFilter(QObject *obj, QEvent *event)
{
    if ( obj == mAboutWidget->getSequenceContent()) {
        if (event->type() == QEvent::MouseButtonPress){
            if (isActivationVersionAtLeast("1.3.11.2-0k5.0")) {
                qDebug() << "kylin-activation version >= 1.3.11.2-0k5.0";
                runActiveWindow();
                return true;
            }
            qDebug() << "kylin-activation version not satisfied >= 1.3.11.2-0k5.0";
            QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton  && !mAboutWidget->getSequenceContent()->text().isEmpty()
                    && mAboutWidget->getSequenceContent()->isEnabled()){
                UkccCommon::buriedSettings(name(), "show activation info", QString("clicked"));
                if (!dateRes.isEmpty()) {
                    activestatus = true;
                    compareTime(dateRes);
                }
                StatusDialog *dialog = new StatusDialog(mAboutWidget);
                dialog->setObjectName("StatusDialog");
                dialog->mLogoLabel->setPixmap(mThemePixmap);
                connect(this,&About::changeTheme,[=](){
                    dialog->mLogoLabel->setPixmap(mThemePixmap);
                });
                dialog->mVersionLabel_1->setText(tr("Version"));
                dialog->mVersionLabel_2->setText(mAboutWidget->getVersion()->text());
                dialog->mStatusLabel_1->setText(tr("Status"));
                dialog->mStatusLabel_2->setText(mAboutWidget->getActiveStatus()->text());
                dialog->mSerialLabel_1->setText(tr("Serial"));
                dialog->mSerialLabel_2->setText(mAboutWidget->getSequenceContent()->text());
                dialog->mTimeLabel_1->setText(mTimeText);
                dialog->mTimeLabel_2->setText(dateRes);
                if (dialog->mTimeLabel_2->text().contains(tr("expired"))) {
                    dialog->mTimeLabel_2->setFontColor(Qt::red);
                }

                if (!activestatus && UkccCommon::isCloudVirtualDesktop()) {
                    dialog->mTimeLabel_1->parentWidget()->hide();
                }
                if (!activestatus) {
                    dialog->mTimeLabel_1->parentWidget()->hide();
                }
                qDebug() << Q_FUNC_INFO << "activestatus" << activestatus << "dateRes" << dateRes;
                dialog->mExtentBtn->setText(mAboutWidget->getActivationBtn()->text());
                connect(dialog->mExtentBtn, &QPushButton::clicked, this, &About::runActiveWindow);
                dialog->exec();
                return true;
            }
        }
    }
    return QObject::eventFilter(obj, event);
}


QStringList About::getUserDefaultLanguage()
{
    QString formats;
    QString language;
    QStringList result;

    unsigned int uid = getuid();
    QString objpath = "/org/freedesktop/Accounts/User"+QString::number(uid);

    QDBusInterface iproperty("org.freedesktop.Accounts",
                             objpath,
                             "org.freedesktop.DBus.Properties",
                             QDBusConnection::systemBus());
    QDBusReply<QMap<QString, QVariant> > reply = iproperty.call("GetAll", "org.freedesktop.Accounts.User");
    if (reply.isValid()) {
        QMap<QString, QVariant> propertyMap;
        propertyMap = reply.value();
        if (propertyMap.keys().contains("FormatsLocale")) {
            formats = propertyMap.find("FormatsLocale").value().toString();
        }
        if(language.isEmpty() && propertyMap.keys().contains("Language")) {
            language = propertyMap.find("Language").value().toString();
        }
    }
    qDebug()<<formats<<"---"<<language;
    result.append(formats);
    result.append(language);
    return result;
}

void About::setupSysInstallComponent()
{
    if (mAboutWidget->getInstallDateFrame()->isHidden())
        return;
    QProcess process;
    process.start("lsblk");
    process.waitForFinished();
    QString diskSize = process.readAllStandardOutput();
    foreach (QString diskResult, diskSize.split("\n")) {
        if (diskResult == NULL)
            continue;
        diskResult.replace(QRegExp("[\\s]+"), " ");
        diskInfo = diskResult.split(" ");

        if (diskInfo.count() >= 6) {
            if (diskInfo.at(5).contains("part"))
                mDiskParts.append(diskInfo.at(0));
        }
        if (diskInfo.count() >= 6 && (!diskInfo.at(0).contains("fd")) &&
               (diskInfo.at(2) !="1")) { //过滤掉可移动硬盘
            if (diskInfo.at(5) == "disk") {
                QStringList totalSize;
                totalSize.append(diskInfo.at(3));
                disk2.insert(diskInfo.at(0),totalSize); //硬盘信息分类存储，用以兼容多硬盘电脑
            }

        }
    }
    qDebug() << Q_FUNC_INFO << "mDiskParts" << mDiskParts;
    for (QString part : mDiskParts) {
        part = part.mid(2);
        if (mUkccDbus->isValid()) {
            QDBusReply<QString>  result = mUkccDbus->call("getSysInstallTime", part);
            qDebug() << Q_FUNC_INFO << "part" << part << "result" << result.isValid() << "value" << result.value();
            if (result != "") {
                qDebug() << Q_FUNC_INFO << "part" << part << "result" << QString(result);
                QStringList list = QString(result).split(" ");
                if (list.count() >= 5) {
                    QString date = list.at(2);
                    date = date.toInt() < 10 ? QString("0%1").arg(date) : date;
                    mAboutWidget->getInstallDate()->setText(QString("%1-%2-%3").arg(list.at(4)).arg(QString::number(getMonth(list.at(1)))).arg(date));
                    qDebug() << Q_FUNC_INFO << "getInstallDate set Text" << date;
                    return;
                }
            }
        }
    }
    mAboutWidget->getInstallDateFrame()->hide();
}

void About::setupUpgradeComponent()
{
    if (mAboutWidget->getUpgradeDateFrame()->isHidden())
        return;

    QString upgradeDate = mAboutDBus->property("upgradeDate").toString();
    if (upgradeDate.isEmpty())
        mAboutWidget->getUpgradeDateFrame()->hide();
    else
        mAboutWidget->getUpgradeDate()->setText(upgradeDate);
}

void About::securityControl()
{
    setVisibleBySecurity();
    QDBusConnection::sessionBus().connect(QString(), QString("/"), "org.ukui.ukcc.session.interface", "configChanged", this, SLOT(setVisibleBySecurity()));
    setEnableBySecurity();
    QDBusConnection::sessionBus().connect(QString(), QString("/"), "org.ukui.ukcc.session.interface", "configChanged", this, SLOT(setEnableBySecurity()));
}

void About::setVisibleBySecurity()
{
    mAboutWidget->getInstallDateFrame()->hide();
    mAboutWidget->getUpgradeDateFrame()->hide();

    QString platformName = QString(QLatin1String(kdk_system_get_appScene()));
    if (platformName.contains("ICBC", Qt::CaseInsensitive)) {
        mAboutWidget->getEditHost()->setVisible(false);
        mAboutWidget->getHostNameLabel1()->setEnabled(false);
    }

    // 安全管控 安装时间及系统更新时间的显示与隐藏
    QVariantMap ModuleMap = UkccCommon::getModuleHideStatus();
    QString moduleSettings = ModuleMap.value(name().toLower() + "Settings").toString();
    QStringList setItems = moduleSettings.split(",");

    foreach (QString setItem, setItems) {
        QStringList item = setItem.split(":");
        qDebug() << "set item Name: " << item.at(0);
        if (item.at(0) == "installedDateFrame") {
            mAboutWidget->getInstallDateFrame()->setVisible(item.at(1) == "true");
        }
        if (item.at(0) == "upgradeDateFrame") {
            mAboutWidget->getUpgradeDateFrame()->setVisible(item.at(1) == "true");
        }
        if (item.at(0) == "mHostNameLabel") {
            mAboutWidget->getEditHost()->setVisible(item.at(1) == "true");
        }
    }
}

void About::setEnableBySecurity() {
    QVariantMap ModuleMap = UkccCommon::getModuleHideStatus();
    QString moduleSettings = ModuleMap.value("aboutEnable").toString();
    QStringList setItems = moduleSettings.split(",");
    qDebug() << Q_FUNC_INFO << moduleSettings;
    foreach (QString setItem, setItems) {
        QStringList item = setItem.split(":");
        if (item.size() != 2) {
            continue;
        }
        qDebug() << "set item Name: " << item.at(0);
        if (item.at(0) == "mHostNameLabel") {
            mAboutWidget->getEditHost()->setEnabled(item.at(1) == "true");
        }
    }
}

void About::compareTime(QString date)
{
    QElapsedTimer timer;
    timer.start();
    QString s1 = mAboutDBus->property("netDate").toString();
    qDebug() << Q_FUNC_INFO << " ntpTime:" << s1 << timer.elapsed();
    QStringList list_1;
    QStringList list_2 = date.split("-");
    int year;
    int mouth;
    int day;
    if (s1.isEmpty()) {    //未连接上网络, 和系统时间作对比
        QString currenttime =  QDateTime::currentDateTime().toString("yyyy-MM-dd");
        qDebug() << currenttime;
         list_1 = currenttime.split("-");
         year = QString(list_1.at(0)).toInt();
         mouth = QString(list_1.at(1)).toInt();
         day = QString(list_1.at(2)).toInt();
    } else {    //获取到网络时间
        s1.remove(QChar('\n'), Qt::CaseInsensitive);
        s1.replace(QRegExp("[\\s]+"), " ");   //把所有的多余的空格转为一个空格
        qDebug()<<"网络时间 : "<<s1;
        list_1 = s1.split(" ");
        year = QString(list_1.at(list_1.count() -1)).toInt();
        mouth = getMonth(list_1.at(1));
        day = QString(list_1.at(2)).toInt();
    }
    if (QString(list_2.at(0)).toInt() > year) { //未到服务到期时间
    } else if (QString(list_2.at(0)).toInt() == year) {
        if (QString(list_2.at(1)).toInt() > mouth) {
        } else if (QString(list_2.at(1)).toInt() == mouth) {
            if (QString(list_2.at(2)).toInt() > day) {
            } else {   // 已过服务到期时间
                showExtend(date);
            }
        } else {
            showExtend(date);
        }
    } else {
        showExtend(date);
    }
}

void About::activeSlot(int activeSignal)
{
    if (!activeSignal) {
        setupSerialComponent();
    }
}

/* 打开激活窗口 */
void About::runActiveWindow()
{
    UkccCommon::buriedSettings(name(), "Activate the system or extend the service", QString("settings"));
    mAboutDBus->call("openActivation");
}

/* 获取用户昵称 */
void About::changedSlot()
{
    QString userName;
    qlonglong uid = getuid();
    QDBusInterface user("org.freedesktop.Accounts",
                        "/org/freedesktop/Accounts",
                        "org.freedesktop.Accounts",
                        QDBusConnection::systemBus());

    if (user.isValid()) {
        QDBusMessage result = user.call("FindUserById", uid);
        QString userpath = result.arguments().value(0).value<QDBusObjectPath>().path();
        QDBusInterface *userInterface = new QDBusInterface ("org.freedesktop.Accounts",
                                                            userpath,
                                                            "org.freedesktop.Accounts.User",
                                                            QDBusConnection::systemBus());
        userName = userInterface->property("RealName").value<QString>();

    }

    if (userName.isEmpty()) {
        mAboutWidget->getUserNameFrame()->setVisible(false);
        qDebug() << Q_FUNC_INFO << "userName is empty, hidden";
    } else {
        if (mAboutWidget && mAboutWidget->getUserName()) {
            mAboutWidget->getUserName()->setText(userName);
        }
    }
}

void About::openIntelSlot(const QString &desktopFile)
{
    QDBusInterface ifc("com.kylin.AppManager",
                       "/com/kylin/AppManager",
                       "com.kylin.AppManager",
                       QDBusConnection::sessionBus());
    ifc.call("LaunchApp", desktopFile);
}

void About::keyChangedSlot(const QString &key)
{
    if (key == "styleName") {
        if (mAboutDBus->property("themeMode").toString() == UKUI_DARK) {
            mThemePixmap = loadSvg("://img/plugins/about/logo-dark.svg", 130, 50);
        } else {
            mThemePixmap = loadSvg("://img/plugins/about/logo-light.svg", 130, 50);
        }
        mAboutWidget->getLogo()->setPixmap(mThemePixmap);
        emit changeTheme();
    }
}

QPixmap About::loadSvg(const QString &path, int width, int height) {
    const auto ratio = qApp->devicePixelRatio();
    if (ratio >= 2) {
        width += width;
        height += height;
    } else {
        height *= ratio;
        width *= ratio;
    }
    QPixmap pixmap(width, height);
    QSvgRenderer renderer(path);
    pixmap.fill(Qt::transparent);

    QPainter painter;
    painter.begin(&pixmap);
    renderer.render(&painter);
    painter.end();

    pixmap.setDevicePixelRatio(ratio);
    return pixmap;
}
