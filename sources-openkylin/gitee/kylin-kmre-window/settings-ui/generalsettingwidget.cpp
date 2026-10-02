/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "generalsettingwidget.h"
#include "preferences.h"
#include "global.h"
#include "ipitem.h"
#include "netmaskitem.h"
#include "messagebox.h"
#include "utils.h"
#include "processresult.h"

#include <QFrame>
#include <QSpinBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QMessageBox>
#include <QDBusInterface>
#include <QDBusReply>
#include <QCameraInfo>
#include <QTimer>
#include <QLineEdit>
#include <QFile>
#include <QDateTime>
#include <QDir>
#include <QProcess>
#include <QDesktopServices>
#include <QSettings>
#include <QThread>
#include <QUrl>
#include <QGSettings>
#include <unistd.h>
#include <syslog.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace Global;
bool runProcessCommand(const QString &cmd, const QStringList &args, QString &output, QString &err)
{
    QProcess process;
    process.setProgram(cmd);//getenv("SHELL);
    if (args.length() > 0) {
        process.setArguments(args);
    }
    //process.setEnvironment({ "LANG=en_US.UTF-8", "LANGUAGE=en_US" });
    process.start();
    // Wait for process to finish without timeout.
    process.waitForFinished(-1);

    QProcess::ExitStatus status = process.exitStatus();
    const ProcessResult r { process.exitCode(), process.readAllStandardOutput(), process.readAllStandardError() };
//    qInfo().noquote() << Q_FUNC_INFO
//                      << "exitCode =" << r.exitCode
//                      << "output =" << r.standardOutput
//                      << "error =" << r.standardError;
    //return r;

    output = r.standardOutput;
    err = r.standardError;

    process.deleteLater();

    return (status == QProcess::NormalExit && r.exitCode == 0);
}


GeneralSettingWidget::GeneralSettingWidget(QWidget *parent)
    :SettingScrollContent(parent)
    ,m_saveBtn(new QPushButton(tr("save")))
    ,m_ipItem(new IpItem)
    ,m_netmaskItem(new NetMaskItem)
    ,DockerNetworkModeList({{DockerNetworkModeIndex::bridge, "bridge", tr("bridge mode")}})
    ,CurrDockerNetworkModeIndex(DockerNetworkModeIndex::bridge)
{
    m_systemBusInterface = new QDBusInterface("cn.kylinos.Kmre", "/cn/kylinos/Kmre", "cn.kylinos.Kmre", QDBusConnection::systemBus(), this);
    m_pref = new Preferences;
    QFrame *centralWidget = new QFrame;
    QLabel *m_title = new QLabel;
    QFont ft;
    QFontMetrics fm(ft);

    m_title->setText(tr("General Setting"));
    m_title->setStyleSheet("QLabel{font-size:18pt;font-style: normal;}");
    this->m_title->setText(tr("General Setting"));
    this->m_title->setStyleSheet("QLabel{font-size:18pt;font-style: normal;}");
    this->m_title->setVisible(true);
    this->setContentsMargins(30,2,24,40);

    m_cameraLabel = new QLabel(this);
    m_cameraComboBox = new QComboBox(this);
    m_cameraLabel->setFixedWidth(150);
    m_cameraLabel->setText(tr("Camera device:"));
    m_cameraComboBox->setFixedWidth(350);
    m_cameraComboBox->setDuplicatesEnabled(false);
    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addWidget(m_cameraLabel);
    hlayout->addSpacing(30);
    hlayout->addWidget(m_cameraComboBox);
    hlayout->addStretch();

    m_saveBtn->setProperty("isImportant",true);

    AutoStarttitle = new QLabel;
    AutoStarttitle->setText(tr("KMRE Auto Start"));
    AutoStarttitle->setStyleSheet("QLabel{font-size:14pt;font-style: normal;}");
    AutoCheckbox = new QCheckBox;
    AutoCheckbox->setChecked(m_pref->m_KmreAutoStart);
    AutoCheckbox->setText(tr("KMRE starts automatically upon startup"));

    QLabel *m_dockertitle = new QLabel;
    m_dockertitle->setText(tr("Docker network"));
    m_dockertitle->setStyleSheet("QLabel{color:#A1A1A1;}");

    QLabel* DockerNetworkModeLbl = new QLabel;
    DockerNetworkModeLbl->setFixedWidth(150);
    DockerNetworkModeLbl->setText(fm.elidedText(tr("mode"), Qt::ElideRight, 150));
    if (fm.width(tr("mode")) > DockerNetworkModeLbl->width()) {
        DockerNetworkModeLbl->setToolTip(tr("mode"));
    }

    DockerNetworkModeCb = new QComboBox;
    DockerNetworkModeCb->setFixedWidth(220);
    for (const auto& item : DockerNetworkModeList) {
        DockerNetworkModeCb->insertItem(item.mode, item.modeName);
        if (m_pref->DockerNetworkMode == item.name) {
            CurrDockerNetworkModeIndex = item.mode;
        }
    }
    DockerNetworkModeCb->setCurrentIndex(CurrDockerNetworkModeIndex);

    QHBoxLayout *modelayout = new QHBoxLayout;
    modelayout->addWidget(DockerNetworkModeLbl);
    modelayout->addSpacing(30);
    modelayout->addWidget(DockerNetworkModeCb);
    modelayout->addStretch();

    QLabel* DockerNetworkDeviceLbl = new QLabel;
    DockerNetworkDeviceLbl->setFixedWidth(150);
    DockerNetworkDeviceLbl->setText(fm.elidedText(tr("device"), Qt::ElideRight, 150));
    if (fm.width(tr("device")) > DockerNetworkDeviceLbl->width()) {
        DockerNetworkDeviceLbl->setToolTip(tr("device"));
    }

    m_ipItem->setTitle(tr("Network segment"));
    m_netmaskItem->setTitle(tr("Subnet mask"));
    mBridgeModeSaveBtn = new QPushButton(tr("save"));
    mBridgeModeSaveBtn->setFixedSize(96,36);
    defaultdns = m_pref->m_defaultdns;
    QLabel *m_dnstitle = new QLabel;
    m_dnstitle->setText(tr("DNS"));
    m_dnstitle->setFixedWidth(150);
    QComboBox *m_dnsbox = new QComboBox;
    m_dnsbox->addItem(tr("auto"));
    m_dnsbox->addItem(tr("manual"));
//    if (defaultdns) {
//        m_dnsbox->setCurrentIndex(0);
//    }
//    else {
//        m_dnsbox->setCurrentIndex(1);
//    }
    m_dnsbox->setFixedWidth(80);
    m_dnsline = new QLineEdit;
    m_dnsline->setFixedWidth(250);
    m_dnsline->setText(m_pref->m_dns);
    m_dnsline->setVisible(!defaultdns);
    QRegExp regExp("((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])[\\.]){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])([\\,])((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])[\\.]){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])");
    QRegExpValidator *dnsvalidator = new QRegExpValidator(regExp, this);
    m_dnsline->setValidator(dnsvalidator);
    QHBoxLayout *netmasklayout = new QHBoxLayout;
    netmasklayout->setMargin(0);
    netmasklayout->setSpacing(0);
    netmasklayout->addWidget(m_netmaskItem);
    netmasklayout->addSpacing(32);
    netmasklayout->addWidget(mBridgeModeSaveBtn);
    netmasklayout->addStretch();
    QHBoxLayout *dnslayout = new QHBoxLayout;
//    dnslayout->setMargin(0);
//    dnslayout->setSpacing(0);
    dnslayout->addWidget(m_dnstitle);
    dnslayout->addSpacing(30);
    dnslayout->addWidget(m_dnsbox);
    dnslayout->addSpacing(20);
    dnslayout->addWidget(m_dnsline);
    dnslayout->addStretch();

    QVBoxLayout *bridgeModelayout = new QVBoxLayout;
    bridgeModelayout->setMargin(0);
    bridgeModelayout->setSpacing(0);
    bridgeModelayout->addWidget(m_ipItem);
    bridgeModelayout->addSpacing(16);
    bridgeModelayout->addLayout(netmasklayout);
    bridgeModelayout->addStretch();

    mBridgeModeWidget = new QWidget;
    mBridgeModeWidget->setLayout(bridgeModelayout);

    QVBoxLayout *dockerLayout = new QVBoxLayout;
    dockerLayout->setMargin(0);
    dockerLayout->setSpacing(0);
    dockerLayout->addWidget(m_dockertitle);
    dockerLayout->addSpacing(8);
    dockerLayout->addLayout(modelayout);
    dockerLayout->addSpacing(16);
    dockerLayout->addWidget(mBridgeModeWidget);
    dockerLayout->addStretch();

    Shortcuttitle = new QLabel;
    Shortcuttitle->setText(tr("Shortcut"));
    Shortcuttitle->setStyleSheet("QLabel{color:#A1A1A1;}");
    QLabel *shortcut1 = new QLabel;
    shortcut1->setText(tr("screenshot"));
    shortcut1->setFixedWidth(100);
    QLineEdit *shortcutline1 = new QLineEdit;
    shortcutline1->setFixedWidth(350);
    shortcutline1->setText("Ctrl + A");
    shortcutline1->setEnabled(false);
    QHBoxLayout *shortcutlayout1 = new QHBoxLayout;
    shortcutlayout1->addWidget(shortcut1);
    shortcutlayout1->addSpacing(80);
    shortcutlayout1->addWidget(shortcutline1);
    shortcutlayout1->addStretch();
    QVBoxLayout *shortcutlayout = new QVBoxLayout;
    shortcutlayout->setMargin(0);
    shortcutlayout->setSpacing(0);
    shortcutlayout->addWidget(Shortcuttitle);
    shortcutlayout->addSpacing(8);
    shortcutlayout->addLayout(shortcutlayout1);

    Abouttitle = new QLabel;
    Abouttitle->setText(tr("About"));
    Abouttitle->setStyleSheet("QLabel{color:#A1A1A1;}");
    QLabel *aboutversion = new QLabel;
    aboutversion->setFixedWidth(100);
    aboutversion->setText(tr("Version"));
    QLabel *version = new QLabel;
    version->setFixedWidth(200);
    version->setText(tr("Version: ") + getKmreVersion());
    m_version = new QPushButton;
    m_version->setFixedWidth(200);
    QString elided_text3 = fm.elidedText(tr("Version: ")  + getKmreVersion(), Qt::ElideRight, 170);
    m_version->setText(elided_text3);
    if (fm.width(tr("Version: ")  + getKmreVersion()) > m_version->width()) {
        m_version->setToolTip(tr("Version: ")  + getKmreVersion());
    }
//    m_version->setText(tr("Version: ") + getKmreVersion());
    m_version->setStyleSheet("QPushButton{background:palette(Base);}");
    m_version->setProperty("isWindowButton",0x01);
    m_logBtn = new QPushButton;
    m_logBtn->setFixedWidth(120);
    m_logBtn->setText(tr("Log"));
    QHBoxLayout *aboutlayout1 = new QHBoxLayout;
    aboutlayout1->setMargin(0);
    aboutlayout1->setSpacing(0);
    aboutlayout1->addWidget(aboutversion);
    aboutlayout1->addSpacing(80);
    aboutlayout1->addWidget(m_version);
//    aboutlayout1->addWidget(version);
    aboutlayout1->addSpacing(30);
    aboutlayout1->addWidget(m_logBtn);
    aboutlayout1->addStretch();
    QVBoxLayout *aboutlayout = new QVBoxLayout;
    aboutlayout->setMargin(0);
    aboutlayout->setSpacing(0);
    aboutlayout->addWidget(Abouttitle);
    aboutlayout->addSpacing(16);
    aboutlayout->addLayout(aboutlayout1);

    QVBoxLayout *mainlayout = new QVBoxLayout;
    mainlayout->setMargin(0);
    mainlayout->setSpacing(0);
//    mainlayout->addWidget(m_title);
    mainlayout->addWidget(AutoCheckbox);
    mainlayout->addSpacing(16);
    mainlayout->addLayout(hlayout);

    mainlayout->addSpacing(24);
    mainlayout->addLayout(dockerLayout);
//    mainlayout->addSpacing(24);
//    mainlayout->addLayout(shortcutlayout);
    mainlayout->addSpacing(24);
    mainlayout->addLayout(aboutlayout);
//    mainlayout->setContentsMargins(30,2,24,40);

    centralWidget->setLayout(mainlayout);
    this->setContent(centralWidget);

    this->updateCameraDevices();
    connect(AutoCheckbox, &QCheckBox::toggled, this, &GeneralSettingWidget::setKmreAutoStart);
    watcher = new QFileSystemWatcher;
    watcher->addPath("/dev/");
    connect(watcher, &QFileSystemWatcher::directoryChanged, this, [=] (const QString &path) {
        Q_UNUSED(path)
        QTimer::singleShot(1000, this, SLOT(updateCameraDevices()));//1s后去获取，否则无法获取到正确的设备列表
    });
    connect(m_cameraComboBox, &QComboBox::currentTextChanged, this, [=]() {
        QString devname =  m_cameraMap[m_cameraComboBox->currentText()];
        m_pref->m_cameraDeviceName = devname;
        m_pref->updateCameraConfig();
        if (!devname.isEmpty() && !devname.isNull()) {
            QDBusInterface interface("cn.kylinos.Kmre.Manager", "/cn/kylinos/Kmre/Manager", "cn.kylinos.Kmre.Manager", QDBusConnection::sessionBus());
            interface.call("setCameraDevice", devname);
        }
    });
    connect(DockerNetworkModeCb, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GeneralSettingWidget::setDockerNetworkMode);
    //connect(m_ipItem, &IpItem::BtnEnable, this, [=]() {mBridgeModeSaveBtn->setEnabled(true);});
    //connect(m_netmaskItem, &NetMaskItem::BtnEnable, this, [=]() {mBridgeModeSaveBtn->setEnabled(true);});
    //connect(m_dnsbox, QOverload<int>::of(&QComboBox::activated), this, &GeneralSettingWidget::setDockerDns);
    //connect(m_dnsline, &QLineEdit::textChanged, this, [=]() {mBridgeModeSaveBtn->setEnabled(true);});
    connect(mBridgeModeSaveBtn, &QPushButton::clicked, this, [=] () {saveNetSetting();});
    connect(m_version, &QPushButton::clicked, this, [=]() {
        clickNum++;
        if (clickNum == 5) {
            QDBusInterface interface("cn.kylinos.Kmre.Manager", "/cn/kylinos/Kmre/Manager", "cn.kylinos.Kmre.Manager", QDBusConnection::sessionBus());
            QDBusReply<QString> reply = interface.call("getSystemProp", 1, "debug_mode");
            if (reply == "0") {
                emit opendeveloperWidget();
            }
        }
    });
    connect(m_logBtn, &QPushButton::clicked, this, &GeneralSettingWidget::logcollect);

    const QByteArray styleid("org.ukui.style");
    if (QGSettings::isSchemaInstalled(styleid)) {
        QGSettings *stylesettings = new QGSettings(styleid, QByteArray(), this);
        connect(stylesettings, &QGSettings::changed, this, [=](const QString &key) {
            if (key == "systemFontSize" || key == "systemFont") {
                QFont ft;
                QFontMetrics fm(ft);
                QString elided_text3 = fm.elidedText(tr("Version: ")  + getKmreVersion(), Qt::ElideRight, 170);
                m_version->setText(elided_text3);
                if (fm.width(tr("Version: ")  + getKmreVersion()) > m_version->width()) {
                    m_version->setToolTip(tr("Version: ")  + getKmreVersion());
                }
            }
        });
    }
}

void GeneralSettingWidget::setKmreAutoStart(bool checked)
{
    if (checked) {
        m_pref->m_KmreAutoStart = true;
        m_pref->updateKmreAutoStartConfig();
        KylinUI::MessageBox::information(this, tr("Tips"), tr("Modify successfully!"));
    }
    else {
        m_pref->m_KmreAutoStart = false;
        m_pref->updateKmreAutoStartConfig();
        KylinUI::MessageBox::information(this, tr("Tips"), tr("Modify successfully!"));
    }
}

void GeneralSettingWidget::setDockerNetworkMode(int index)
{
    qDebug() << "Set docker network mode to:" << index;
    if (index == DockerNetworkModeIndex::bridge) {
        mBridgeModeWidget->setVisible(true);
    }
}

void GeneralSettingWidget::updateCurrentCameraDevice(const QString &deviceName)
{
    m_pref->m_cameraDeviceName = deviceName;
    m_pref->updateCameraConfig();

    if (!m_pref->m_cameraDeviceName.isEmpty() && !m_pref->m_cameraDeviceName.isNull()) {
        QList<QString> values = m_cameraMap.values();
        for (int i = 0; i < values.count(); i++) {
            if (values.at(i) == m_pref->m_cameraDeviceName) {
                m_cameraComboBox->setCurrentText(m_cameraMap.key(m_pref->m_cameraDeviceName));
                m_cameraComboBox->setToolTip(m_cameraMap.key(m_pref->m_cameraDeviceName));
            }
        }
    }
}

void GeneralSettingWidget::updateCameraDevices()
{
    bool selectedDeviceExists = false;
    m_cameraComboBox->blockSignals(true);
    m_cameraComboBox->clear();
    //m_cameraComboBox->setDuplicatesEnabled(1);
    m_cameraMap.clear();

    QString nocamera = tr("No camera detected");
    m_cameraComboBox->addItem(nocamera);
    m_cameraComboBox->setCurrentText(nocamera);
    m_cameraComboBox->setEnabled(false);

    foreach (const QCameraInfo &cameraInfo, QCameraInfo::availableCameras()) {
        QString nameTag;
        nameTag = QString("%1 (%2)").arg(cameraInfo.description()).arg(cameraInfo.deviceName());

        m_cameraMap[nameTag] = cameraInfo.deviceName();
        m_cameraComboBox->addItem(nameTag);

        if (!m_pref->m_cameraDeviceName.isEmpty() && !m_pref->m_cameraDeviceName.isNull()) {
            if (nameTag.contains(m_pref->m_cameraDeviceName)) {
                m_cameraComboBox->removeItem(0);
                m_cameraComboBox->setEnabled(true);
                m_cameraComboBox->setCurrentText(nameTag);
                m_cameraComboBox->setToolTip(nameTag);
                selectedDeviceExists = true;
            }
        }
    }

    //之前选择的摄像头设备已经不存在了
    if (!selectedDeviceExists && (m_cameraMap.count() > 0)) {
        QString name = m_cameraMap.keys().first();
        QString deviceName = m_cameraMap.value(name);
        m_pref->m_cameraDeviceName = deviceName;
        m_pref->updateCameraConfig();
        m_cameraComboBox->setCurrentText(m_cameraMap.key(m_pref->m_cameraDeviceName));
        m_cameraComboBox->setToolTip(m_cameraMap.key(m_pref->m_cameraDeviceName));
    }

    m_cameraComboBox->blockSignals(false);
}

QString GeneralSettingWidget::getKmreVersion()
{
    QString kmreVersion = "V";
    // 获取kmre 虚包版本信息
    QString cmd = QString("dpkg -s kmre | grep Version | awk '{print $2}'");
    QString version = kmre::utils::execCmd(cmd).trimmed();
    qDebug() << "Get KMRE version:" << version;
    if (!version.isEmpty()) {
        kmreVersion.append(version);
        return kmreVersion;
    }

    return qApp->applicationVersion();
}

void GeneralSettingWidget::logcollect()
{
    m_logBtn->setText(tr("Getting..."));
    m_logBtn->setEnabled(false);

    m_displayInfo.clear();
    QDBusInterface interface("cn.kylinos.Kmre.Manager", "/cn/kylinos/Kmre/Manager", "cn.kylinos.Kmre.Manager", QDBusConnection::sessionBus());
    QDBusPendingReply<QString> reply = interface.asyncCall("getDisplayInformation");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [=] (QDBusPendingCallWatcher *w) {
        if (!w->isError()) {
            QDBusPendingReply<QString> reply = *w;// 无返回值时<>中无需填写类型,如: QDBusPendingReply<> r = *w;
            if (reply.isValid()) {
                m_displayInfo = reply.value();
            }
        }
        w->deleteLater();

        this->getlogcollectstatus();
        this->startCopyLogFils();
    });
}

void GeneralSettingWidget::getlogcollectstatus()
{
    QString info;
    QString output;
    QString err;

    QDateTime current_date_time = QDateTime::currentDateTime();
    QString dateStr = current_date_time.toString("yyyyMMddhhmmss");

    const QString &homePath = QDir::homePath();//QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
    if (!QDir(homePath).exists("KmreLog")) {
        QDir(homePath).mkdir("KmreLog");
    }
    m_logPath = QString("%1/KmreLog/%2").arg(homePath).arg(dateStr);
    QDir dir(m_logPath);
    if(!dir.exists()) {
        bool ok = dir.mkpath(m_logPath);
        Q_UNUSED(ok)
    }

    QString fileName = QString("%1/kmre_service_status_%2.txt").arg(m_logPath).arg(dateStr);

    if (!m_displayInfo.isEmpty() && !m_displayInfo.isNull()) {
        info.append(QString("displayInfo: %1").arg(m_displayInfo));
    }
    info.append("===============================================================");
    info.append("\r\n");

    const QString &confFileName = "/usr/share/kmre/kmre.conf";
    output = kmre::utils::readFileContent(confFileName);
    info.append(confFileName);
    info.append("\n");
    if (!output.isEmpty()) {
        info.append(QString("Info: %1").arg(output));
    }
    if (!err.isEmpty()) {
        info.append(QString("Error: %1").arg(err));
    }
    info.append("===============================================================");
    info.append("\r\n");

    output.clear();
    err.clear();
    const QString &updateFile = "/usr/share/kmre/kmre-update.conf";
    if (QFile::exists(updateFile)) {
        output = kmre::utils::readFileContent(updateFile);
        info.append(updateFile);
        info.append("\n");
        if (!output.isEmpty()) {
            info.append(QString("Info: %1").arg(output));
        }
        if (!err.isEmpty()) {
            info.append(QString("Error: %1").arg(err));
        }
        info.append("===============================================================");
        info.append("\r\n");
    }

    output.clear();
    err.clear();
    const QString &cpuInfoFile = "/proc/cpuinfo";
    output = kmre::utils::readFileContent(cpuInfoFile);
    info.append(cpuInfoFile);
    info.append("\n");
    if (!output.isEmpty()) {
        info.append(QString("Info: %1").arg(output));
    }
    if (!err.isEmpty()) {
        info.append(QString("Error: %1").arg(err));
    }
    info.append("===============================================================");
    info.append("\r\n");

    output.clear();
    err.clear();
    bool ret = runProcessCommand("systemctl", {"--user", "status", "kydroid-appstream.service"}, output, err);
    info.append("kylin-kmre-appstream");
    info.append("\n");
    if (!output.isEmpty()) {
        info.append(QString("Info: %1").arg(output));
    }
    if (!err.isEmpty()) {
        info.append(QString("Error: %1").arg(err));
    }
    info.append("===============================================================");
    info.append("\r\n");

    output.clear();
    err.clear();
    ret = runProcessCommand("systemctl", {"--user", "status", "kylin-kmre-audio.service"}, output, err);
    info.append("kylin-kmre-audio");
    info.append("\n");
    if (!output.isEmpty()) {
        info.append(QString("Info: %1").arg(output));
    }
    if (!err.isEmpty()) {
        info.append(QString("Error: %1").arg(err));
    }
    info.append("===============================================================");
    info.append("\r\n");

    output.clear();
    err.clear();
    ret = runProcessCommand("systemctl", {"status", "docker.service"}, output, err);
    info.append("docker");
    info.append("\n");
    if (!output.isEmpty()) {
        info.append(QString("Info: %1").arg(output));
    }
    if (!err.isEmpty()) {
        info.append(QString("Error: %1").arg(err));
    }
    info.append("===============================================================");
    info.append("\r\n");

    output.clear();
    err.clear();
    ret = runProcessCommand("lspci", QStringList(), output, err);
    info.append("lspci");
    info.append("\n");
    if (!output.isEmpty()) {
        info.append(QString("Info: %1").arg(output));
    }
    if (!err.isEmpty()) {
        info.append(QString("Error: %1").arg(err));
    }
    info.append("===============================================================");
    info.append("\r\n");

    output.clear();
    err.clear();
    ret = runProcessCommand("free", {"-m"}, output, err);
    info.append("memory");
    info.append("\n");
    if (!output.isEmpty()) {
        info.append(QString("Info: %1").arg(output));
    }
    if (!err.isEmpty()) {
        info.append(QString("Error: %1").arg(err));
    }
    info.append("===============================================================");
    info.append("\r\n");

    if (!QFile::exists("/sbin/iptables")) {
        err = tr("/sbin/iptables command does not exist!");
        info.append("iptables");
        info.append("\n");
        if (!err.isEmpty()) {
            info.append(QString("Error: %1").arg(err));
        }
        info.append("===============================================================");
        info.append("\r\n");
        return;
    }

    QFile file;
    file.setFileName(fileName);
    file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
    if (file.isOpen()) {
        file.write(info.toUtf8().constData());
        file.flush();
    }

    if (file.isOpen()) {
        file.close();
    }
}

void GeneralSettingWidget::startCopyLogFils()
{
    QString m_loginUserName = kmre::utils::getUserName();
    QString m_loginUserId = kmre::utils::getUid();
    QDBusInterface interface("cn.kylinos.Kmre.Pref", "/cn/kylinos/Kmre/Pref", "cn.kylinos.Kmre.Pref", QDBusConnection::systemBus());
    QDBusPendingReply<int> reply = interface.asyncCall("copyLogFiles", m_loginUserName, m_loginUserId, m_logPath);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(reply, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, this, [=] (QDBusPendingCallWatcher *w) {
        if (w->isError()) {
            w->deleteLater();
            m_logBtn->setText(tr("failed"));
            m_logBtn->setEnabled(true);
            return;
        }

        QDBusPendingReply<int> reply = *w;// 无返回值时<>中无需填写类型,如: QDBusPendingReply<> r = *w;
        if (reply.isValid()) {
            int ret = reply.value();
            Q_UNUSED(ret);
        }
        w->deleteLater();
        m_logBtn->setText(tr("Log"));
        m_logBtn->setEnabled(true);

        const QString path = QString("%1/KmreLog/").arg(QDir::homePath());
        bool openEnabled = QFileInfo(path).isDir();
        if (openEnabled) {
            QDesktopServices::openUrl(QUrl(QString("file:%1").arg(path), QUrl::TolerantMode));
        }
    });
}

void GeneralSettingWidget::saveNetSetting()
{
    if (DockerNetworkModeCb->currentIndex() == DockerNetworkModeIndex::bridge) {
        setDockerBridgeModeSettings();
    }
}

void GeneralSettingWidget::setDockerBridgeModeSettings()
{
    if (!KylinUI::MessageBox::question(this, tr("Tips"), 
            tr("Are you sure you want to change the docker network settings?"
            "The changes will take effect after kmre restart."))) {
        return;
    }

    qDebug() << "setDockerBridgeModeSettings";

    CurrDockerNetworkModeIndex = DockerNetworkModeIndex::bridge;

    // if (!defaultdns) {
    //     QStringList strlist = m_dnsline->text().split(",");
    //     dns1 = strlist.at(0);
    //     dns2 = strlist.at(1);
    //     if (dns2 == "") {
    //         dns2 = "114.114.114.114";
    //     }
    // }
    // m_systemBusInterface->call("SetDefaultPropOfContainer", kmre::utils::getUserName(), (int32_t)getuid(), "sys.custom.dns1", dns1);
    // m_systemBusInterface->call("SetDefaultPropOfContainer", kmre::utils::getUserName(), (int32_t)getuid(), "sys.custom.dns2", dns2);
    // m_pref->m_defaultdns = defaultdns;
    // m_pref->m_dns = m_dnsline->text();
    // m_pref->updateDNS();
    QString result = QString("%1/%2").arg(m_ipItem->getIP()).arg(m_netmaskItem->getNetmask());
    QDBusInterface interface("cn.kylinos.Kmre.Pref", "/cn/kylinos/Kmre/Pref", "cn.kylinos.Kmre.Pref", QDBusConnection::systemBus());
    QDBusReply<bool> reply = interface.call("setDockerDefaultIpAddress", result);
    if (reply.isValid()) {
        bool ret = reply.value();
        Q_UNUSED(ret);
        if (KylinUI::MessageBox::restartEnv(this)) {
            m_systemBusInterface->call("StopContainer", kmre::utils::getUserName(), (int32_t)getuid());
            emit restartEnv();
        }
    }
}

void GeneralSettingWidget::setDockerDns(int index)
{
    mBridgeModeSaveBtn->setEnabled(true);
    if (index == 0) {
        QFile file("/etc/resolv.conf");
        file.open(QIODevice::ReadOnly | QIODevice::Text);
        QTextStream in(&file);
        QStringList strlist;
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.contains("nameserver") && !line.contains("127")) {
                QStringList list = line.split(" ");
                strlist << list.at(1);
            }
        }
        file.close();
        dns1 = strlist.at(0);
        dns2 = strlist.at(1);
        if (dns2 == "") {
            dns2 = "114.114.114.114";
        }
        defaultdns = true;
        m_dnsline->setVisible(false);
    }
    if (index == 1) {
        dns1 = "";
        dns2 = "";
        defaultdns = false;
        m_dnsline->setVisible(true);
    }
}

GeneralSettingWidget::~GeneralSettingWidget()
{
    {
        if (m_pref) {
            delete m_pref;
            m_pref = nullptr;
        }
    }
}
