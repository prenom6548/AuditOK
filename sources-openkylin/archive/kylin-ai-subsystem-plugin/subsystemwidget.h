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

#ifndef SUBSYSTEMWIDGET_H
#define SUBSYSTEMWIDGET_H

#include <QAction>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QThread>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

#include <atomic>

#include <QApt/Backend>
#include <kysdk/applications/klabel.h>
#include <kysdk/kysdk-system/libkysysinfo.h>
#include <sys/sysinfo.h>

#include "configmanager.h"
#include "subsystempackagemanager.h"

enum class SystemVersion { Version_2403, Version_2503, Version_V11 };

class SubSystemWidget : public QWidget {
    Q_OBJECT

public:
    explicit SubSystemWidget(QWidget *parent = nullptr);

    void continueLoading();

    QWidget *mainWidget();
    QWidget *modelConfigWidget();
    QWidget *computePowerSharingWidget();
    bool runtimeProcessExists();
    bool isRestarted();

private:
    void initSubSystemWidget();
    void initMenu();
    void initMsgBox();
    void initInstallSuccessMsgBox();
    void initUnInstallMsgBox();
    void initServiceUnavailableMsgBox();
    void initGsettings();

    void setLabelTextColor();
    void connectSignals();
    void initQaptBackend();
    bool isInitQaptSuccessful();

    bool isPkgInstalled();
    bool isPkgUninstalling();
    QString getCurPkgVersion();
    QString getLastestVersion();
    void checkPkgVersion();
    void checkRuntime();

    void loadModelConfigWidget();

    void showInstallBtn();
    void showProgressBar();
    void showRebootBtn();
    void showMenuBtn();
    void setMainLabelVesionText(const QString &text);
    void updateMainLabelTextByDetectTime();

    bool isPkgInstallingByInstallProgress();
    bool isPkgUninstallingByUninstallProgress();
    void showNotInstalledState();
    void showInstallingState(const int progress);
    void showInstalledAndRebootState();
    void showInstalledAndNotRebootState();
    void showUninstallingState();

    void setWidgetProperty();
    long getSystemRunningTime();
    void createTmpFile();
    void createUninstallingTmpFile();
    QStringList getUninstallList();

    bool isRestartedInUninstalling();
    SystemVersion curSystemVersion();

    void setKLabelFontColor(kdk::KLabel *klabel, QColor color);

private slots:
    void startProgress();
    void installFailed(QString errorResult, QString errorReason);
    void networkAnomaly();
    void installSuccess();
    void showMenu();
    void detectAndUpdate();
    void uninstalling();
    void uninstallSuccess();
    void uninstallFailed();
    void updateLoadingImage();

    void updateUninstallProgress(int progress);
    void updateInstallProgress(int progress);
    void updateLastDetectUpdateTime(const QString &time);

signals:
    void modelConfigLoaded();
    void uninstallPkgSuccess();
    void loadingPlugin();

private:
    QFrame *topMainFrame = nullptr;
    QVBoxLayout *topMainVBoxLayout = nullptr;

    QLabel *topLabel = nullptr;

    QFrame *topHFrame = nullptr;
    QLabel *mainIconLabel = nullptr;
    QLabel *textLabel1 = nullptr;
    kdk::KLabel *textLabel2 = nullptr;
    kdk::KLabel *mainTextLabel = nullptr;
    QLabel *redMainTextLabel = nullptr;

    QPushButton *installButton = nullptr;
    QProgressBar *mainProgressBar = nullptr;
    QPushButton *rebootButton = nullptr;
    QLabel *loadingLabel = nullptr;
    QToolButton *menuButton = nullptr;
    QPushButton *updateButton = nullptr;

    QTimer *loadingTimer = nullptr;
    int imageIndex = 0;

    QMenu *moreOptionsMenu = nullptr;
    QAction *detectUpdateAction = nullptr;
    QAction *unloadAction = nullptr;

    QMessageBox *installSuccessMsgBox = nullptr;
    QMessageBox *unInstallMsgBox = nullptr;
    QMessageBox *serviceUnavailableMsgBox = nullptr;

    QWidget *configWidget_ = nullptr;
    QWidget *computePowerSharingWidget_ = nullptr;

    QApt::Backend qaptBackend_;
    SubSystemPackageManager subSystemPackageManager_;
    QString pluginDir_ = KYLIN_AI_SUBSYSTEM_PLUGIN_PATH;
    QString packageName_ = "kylin-ai-subsystem";

    std::atomic_bool qaptInited{false};
    std::atomic_bool initSuccess_{true};

    ConfigManager configManager_;

    const QStringList installList_ = {
        "kylin-ai-subsystem",           "kylin-ai-runtime",
        "kylin-ai-engine-plugins",      "kylin-ai-subsystem-modelconfig"};
    const QStringList uninstallList_ = {
        "kylin-ai-subsystem",           "kylin-ai-runtime",
        "kylin-ai-engine-plugins",      "kylin-ai-subsystem-modelconfig",
        "kyai-data-management-service", "kylin-ai-document-qa-service",
        "kylin-ai-document-service",    "kylin-ai-vector-engine",
        "kylin-ai-python-env",          "libtriton-server"};
    const QString tmpFilePath_ = "/tmp/kylin-ai-subsystem-plugin";
    const QString subsystemUninstallingFilePath_ =
        "/tmp/kylin-ai-subsystem-plugin-uninstalling";
};

#endif // SUBSYSTEMWIDGET_H
