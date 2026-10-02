/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
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
#ifndef BOOT_H
#define BOOT_H

#include <QObject>

#include <unistd.h>
#include <QRadioButton>
#include <QButtonGroup>

#include <kysdk/kysdk-system/libkysysinfo.h>
#include <kysdk/applications/kswitchbutton.h>
#include <kysdk/applications/kpushbutton.h>
#include <kysdk/applications/klabel.h>
#include <kysdk/applications/kbackgroundgroup.h>
using namespace kdk;

#include "comboxwidget.h"
#include "switchwidget.h"
#include "settinggroup.h"
#include "shell/interface.h"

#include "loadingwidget.h"

#define THEME_SCHAME         "org.ukui.style"
#define UKCC_SCHAME "org.ukui.control-center"
#define SYSTEMS_SAME "systems-same"

class Boot : public QObject, CommonInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.ukcc.CommonInterface")
    Q_INTERFACES(CommonInterface)

public:
    Boot();
    ~Boot() Q_DECL_OVERRIDE;

    QString plugini18nName() Q_DECL_OVERRIDE;
    int pluginTypes() Q_DECL_OVERRIDE;
    QWidget *pluginUi() Q_DECL_OVERRIDE;
    const QString name() const  Q_DECL_OVERRIDE;
    bool isShowOnHomePage() const Q_DECL_OVERRIDE;
    QIcon icon() const Q_DECL_OVERRIDE;
    bool isEnable() const Q_DECL_OVERRIDE;

private:
    QWidget *m_pPluginWidget = nullptr;
    int m_pluginType;
    QString m_pluginName;
    bool m_firstLoad;

    SwitchWidget *m_pBootFrame = nullptr;
    KPushButton *m_pResetPasswdBtn = nullptr;
    ComboxWidget *m_pGrubBootDelayFrame = nullptr;
    KBackgroundGroup *m_pBootOrderFrame = nullptr;
    KLabel *m_pDefaultBootLabel = nullptr;
    QButtonGroup *m_pBootListGroup = nullptr;
    QDBusInterface *m_pUkccSystemDbus = nullptr;
    QDBusInterface *m_pUkccSessionDbus = nullptr;
    int m_preBootDelay;
    int m_preBootDefault = 0;
    int m_preFailureTimes;
    LoadingWidget *m_pLoadingWidget = nullptr;
    QGSettings *m_pukccGsetting = nullptr;

    SettingGroup *m_pRescueModeGroup = nullptr;
    SwitchWidget *m_pRescueModeFrame = nullptr;
    ComboxWidget *m_pFailuersFrame = nullptr;

private:
    void initUI(QWidget *widget);
    void initConnection();
    void gsettingConnection();
    void initGrubVerifyStatus();
    void initBootTime();
    void initRescueMode();

    void setGrubPasswd(QString pwd, bool isOpen);
    void initBootList();
    void initDefaultBoot();
    // 行标题只绑/写入当前 Tab 落点那一项（优先已选），避免每项都读「默认启动的系统」
    void refreshDefaultBootRowAccessible();
    bool authoriyGrub();
    void showLoadingWidget(QString text);

private Q_SLOTS:
    void bootSlot(bool checked);
    void resetPasswdSlot();
    void changeBootDelay(int index);
    void setDefaultBoot(QAbstractButton *button);
    void rescueModeChangedSlot(bool checked);
    void failureTimesChangedSlot(int index);
};
#endif // BOOT_H
