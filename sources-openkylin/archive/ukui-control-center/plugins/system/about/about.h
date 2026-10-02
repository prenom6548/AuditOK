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
#ifndef ABOUT_H
#define ABOUT_H

#include <QObject>
#include <QtPlugin>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusError>
#include <QDBusReply>
#include <QSysInfo>
#include <QLabel>
#include <QStringList>
#include <QSharedPointer>
#include <QGSettings/QGSettings>
#include <QPushButton>
#include <QHBoxLayout>
#include <QThread>
#include <QtConcurrent>
#include <QFuture>
#include <QMouseEvent>

#include "aboutui.h"
#include "HpQRCodeInterface.h"
#include "ukcccommon.h"
using namespace ukcc;
#include "hostnamedialog.h"
#include "privacydialog.h"
#include "statusdialog.h"

#include "shell/interface.h"
#include "lightlabel.h"
#include "kswitchbutton.h"
#include <kysdk/kysdk-system/libkysysinfo.h>
#include <kborderlessbutton.h>
#include <QGSettings>
using namespace kdk;


struct dnsInfo
{
    char *servname;
    pthread_t pid;
};

namespace Ui {
class About;
}

typedef bool (*KylinActivationRsaCheckFunc)(const char* randstr, const char* rsa_sig);
static constexpr const char* LIB_KYLIN_ACTIVATION_PATH = "/usr/lib/libkylin-activation.so";

class About : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    About();
    ~About();

    QString plugini18nName() Q_DECL_OVERRIDE;
    int pluginTypes() Q_DECL_OVERRIDE;
    QWidget *pluginUi() Q_DECL_OVERRIDE;
    const QString name() const Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;

private:
    void initActiveDbus();
    void setConnect();
    void setupSerialComponent();
    void setVersionNumCompenent();
    void setupVersionCompenent();
    void setupDesktopComponent();
    void setupKernelCompenent();
    void setupDiskComponent();
    void setHostNameCompenet();
    void setPrivacyCompent();
    void setupSysInstallComponent();
    void setupUpgradeComponent();
    void securityControl();
    void showExtend(QString dateres);
    int getMonth(QString month);
    void reboot();
    bool eventFilter(QObject *obj, QEvent *event);
    QStringList getUserDefaultLanguage();
    void compareTime(QString date);

    QPixmap loadSvg(const QString &path, int width, int height);
    void preInitialize();
    void logoClickSlot();
    // 主机名重命名：走 clicked（空格/回车/鼠标均可），勿仅监听 MouseButtonPress
    void openHostNameDialog();
    int loadLibrary();
    bool isActivationVersionAtLeast(const QString &requiredVersion);

private:
    AboutUi *mAboutWidget;
    QString pluginName;
    int pluginType;

    QStringList diskInfo;
    QMap<QString, QStringList> disk2; //disk2的结构 QMap<块设备名，总大小和可用大小>

    bool mFirstLoad;

    QSharedPointer<QDBusInterface> activeInterface;
    hp::QRCodeInterface *app; //hp插件

    QString mMemtotal;
    QString mMemAvaliable;

    QString dateRes = "";

    QStringList mNtphostName;
    QStringList mDiskParts;
    QString mTimeText;

    QPixmap mThemePixmap;
    bool activestatus = true;

    QDBusInterface *mUkccDbus = nullptr;
    QDBusInterface *mAboutDBus = nullptr;
    qint64 m_clickTimes[5] = {0};
    int m_currentIndex = 0;
    bool m_isFilled = false;
    QGSettings *m_maintainSettings = nullptr;
    KylinActivationRsaCheckFunc m_rsaCheckFunc = nullptr;
    QLibrary m_library;

Q_SIGNALS:
    void changeTheme();

private Q_SLOTS:
    void diskInfoReadySlot(QString diskInfo);
    void uploadStateChangedSlot(int state);

private slots:
    void activeSlot(int activeSignal);
    void runActiveWindow();
    void changedSlot();
    void openIntelSlot(const QString &desktopFile);
    void keyChangedSlot(const QString &key);
    void setVisibleBySecurity();
    void setEnableBySecurity();
};

#endif // ABOUT_H
