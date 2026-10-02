/*
 * Copyright 2025 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "subsystemwidget.h"

#include <QApplication>
#include <QApt/Transaction>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QPalette>
#include <QPluginLoader>

#include <cstdio>
#include <memory>
#include <string>

#include "aisubsystemplugininterface.h"

static const char *AI_SUBSYSTEM_PROCESS = "kylin-ai-runtime";

namespace {

bool hasProcessByName(const std::string &namePart) {
    const std::string command = "ps ax | grep " + namePart + " | grep -v grep";

    // 通过管道获取命令执行结果
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"),
                                                  pclose);
    if (!pipe) {
        return false;
    }

    char buffer[128];
    const bool found = fgets(buffer, sizeof(buffer), pipe.get()) != nullptr;
    return found;
}

}  // namespace

SubSystemWidget::SubSystemWidget(QWidget *parent) : QWidget(parent) {
    initSubSystemWidget();
    connectSignals();
}

void SubSystemWidget::continueLoading() {
    initQaptBackend();
    setWidgetProperty();
    checkRuntime();
}

QWidget *SubSystemWidget::mainWidget() { return topMainFrame; }

QWidget *SubSystemWidget::modelConfigWidget() { return configWidget_; }

QWidget *SubSystemWidget::computePowerSharingWidget() {
    return computePowerSharingWidget_;
}

bool SubSystemWidget::runtimeProcessExists() {
    return hasProcessByName(AI_SUBSYSTEM_PROCESS);
}

bool SubSystemWidget::isRestarted() { return !QFile::exists(tmpFilePath_); }

void SubSystemWidget::initSubSystemWidget() {
    topMainFrame = new QFrame(this);
    topMainFrame->setAccessibleName(
        "kylin-ai-subsystem-plugin_QFrame_subsystemBackFrame");
    topMainFrame->setMinimumWidth(550);
    topMainFrame->setMaximumWidth(16777215);

    topMainVBoxLayout = new QVBoxLayout();
    topMainVBoxLayout->setObjectName(
        "kylin-ai-subsystem-plugin_QVBoxLayout_subsystemBackFrameLayout");
    topMainVBoxLayout->setContentsMargins(0, 0, 0, 0);

    topLabel = new QLabel(topMainFrame);
    topLabel->setAccessibleName("kylin-ai-subsystem-plugin_QLabel_aiSubSystem");
    topLabel->setText(tr("AI SubSystem"));
    topLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    topLabel->setContentsMargins(16, 0, 0, 0);

    topHFrame = new QFrame(topMainFrame);
    topHFrame->setAccessibleName(
        "kylin-ai-subsystem-plugin_QFrame_subsystemFrame");
    topHFrame->setFrameShape(QFrame::Box);
    topHFrame->setFixedHeight(128);

    QHBoxLayout *hLayout = new QHBoxLayout(topHFrame);
    hLayout->setObjectName(
        "kylin-ai-subsystem-plugin_QHBoxLayout_subsystemFrameLayout");
    hLayout->setContentsMargins(16, 0, 16, 0);
    topHFrame->setLayout(hLayout);

    mainIconLabel = new QLabel(topHFrame);
    mainIconLabel->setAccessibleName(
        "kylin-ai-subsystem-plugin_QLabel_subsystemIcon");
    QPixmap iconPixmap(":/image/KART.svg");
    mainIconLabel->setPixmap(iconPixmap);
    hLayout->addWidget(mainIconLabel);

    QVBoxLayout *vLayout = new QVBoxLayout(topHFrame);
    vLayout->setObjectName(
        "kylin-ai-subsystem-plugin_QVBoxLayout_functionDescriptionLayout");
    textLabel1 = new QLabel(tr("Kylin AI SubSystem"), topHFrame);
    textLabel1->setAccessibleName(
        "kylin-ai-subsystem-plugin_QLabel_aiSubSystem");
    QFont textFont1 = textLabel1->font();
    textFont1.setBold(true);
    textLabel1->setFont(textFont1);

    textLabel2 = new kdk::KLabel(topHFrame);
    textLabel2->setObjectName(
        "kylin-ai-subsystem-plugin_KLabel_functionDescription");
    textLabel2->setText(
        tr("Supports stable AI operations by centrally managing models, "
           "runtime libraries, and configuration parameters for efficient AI "
           "deployment."));
    textLabel2->setContentsMargins(0, 0, 16, 0);

    mainTextLabel = new kdk::KLabel(topHFrame);
    mainTextLabel->setText(tr("loading"));
    mainTextLabel->setObjectName("kylin-ai-subsystem-plugin_KLabel_version");
    mainTextLabel->setContentsMargins(0, 0, 16, 0);
    setKLabelFontColor(mainTextLabel, "#808080");

    redMainTextLabel = new QLabel(topHFrame);
    redMainTextLabel->setObjectName("kylin-ai-subsystem-plugin_QLabel_error");
    QPalette mainPalette = mainTextLabel->palette();
    mainPalette.setColor(QPalette::WindowText, Qt::red);
    redMainTextLabel->setPalette(mainPalette);
    redMainTextLabel->hide();

    vLayout->setContentsMargins(0, 30, 0, 30);
    vLayout->setSpacing(0);
    vLayout->addWidget(textLabel1);
    vLayout->addWidget(textLabel2);
    vLayout->addWidget(mainTextLabel);
    vLayout->addWidget(redMainTextLabel);

    hLayout->addLayout(vLayout);
    hLayout->addStretch(1);

    // 重启按钮
    rebootButton = new QPushButton(tr("Reboot Immediately"), topHFrame);
    rebootButton->setAccessibleName(
        "kylin-ai-subsystem-plugin_QPushButton_reboot");
    rebootButton->hide();
    hLayout->addWidget(rebootButton);

    // loading
    loadingLabel = new QLabel(topHFrame);
    loadingLabel->setAccessibleName(
        "kylin-ai-subsystem-plugin_QLabel_loadingImage");
    loadingLabel->setPixmap(
        QIcon::fromTheme("ukui-loading-0-symbolic").pixmap(16, 16));
    loadingLabel->setContentsMargins(16, 0, 16, 0);
    loadingTimer = new QTimer(this);
    loadingTimer->start(100);
    hLayout->addWidget(loadingLabel);

    topMainVBoxLayout->addWidget(topLabel);
    topMainVBoxLayout->addWidget(topHFrame);

    topMainFrame->setLayout(topMainVBoxLayout);
}

void SubSystemWidget::setKLabelFontColor(kdk::KLabel *klabel, QColor color) {
#ifdef SYSTEM_VERSION_2403
    QPalette palette;
    palette.setColor(QPalette::WindowText, color);
    klabel->setPalette(palette);
#else
    klabel->setFontColor(color);
#endif
}

void SubSystemWidget::initMenu() {
    moreOptionsMenu = new QMenu(this);
    moreOptionsMenu->setAccessibleName(
        "kylin-ai-subsystem-plugin_QMenu_PackageOperation");
    detectUpdateAction = moreOptionsMenu->addAction(tr("Detect Update"));
    unloadAction = moreOptionsMenu->addAction(tr("UnInstall"));
}

void SubSystemWidget::initMsgBox() {
    initInstallSuccessMsgBox();
    initUnInstallMsgBox();
    initServiceUnavailableMsgBox();
}

void SubSystemWidget::initInstallSuccessMsgBox() {
    installSuccessMsgBox = new QMessageBox(this);
    installSuccessMsgBox->setAccessibleName(
        "kylin-ai-subsystem-plugin_QMessageBox_installSuccess");
    installSuccessMsgBox->setIconPixmap(
        QIcon::fromTheme("ukui-dialog-success").pixmap(24, 24));
    installSuccessMsgBox->setText(tr("Install success!"));
    installSuccessMsgBox->setInformativeText(
        tr("The AI subsystem has been installed and needs to be restarted "
           "before it can take effect. Do you want to restart it?"));

    installSuccessMsgBox->addButton(tr("Reboot Later"),
                                    QMessageBox::RejectRole);
    QPushButton *rebootImmediately = installSuccessMsgBox->addButton(
        tr("Reboot Immediately"), QMessageBox::AcceptRole);
    installSuccessMsgBox->setDefaultButton(rebootImmediately);

    connect(rebootImmediately, &QPushButton::clicked, this,
            [this]() { subSystemPackageManager_.reboot(); });
}

void SubSystemWidget::initUnInstallMsgBox() {
    unInstallMsgBox = new QMessageBox(this);
    unInstallMsgBox->setAccessibleName(
        "kylin-ai-subsystem-plugin_QMessageBox_uninstallConfirm");
    unInstallMsgBox->setIcon(QMessageBox::Question);

    unInstallMsgBox->setText(
        tr("Are you sure you want to uninstall the AI subsystem?"));
    unInstallMsgBox->setInformativeText(
        tr("After uninstallation, the AI services of "
           "related applications will not be available."));

    unInstallMsgBox->addButton(tr("Cancel"), QMessageBox::RejectRole);
    QPushButton *unInstall =
        unInstallMsgBox->addButton(tr("Uninstall"), QMessageBox::AcceptRole);
    unInstallMsgBox->setDefaultButton(unInstall);

    connect(unInstall, &QPushButton::clicked, this,
            &SubSystemWidget::uninstalling);
}

void SubSystemWidget::initServiceUnavailableMsgBox() {
    serviceUnavailableMsgBox = new QMessageBox();
    serviceUnavailableMsgBox->setIcon(QMessageBox::Warning);

    serviceUnavailableMsgBox->setText(tr("AI service unavailable"));
    serviceUnavailableMsgBox->setInformativeText(
        tr("The AI subsystem was not detected, and the AI service of "
           "the application cannot be used. Please go to System Settings"
           ">AI Module Management page to install."));
    serviceUnavailableMsgBox->addButton("Cancel", QMessageBox::RejectRole);
    QPushButton *goNow = serviceUnavailableMsgBox->addButton(
        tr("Go Now"), QMessageBox::AcceptRole);
    installSuccessMsgBox->setDefaultButton(goNow);
}

void SubSystemWidget::initGsettings() {
    if (!QGSettings::isSchemaInstalled("org.ukui.style")) {
        qWarning() << __func__ << "Failed to find schema org.ukui.style";
        return;
    }
    QGSettings *iconThemeSettings =
        new QGSettings("org.ukui.style", "/org/ukui/style/");
    connect(iconThemeSettings, &QGSettings::changed, [&](const QString &value) {
        if (value == "styleName") {
            setLabelTextColor();
        }
        if (value == "systemFontSize" || value == "systemFont") {
            QFont textFont2 = textLabel2->font();
            textFont2.setPointSize(textLabel1->font().pointSize());
            textLabel2->setFont(textFont2);

            QFont mainTextFont = mainTextLabel->font();
            mainTextFont.setPointSize(textLabel1->font().pointSize() - 2);
            mainTextLabel->setFont(mainTextFont);
        }
    });
}

void SubSystemWidget::setLabelTextColor() {
    QPalette palette;
    QColor placeholderTextColor = palette.color(QPalette::PlaceholderText);

    if (!placeholderTextColor.isValid()) {
        qWarning() << "PlaceholderText color is not valid on this platform.";
        return;
    }
    palette.setColor(QPalette::WindowText, placeholderTextColor);
    textLabel2->setPalette(palette);
    mainTextLabel->setPalette(palette);
}

void SubSystemWidget::loadModelConfigWidget() {
    // 获取插件目录中的所有.so 文件
    QDir dir(pluginDir_);
    QStringList filters;
    filters << "*.so";
    dir.setNameFilters(filters);
    QFileInfoList plugins = dir.entryInfoList();

    foreach (const QFileInfo &fi, plugins) {
        QString filePath = fi.absoluteFilePath();
        QPluginLoader loader(filePath);

        // 加载插件
        QObject *plugin = loader.instance();
        if (!plugin) {
            qWarning() << "Failed to load config plugin:" << filePath
                       << "Error:" << loader.errorString();
            continue;
        }
        qDebug() << "Plugin loaded:" << filePath;

        KylinAiSubSystemPluginInterface *myPlugin =
            qobject_cast<KylinAiSubSystemPluginInterface *>(plugin);
        if (!myPlugin) {
            qWarning() << "Failed to cast plugin to MyPluginInterface";
            continue;
        }
        QWidget *modelConfigWidget = myPlugin->createModelConfigWidget();
        if (modelConfigWidget) {
            configWidget_ = modelConfigWidget;
        }
        QWidget *computePowerSharingWidget =
            myPlugin->createComputePowerSharingWidget();
        if (computePowerSharingWidget) {
            computePowerSharingWidget_ = computePowerSharingWidget;
        }
    }
    emit modelConfigLoaded();
}

void SubSystemWidget::showInstallBtn() {
    installButton->show();
    mainProgressBar->hide();
    rebootButton->hide();
    menuButton->hide();
    updateButton->hide();
}

void SubSystemWidget::showProgressBar() {
    installButton->hide();
    mainProgressBar->show();
    rebootButton->hide();
    menuButton->hide();
    updateButton->hide();
}

void SubSystemWidget::showRebootBtn() {
    installButton->hide();
    mainProgressBar->hide();
    rebootButton->show();
    menuButton->hide();
    updateButton->hide();
}

void SubSystemWidget::showMenuBtn() {
    installButton->hide();
    mainProgressBar->hide();
    rebootButton->hide();
    menuButton->show();
    updateButton->hide();
}

void SubSystemWidget::setMainLabelVesionText(const QString &text) {
    if (!text.isEmpty()) {
        mainTextLabel->setText(tr("Version: %1").arg(text));
    } else {
        mainTextLabel->setText("");
    }
}

void SubSystemWidget::updateMainLabelTextByDetectTime() {
    QString lastDetectTime = configManager_.getLastDetectAndUpdateTime();
    QString curVersion = getCurPkgVersion();
    QString lastestVersion = getLastestVersion();

    if (lastDetectTime.isEmpty()) {
        setMainLabelVesionText(curVersion);
        return;
    }
    if (curVersion == lastestVersion || lastestVersion.isEmpty()) {
        mainTextLabel->setText(tr("Version: %1 "
                                  "Last detection time: %2")
                                   .arg(curVersion)
                                   .arg(lastDetectTime));
    } else {
        mainTextLabel->setText(tr("Detected an upgradable version: %1 "
                                  "Last detection time: %2")
                                   .arg(lastestVersion)
                                   .arg(lastDetectTime));
        updateButton->show();
    }
}

bool SubSystemWidget::isPkgInstallingByInstallProgress() {
    int installProgress = configManager_.getInstallProgress();
    return installProgress > 0 && installProgress < 100;
}

bool SubSystemWidget::isPkgUninstallingByUninstallProgress() {
    int uninstallProgress = configManager_.getUninstallProgress();
    return uninstallProgress > 0 && uninstallProgress < 100 &&
           isPkgUninstalling();
}

void SubSystemWidget::showNotInstalledState() {
    updateInstallProgress(0);
    updateUninstallProgress(0);
    showInstallBtn();
    setMainLabelVesionText(getCurPkgVersion());
}

void SubSystemWidget::showInstallingState(const int progress) {
    showProgressBar();
    mainProgressBar->setValue(progress);
    mainTextLabel->setText(tr("Installing..."));
}

void SubSystemWidget::showInstalledAndRebootState() {
    showMenuBtn();
    updateMainLabelTextByDetectTime();
    updateInstallProgress(100);
    updateUninstallProgress(0);
    emit loadingPlugin();
}

void SubSystemWidget::showInstalledAndNotRebootState() {
    updateInstallProgress(100);
    updateUninstallProgress(0);
    showRebootBtn();
    mainTextLabel->setText(tr("Installed, effective after restarting."));
}

void SubSystemWidget::showUninstallingState() {
    showMenuBtn();
    mainTextLabel->setText(tr("Uninstalling..."));
}

void SubSystemWidget::setWidgetProperty() {
    QString curVersion = getCurPkgVersion();
    setMainLabelVesionText(curVersion);

    if (curSystemVersion() == SystemVersion::Version_2403) {
        if (isPkgInstalled() && !hasProcessByName(AI_SUBSYSTEM_PROCESS)) {
            rebootButton->show();
            mainTextLabel->setText(
                tr("Installed, effective after restarting."));
        }
    }
    loadingLabel->hide();
    loadingTimer->stop();
}

SystemVersion SubSystemWidget::curSystemVersion() {
    QString version = kdk_system_get_version_detaile().os_version;
    qInfo() << "System version: " << version;

    if (version == "2403") {
        return SystemVersion::Version_2403;
    } else if (version == "2503") {
        return SystemVersion::Version_2503;
    } else {
        return SystemVersion::Version_V11;
    }
}

long SubSystemWidget::getSystemRunningTime() {
    struct sysinfo info;
    if (sysinfo(&info) != 0) {
        qWarning() << "Failed to obtain system information!";
        return 0;
    }
    return info.uptime;
}

void SubSystemWidget::createTmpFile() {
    QFile file(tmpFilePath_);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.close();
    } else {
        qWarning() << "unable to create file:" << tmpFilePath_;
    }
}

void SubSystemWidget::createUninstallingTmpFile() {
    QFile file(subsystemUninstallingFilePath_);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.close();
    } else {
        qWarning() << "unable to create file:"
                   << subsystemUninstallingFilePath_;
    }
}

QStringList SubSystemWidget::getUninstallList() {
    QStringList uninstallPkgList = uninstallList_;
    for (int i = 0; i < uninstallPkgList.size(); ++i) {
        QString pkgName = uninstallPkgList[i];
        auto package = qaptBackend_.package(pkgName);
        if (package == nullptr) {
            uninstallPkgList.removeOne(pkgName);
            --i;
        }
    }
    qDebug() << __func__ << uninstallPkgList;
    return uninstallPkgList;
}

bool SubSystemWidget::isRestartedInUninstalling() {
    return !QFile::exists(subsystemUninstallingFilePath_);
};

void SubSystemWidget::connectSignals() {
    connect(loadingTimer, &QTimer::timeout, this,
            &SubSystemWidget::updateLoadingImage);
    connect(rebootButton, &QPushButton::clicked,
            [this]() { subSystemPackageManager_.reboot(); });
}

void SubSystemWidget::initQaptBackend() {
    auto *initThread = QThread::create([this]() {
        int timeout = 0;

        while (timeout < 15) {
            if (qaptBackend_.init()) {
                qaptInited = true;
                break;
            }

            qWarning() << __func__ << "QApt backend initialization failed."
                       << qaptBackend_.initErrorMessage();
            QThread::sleep(1);
            timeout += 1;
        }

        if (!qaptInited) {
            initSuccess_ = false;
            qWarning() << __func__
                       << "QApt backend initialization failed after 15 seconds."
                       << qaptBackend_.initErrorMessage();
        }
    });

    connect(initThread, &QThread::finished, initThread, &QObject::deleteLater);
    connect(qApp, &QCoreApplication::aboutToQuit, initThread, &QThread::quit);

    initThread->start();
}

bool SubSystemWidget::isInitQaptSuccessful() {
    while (!qaptInited) {
        if (!initSuccess_) {
            return false;
        }
        qApp->processEvents();
        QThread::usleep(1);
    }

    return true;
}

bool SubSystemWidget::isPkgInstalled() {
    if (!isInitQaptSuccessful()) {
        return false;
    }

    for (auto pkgName : installList_) {
        auto package = qaptBackend_.package(pkgName);
        if (!package || !package->isInstalled()) {
            qWarning() << __func__ << pkgName
                       << " not exists or not installed!";
            return false;
        }
    }
    return true;
}

bool SubSystemWidget::isPkgUninstalling() {
    if (!isInitQaptSuccessful()) {
        return false;
    }

    for (auto pkgName : uninstallList_) {
        auto package = qaptBackend_.package(pkgName);
        if (package && package->isInstalled()) {
            qDebug() << __func__ << pkgName << package->state();
            return true;
        }
    }
    return false;
}

QString SubSystemWidget::getCurPkgVersion() {
    if (!isInitQaptSuccessful()) {
        return "";
    }

    auto package = qaptBackend_.package(packageName_);
    if (package == nullptr) {
        return QString();
    }
    return package->version();
}

QString SubSystemWidget::getLastestVersion() {
    if (!isInitQaptSuccessful()) {
        return "";
    }

    auto updatePackageList = qaptBackend_.upgradeablePackages();
    for (auto tmpPkg : updatePackageList) {
        if (tmpPkg->name() == packageName_) {
            return tmpPkg->availableVersion();
        }
    }
    return QString();
}

void SubSystemWidget::checkPkgVersion() {
    qaptInited = false;
    initQaptBackend();

    QDateTime currentDate = QDateTime::currentDateTime();
    QString formattedDate = currentDate.toString("yyyy/MM/dd HH:mm");
    QString curVersion = getCurPkgVersion();
    QString lastestVersion = getLastestVersion();

    loadingTimer->stop();
    loadingLabel->hide();

    if (curVersion == lastestVersion || lastestVersion.isEmpty()) {
        mainTextLabel->setText(
            tr("The current version is already the latest version: %1 "
               "Last detection time: %2")
                .arg(curVersion)
                .arg(formattedDate));

        QTimer::singleShot(6000, this, [this, curVersion, formattedDate]() {
            mainTextLabel->setText(tr("Version: %1 "
                                      "Last detection time: %2")
                                       .arg(curVersion)
                                       .arg(formattedDate));
        });
    } else {
        mainTextLabel->setText(tr("Detected an upgradable version: %1 "
                                  "Last detection time: %2")
                                   .arg(lastestVersion)
                                   .arg(formattedDate));
        updateButton->show();
    }

    updateLastDetectUpdateTime(formattedDate);
}

void SubSystemWidget::checkRuntime() {
    if (hasProcessByName(AI_SUBSYSTEM_PROCESS) && isPkgInstalled()) {
        emit loadingPlugin();
        loadModelConfigWidget();
    }
}

void SubSystemWidget::startProgress() {
    createTmpFile();

    redMainTextLabel->hide();
    mainTextLabel->show();

    updateInstallProgress(1);

    showProgressBar();

    mainTextLabel->setText(tr("Installing..."));

    subSystemPackageManager_.installPackage();
}

void SubSystemWidget::installFailed(QString errorResult, QString errorReason) {
    qWarning() << __func__ << packageName_ << errorResult << errorReason;

    if (errorResult.contains("软件包") || errorReason.contains("软件包")) {
        subSystemPackageManager_.fixBrokenDepends();
        subSystemPackageManager_.fixIncompleteInstall();
    }

    redMainTextLabel->show();
    mainTextLabel->hide();

    updateInstallProgress(0);
    updateUninstallProgress(0);

    if (errorReason.contains("网络")) {
        networkAnomaly();
        return;
    }

    installButton->show();
    mainProgressBar->hide();

    redMainTextLabel->setText(
        tr("Installation failed, necessary files are missing."));
}

void SubSystemWidget::networkAnomaly() {
    installButton->show();
    mainProgressBar->hide();

    redMainTextLabel->setText(
        tr("Network abnormality, "
           "please check the network connection and try again."));
}

void SubSystemWidget::installSuccess() {
    updateInstallProgress(100);
    updateUninstallProgress(0);

    installButton->hide();
    mainProgressBar->hide();
    rebootButton->show();
    menuButton->hide();

    installSuccessMsgBox->show();

    mainTextLabel->setText(tr("Installed, effective after restarting."));
}

void SubSystemWidget::showMenu() { moreOptionsMenu->show(); }

void SubSystemWidget::detectAndUpdate() {
    mainTextLabel->setText(tr("Detecting And Update..."));

    loadingLabel->show();
    loadingTimer->start(100);
    checkPkgVersion();
}

void SubSystemWidget::uninstalling() {
    updateUninstallProgress(1);

    mainTextLabel->setText(tr("Uninstalling..."));

    menuButton->setEnabled(false);

    subSystemPackageManager_.unInstallPackage(getUninstallList());
}

void SubSystemWidget::uninstallSuccess() {
    updateInstallProgress(0);
    updateUninstallProgress(100);
    updateLastDetectUpdateTime("");

    emit uninstallPkgSuccess();

    installButton->show();
    updateButton->hide();

    setMainLabelVesionText(getCurPkgVersion());

    menuButton->setEnabled(true);
    menuButton->hide();
}

void SubSystemWidget::uninstallFailed() { showNotInstalledState(); }

void SubSystemWidget::updateLoadingImage() {
    imageIndex += 1;
    int index = imageIndex % 8;
    loadingLabel->setPixmap(
        QIcon::fromTheme(QString("ukui-loading-%1-symbolic").arg(index))
            .pixmap(16, 16));
}

void SubSystemWidget::updateUninstallProgress(int progress) {
    configManager_.setUninstallProgress(progress);
}

void SubSystemWidget::updateInstallProgress(int progress) {
    mainProgressBar->setValue(progress);
    configManager_.setInstallProgress(progress);
}

void SubSystemWidget::updateLastDetectUpdateTime(const QString &time) {
    configManager_.setLastDetectAndUpdateTime(time);
}
