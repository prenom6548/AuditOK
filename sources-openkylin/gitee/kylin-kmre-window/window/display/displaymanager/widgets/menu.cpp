/*
 * Copyright (C) 2013 ~ 2020 KylinSoft Ltd.
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

#include "menu.h"
#include "action.h"
#include "utils.h"
#include "preferences.h"
#include "kmreenv.h"
#include "recordscreenwidget.h"
#include "kmrewindow.h"
#include "appsettings.h"
#include "screensharing.h"
#include "messagebox.h"
#include "dbusclient.h"

#include <QDebug>
#include <QDir>
#include <QTimer>

#include <sys/syslog.h>

using namespace kmre;

Menu::Menu(QWidget *parent) : QMenu(parent)
{
    this->setMouseTracking(true);
    this->setMinimumWidth(180);
    //    this->setStyleSheet("QMenu{background-color:rgb(255,255,255);border-radius:8px;padding:4px 0px;margin:4px
    //    4px;}"
    //                        "QMenu::item {color: rgb(48,49,51);background-color: transparent;border-radius: 4px;
    //                        padding:6px 1px;padding-left: 25px;margin:0px 3px;}" "QMenu::separator {height:
    //                        1px;background: rgb(100,100,100);margin-left: 10px;margin-right: 10px;}"
    //                        "QMenu::item:!enabled{color: rgb(155, 155, 155);}"
    //                        "QMenu::item:selected {background-color: rgb(247,247,247);}");
}

TitleMenu::TitleMenu(const QString &pkgName, QWidget *parent)
    : Menu(parent)
    , mPkgName(pkgName)
{
    createOneLevelAction();
    createAppConfigMenu();
    updateUi();
}

TitleMenu::~TitleMenu() 
{
    
}

void TitleMenu::updateActions(QString name, Action *action, QMenu *menu)
{
    QList<QAction *> allActions = menu->actions();
    if (action) {
        QString pkgName = kmre::utils::getRealPkgName(mPkgName);
        if (KmreConfig::Feature::getInstance()->isEnabled(pkgName, name) && (!allActions.contains(action))) {
            menu->addAction(action);
        } else if ((!KmreConfig::Feature::getInstance()->isEnabled(pkgName, name)) && (allActions.contains(action))) {
            menu->removeAction(action);
        }
    }
}

void TitleMenu::createOneLevelAction()
{
    m_prefAction = new Action(tr("Preference"));
    connect(m_prefAction, &Action::triggered, [this]() {
        emit this->sigPref();
    });

    m_joystickAction = new Action(tr("Joystick"));
    connect(m_joystickAction, &Action::triggered, [this]() {
        emit this->sigJoystick();
    });

    // m_screenshotAction = new Action(tr("Screenshot Shared"));
    // connect(m_screenshotAction, &Action::triggered, [this]() {
    //     emit this->sigScreenShot();
    // });

    m_recordScreenAction = new Action(tr("Screen recording Shared"));
    connect(m_recordScreenAction, &Action::triggered, [this]() {
        emit this->sigRecordScreen();
    });

    m_keyboardAction = new Action(tr("Pop up virtual keyboard"));
    connect(m_keyboardAction, &Action::triggered, [this]() {
        emit this->sigKeyboard();
    });
    //m_keyboardAction->setEnabled(true);

    m_openStorageAction = new Action(tr("Open Storage"));
    connect(m_openStorageAction, &Action::triggered, [this]() {
        emit this->sigOpenStorage(initLink());
    });

    m_manulAction = new Action(tr("Help"));
    connect(m_manulAction, &Action::triggered, [this]() {
        emit this->sigManual();
    });

    // m_closeEnvAciton = new Action(tr("Close Kmre"));
    // connect(m_closeEnvAciton, &Action::triggered, [this]() {
    //     emit this->sigCloseEnv();
    // });

    // m_lockScreenAction = new Action(tr("Lock screen"));
    // connect(m_lockScreenAction, &Action::triggered, [this]() {
    //     emit this->sigLockScreen();
    // });

    m_aboutAction = new Action(tr("About"));
    connect(m_aboutAction, &Action::triggered, [this]() {
        emit this->sigAbout();
    });

    m_quitAction = new Action(tr("Quit"));
    connect(m_quitAction, &Action::triggered, [this]() {
        emit this->sigQuit();
    });

    m_shakeAction = new Action(tr("Shake"));
    connect(m_shakeAction, &Action::triggered, [this]() {
        emit this->sigShake();
    });

    // m_gpsAction = new Action(tr("virtualgps"));
    // connect(m_gpsAction, &Action::triggered, [this]() {
    //     emit this->sigGps();
    // });

    m_sensorAction = new Action(tr("gravity_sensor"));
    connect(m_sensorAction, &Action::triggered, [this]() {
        emit this->sigSensor();
    });

    // m_rotationAction = new Action(tr("Rotate"));
    // connect(m_rotationAction, &Action::triggered, [this]() {
    //     emit this->sigRotate();
    // });

    // m_scrollAction = new Action(tr("Scroll"));
    // connect(m_scrollAction, &Action::triggered, [this]() {
    //     emit this->sigscroll();
    // });
}

void TitleMenu::createAppConfigMenu()
{
    m_appConfigMenu = new Menu();
    m_appConfigMenu->setTitle(tr("AppConfig"));
    AppSettings &appSettings = AppSettings::getInstance();
    AppSettings::AppConfig appConfig = appSettings.getAppConfig(mPkgName);

    // 平行界面persist.kmre.multiwindow
    if (appSettings.isMultiperEnabled() && appSettings.isAppSupportMultiper(mPkgName)) {
        bool enabled = appSettings.isAppMultiperEnabled(mPkgName);
        mMultipleAction = new Action(tr("Multiple"), enabled);
        connect(mMultipleAction, &Action::triggered, [this] {
            bool checked = mMultipleAction->isChecked();
            mMultipleAction->update(!checked);
            emit sigMultipleEnabled(!checked);
        });
        m_appConfigMenu->addAction(mMultipleAction);
    }

    // 全屏启动
    mBootAction = new Action(tr("FullScreenBoot"), appConfig.bootFullScreen);
    connect(mBootAction, &Action::triggered, [=] {
        bool checked = mBootAction->isChecked();
        if (AppSettings::getInstance().setAppBootFullScreen(mPkgName, !checked)) {
            mBootAction->update(!checked);
        }
    });
    m_appConfigMenu->addAction(mBootAction);
}

void TitleMenu::updateAppConfigMenuUi()
{
    // 平行界面
    if (mMultipleAction) {
        mMultipleAction->update(AppSettings::getInstance().isAppMultiperEnabled(mPkgName));
    }

    AppSettings::AppConfig appConfig = AppSettings::getInstance().getAppConfig(mPkgName);

    // 全屏启动
    if (mBootAction) {
        mBootAction->update(appConfig.bootFullScreen);
    }
}

void TitleMenu::updateUi()
{
    this->clear();
    updateActions("settings", m_prefAction, this);

    addMenu(m_appConfigMenu);
    updateAppConfigMenuUi();

    updateActions("shack_screen", m_shakeAction, this);
    if (ScreenSharing::getInstance()->isSharingSupported()) {
        updateActions("screen_record_sharing", m_recordScreenAction, this);
    }

    if (!AppSettings::getInstance().isAppSupportDDS(mPkgName)) {
        updateActions("joy_stick", m_joystickAction, this);
    }
    updateActions("virtual_keyboard", m_keyboardAction, this);
    updateActions("gravity_sensor", m_sensorAction, this);
    updateActions("mobile_data_folder", m_openStorageAction, this);
    updateActions("help", m_manulAction, this);
    updateActions("quit", m_quitAction, this);
}

QString TitleMenu::initLink()
{
    AppSettings::getInstance().initAndroidDataPath();
    
    QString destPath = KmreEnv::GetAndroidDataPath() + "/KmreData";
    QDir dir;
    if (!dir.mkpath(destPath)) {
        syslog(LOG_ERR, "[%s] Make path '%s' failed!", __func__, destPath.toStdString().c_str());
    }

    if (AppSettings::getInstance().isAppSupportKmreData(mPkgName)) {
        KmreWindow* mainWindow = (KmreWindow*)parentWidget();
        QString appDataPath = destPath + "/" + mainWindow->getAppName();
        if (!dir.mkpath(appDataPath)) {
            syslog(LOG_ERR, "[%s] Make path '%s' failed!", __func__, appDataPath.toStdString().c_str());
        }

        QString picturePath = appDataPath + "/图片";
        if (!dir.mkpath(picturePath)) {
            syslog(LOG_ERR, "[%s] Make path '%s' failed!", __func__, picturePath.toStdString().c_str());
        }

        QString moviePath = appDataPath + "/视频";
        if (!dir.mkpath(moviePath)) {
            syslog(LOG_ERR, "[%s] Make path '%s' failed!", __func__, moviePath.toStdString().c_str());
        }

        QString musicPath = appDataPath + "/音乐";
        if (!dir.mkpath(musicPath)) {
            syslog(LOG_ERR, "[%s] Make path '%s' failed!", __func__, musicPath.toStdString().c_str());
        }

        QString documentPath = appDataPath + "/文档";
        if (!dir.mkpath(documentPath)) {
            syslog(LOG_ERR, "[%s] Make path '%s' failed!", __func__, documentPath.toStdString().c_str());
        }

        return appDataPath;
    }

    return destPath;
}
