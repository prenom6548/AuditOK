/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
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
#ifndef HIDDENWIFIPAGE_H
#define HIDDENWIFIPAGE_H

#include <QWidget>
#include <QDesktopWidget>
#include <QApplication>
#include <QFormLayout>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QScrollArea>

//#include "coninfo.h"
#include "../component/Divider/divider.h"
#include "kwidget.h"
#include "kpasswordedit.h"
#include "kborderlessbutton.h"
#include "entsecuritywidget.h"
#include "kylin-nm/depend/kywirelessconnectoperation.h"

using namespace kdk;


class HiddenWiFiPage : public QWidget
{
    Q_OBJECT

public:
    HiddenWiFiPage(QString interface, bool isLockScreen, QWidget *parent = nullptr);
    ~HiddenWiFiPage();

    void getSecuType(KySecuType &secuType);
    void setOobe(bool on);

protected:
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *w, QEvent *event);

private:
    void initUI();
    void centerToScreen();
    void initComponent();

    void showNone();
    void showPsk();
    void showEnt();

    void setBtnEnable(bool on);
    void setJoinBtnEnable();
    void setWindowWidth(KyEapMethodType eapType);

private:
    QWidget *m_topWidget;
    QWidget *m_centerWidget;
    QWidget *m_bottomWidget;
    EntSecurityWidget *m_secuWidget;

    QLabel *m_descriptionLabel;
    FixLabel *m_nameLabel;
    FixLabel *m_secuTypeLabel;
    QLabel *m_pwdLabel;
    QLabel *m_emptyLabel;
    QLabel *m_checkLabel;

    LineEdit *m_nameEdit;
    QComboBox *m_secuTypeCombox;
    KPasswordEdit *m_pwdEdit = nullptr;

    QCheckBox *m_rememberCheckBox = nullptr;

    Divider *m_bottomDivider = nullptr;
//    KBorderlessButton *m_showListBtn;
    QPushButton *m_cancelBtn;
    QPushButton *m_joinBtn;

    QScrollArea *m_scrollArea;
    QVBoxLayout *m_pageLayout;
    QHBoxLayout *m_topLayout;
    QVBoxLayout *m_centerVBoxLayout;
    QHBoxLayout *m_bottomLayout;

    bool m_isJoinBtnEnable;
    bool m_isSecuOk = false;

    QString      m_deviceName;
    bool m_isLockScreen = false;
    bool m_isOobe = false;

private Q_SLOTS:
    void on_btnJoin_clicked();
    void onSecuTypeComboxIndexChanged();

Q_SIGNALS:
    void connectHidePeapConnect(KyEapMethodPeapInfo info, KyWirelessConnectSetting connSettingInfo);
    void connectHideTtlsConnect(KyEapMethodTtlsInfo info, KyWirelessConnectSetting connSettingInfo);
    void connectHideNormalConnect(KyWirelessConnectSetting connSettingInfo, KySecuType type);
    void connectHideLeapConnect(KyEapMethodLeapInfo info, KyWirelessConnectSetting connSettingInfo);
    void connectHidePwdConnect(KyEapMethodPwdInfo info, KyWirelessConnectSetting connSettingInfo);
    void connectHideFastConnect(KyEapMethodFastInfo info, KyWirelessConnectSetting connSettingInfo);
};
#endif // HIDDENWIFIPAGE_H
