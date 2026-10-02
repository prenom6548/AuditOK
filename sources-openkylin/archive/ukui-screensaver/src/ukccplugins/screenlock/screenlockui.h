/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
#ifndef SCREENLOCKUI_H
#define SCREENLOCKUI_H

#include <widgets/settinggroup.h>
#include <widgets/flowlayout.h>
#include <widgets/switchwidget.h>
#include <widgets/comboxwidget.h>
#include <widgets/pushbuttonwidget.h>
#include <QProcess>
#include "pictureunit.h"
#include <QHBoxLayout>
#include <QWidget>
#include <kysdk/applications/klabel.h>
#include <kysdk/applications/accessinfohelper.h>
#include <kysdk/applications/kbackgroundgroup.h>
#include <QScrollArea>
#include "../screensaver/photocorver.h"

using namespace kdk;

class ScreenlockUi : public QWidget
{
    Q_OBJECT
public:
    ScreenlockUi();
    ~ScreenlockUi();
    void setShowOnLogin(bool b);
    void setLockDatetimeEnable(bool b);
    void setPicture(const QString &data);
    void setPictures(const QStringList &picturePathList, const QStringList &sourcePathList);
    void createPictureUnit(const QPixmap &pixmap, const QString &filename, const bool &clicked);
    void resetClickedPic();

private:
    void initUi();
    void initScreenLock();
    void initSignals();
    void initLeavLockStatus();
    bool hasBluetoothAdapter();
    bool isBluetoothPowered();
    bool getLeavLockOn();
    QString getLeaveLockDev();
    bool getLeaveLockPower();
    void setLeaveLockPower(bool on);
    QMap<QString, QString> getBtPairedDevices();
    /**
     * @brief 域管是否管控
     * @return true 管控  false 不管控
     */;
    bool getLeaveLockPowerCtl(); //域管离座锁

private slots:
    void addPairedDev(QString dev, bool paired);
    void powerChanged(bool powered);
    void setLeaveLock(bool on);
    void defaultAdapterChanged(QString address);
    void adapterAdded(QString address);
    void adapterRemoved(QString address);
    void setBtPushButtonClickSlot();
    void btLeaveLockButtonClickSlot(bool clicked);
    void bluetoothWidgetIdxChangeSlot(int index);
    void devConnectStatusChanged(QString dev, bool status);
    void leaveLockSuc();
    void leaveLockCtlChanged(bool on);

private:
    QVBoxLayout *m_uiLayout         = nullptr;
    QVBoxLayout *m_screenlockLayout = nullptr;

private:
    KLabel       *m_wallpaperTitleLabel = nullptr;
    UkccFrame    *m_previewFrame        = nullptr;
    QHBoxLayout  *m_previewLayout       = nullptr;
    QLabel       *m_previewLabel        = nullptr;
    SettingGroup *m_centerGroup         = nullptr;
    QWidget      *m_noBtWidget          = nullptr;
    UkccFrame    *m_pictureFrame        = nullptr;
    FlowLayout   *m_pictureLayout       = nullptr;
    UkccFrame    *m_bottomFrame         = nullptr;
    QHBoxLayout  *m_bottomLayout        = nullptr;
    QPushButton  *m_localBtn            = nullptr;
    QPushButton  *m_onlineBtn           = nullptr;
    QPushButton  *m_resetBtn            = nullptr;
    KSwitchButton* m_showPicSwitchButton = nullptr;
    KSwitchButton* m_lockDatetimeEnableBtn = nullptr;
    KSwitchButton* m_btLeaveLockSwitchButton = nullptr;
    KPushButton* m_setBtPushButton = nullptr;
    QWidget* m_setBtWidget = nullptr;
    UkccFrame* m_relateFrame = nullptr;
    QWidget* m_bluetoothWidget = nullptr;
    QComboBox* m_bluetoothComboBox = nullptr;
    KBackgroundGroup* m_contentBackgroundGroup = nullptr;
    QScrollArea *previewScrollArea = nullptr;
private:
    KLabel       *relateTitleLabel  = nullptr;
    SettingGroup     *relatedGroup      = nullptr;
    PushButtonWidget *monitorWidget     = nullptr;
    PushButtonWidget *screensaverWidget = nullptr;

private:
    PhotoCorver *prePictureUnit = nullptr;
    bool        canLeaveLock    = false;
    QString     currentFile;
    QString     lockdev;

Q_SIGNALS:
    void showOnLoginChanged(const bool &b);
    void lockDatetimeEnableChanged(const bool &b);
    void pictureChanged(const QString &picture);
    void localButtonClicked();
    void onlineButtonClicked();
    void resetButtonClicked();
    void toSetMonitor();
    void toSetScreensaver();
};


#endif // SCREENLOCKUI_H
