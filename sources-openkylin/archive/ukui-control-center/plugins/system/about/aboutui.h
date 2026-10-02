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
#ifndef ABOUTUI_H
#define ABOUTUI_H

#include <QObject>
#include <QWidget>
#include <QHBoxLayout>

#include "HpQRCodeInterface.h"
#include "ukcccommon.h"
#include "hostnamedialog.h"
#include "privacydialog.h"
#include "statusdialog.h"
#include "shell/interface.h"
#include "lightlabel.h"
#include "kswitchbutton.h"
#include <kysdk/kysdk-system/libkysysinfo.h>
#include <kborderlessbutton.h>
#include <klabel.h>
#include <kpushbutton.h>

using namespace ukcc;
using namespace kdk;

class AboutUi : public QWidget
{
    Q_OBJECT
public:
    explicit AboutUi(QWidget *parent = nullptr);

    void initUi();
    void initCopyRightName();
    QFrame *initHoldUi();
    QFrame *initActiveUi();
    QFrame *initPrivacyUi();
    QFrame *initInfoBtnUi();
    void initSearchText();
    void setComponent();

    QPushButton *getActivationBtn() {return mActivationBtn;}
    QPushButton *getHpBtn() {return mHpBtn;}
    QPushButton *getEducateBtn() {return mEducateBtn;}

    KSwitchButton *getPriBtn() {return mPriBtn;}

    KBorderlessButton *getTrialBtn() {return mTrialBtn;}
    KBorderlessButton *getAgreeBtn() {return mAgreeBtn;}

    QLabel *getLogo() {return mLogoLabel;}
    QLabel *getAndLabel() {return mAndLabel;}

    LightLabel *getVersion() {return mVersionLabel_2;}
    LightLabel *getBuild() {return mVersionNumberLabel_2;}
    LightLabel *getPatchVersion() {return mPatchVersionLabel_2;}
    LightLabel *getDesktopEnv() {return mDesktopLabel_2;}
    LightLabel *getUserName() {return mUsernameLabel_2;}
    LightLabel *getKerner() {return mKernelLabel_2;}
    LightLabel *getCpuInfo() {return mCpuLabel_2;}
    KLabel *getHostName() {return m_hostNameRenameValueLabel;}
    LightLabel *getShowHostName() {return mHostNameLabel_2;}
    LightLabel *getMemeory() {return mMemoryLabel_2;}
    KLabel *getActiveStatus() {return mStatusLabel_2;}
    LightLabel *getDiskContent() {return mDiskLabel_2;}
    LightLabel *getInstallDate() {return mInstallDateLabel_2;}
    LightLabel *getUpgradeDate() {return mUpgradeDateLabel_2;}
    KPushButton *getEditHost() {return m_hostNameRenameButton;}
    // 主机名变更后刷新重命名按钮读屏名（「主机名 重命名」，行标题由 buddy 提供）
    void refreshHostNameRenameAccessibleName();

    KLabel *getSequence() {return mSequenceLabel_1;}
    KBorderlessButton *getSequenceContent() {return mSequenceLabel_2;}
    KLabel *getDiskLabel() {return mDiskLabel_1;}

    QFrame *getBuildFrame() {return mVersionNumberFrame;}
    QFrame *getPatchFrame() {return mPatchVersionFrame;}
    QFrame *getActivaFrame() {return mActivationFrame;}
    QFrame *getDiskFrame() {return mDiskFrame;}
    QFrame *getInstallDateFrame() {return mInstallDateFrame;}
    QFrame *getUpgradeDateFrame() {return mUpgradeDateFrame;}
    QFrame* getUserNameFrame() {return mUsernameFrame;}
    KLabel* getHostNameLabel1() {return mHostNameLabel_1;}

    QHBoxLayout *getDiskLayout() {return mDiskLayout;}
    KLabel *getCustomerInfoLabel() {return m_customerInfoLabel;}
    LightLabel *getCustomerInfoValueLabel() {return m_customerInfoValueLabel;}

    bool isDomain() {return m_isHostNameEdit;}

Q_SIGNALS:
    void onLabelClicked();

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    QFrame *createFrame(KLabel *label_1, LightLabel *label_2, LightLabel *label_3 = nullptr);
    void setShape(QFrame *frame, QFrame::Shape shape);
    QPixmap loadSvg(const QString &path, int width, int height);

private:
    QFrame *mHoldWidget;
    QWidget *mQrCodeWidget;

    KLabel *mTitleLabel;
    KLabel *mPriTitleLabel;
    KLabel *mHoldTitleLabel;

    QFrame *mInformationFrame;
    QFrame *mActivationFrame;
    QFrame *mVersionFrame;
    QFrame *mVersionNumFrame;
    QFrame *mVersionNumberFrame;
    QFrame *mPatchVersionFrame;
    QFrame *mInstallDateFrame;
    QFrame *mUpgradeDateFrame;
    QFrame *mInterVersionFrame;
    QFrame *mHostNameFrame;
    QFrame *mKernelFrame;
    QFrame *mCpuFrame;
    QFrame *mMemoryFrame;
    QFrame *mDiskFrame;
    QFrame *mDesktopFrame;
    QFrame *mUsernameFrame;
    QFrame *mPrivacyFrame;
    QFrame *mBtnFrame;

    QHBoxLayout *mDiskLayout;

    QLabel *mLogoLabel;
    KLabel *mVersionLabel_1;
    KLabel *mVersionNumLabel_1;
    KLabel *mVersionNumberLabel_1;
    KLabel *mPatchVersionLabel_1;
    KLabel *mInstallDateLabel_1;
    KLabel *mUpgradeDateLabel_1;
    KLabel *mInterVersionLabel_1;
    KLabel *mHostNameLabel_1;
    KLabel *mKernelLabel_1;
    KLabel *mCpuLabel_1;
    KLabel *mMemoryLabel_1;
    KLabel *mDiskLabel_1;
    KLabel *mDesktopLabel_1;
    KLabel *mUsernameLabel_1;
    LightLabel *mVersionLabel_2;
    LightLabel *mVersionNumLabel_2;
    LightLabel *mVersionNumberLabel_2;
    LightLabel *mPatchVersionLabel_2;
    LightLabel *mInstallDateLabel_2;
    LightLabel *mUpgradeDateLabel_2;
    LightLabel *mInterVersionLabel_2;
    LightLabel *mHostNameLabel_2;

    QFrame *m_hostNameRenameFrame;
    KLabel *m_hostNameRenameLabel;
    KLabel *m_hostNameRenameValueLabel;
    KPushButton *m_hostNameRenameButton;
    LightLabel *mKernelLabel_2;
    LightLabel *mCpuLabel_2;
    LightLabel *mMemoryLabel_2;
    LightLabel *mDiskLabel_2;
    LightLabel *mDesktopLabel_2;
    LightLabel *mUsernameLabel_2;


    KLabel *mStatusLabel_1;
    KLabel *mStatusLabel_2;
    KLabel *mSequenceLabel_1;
    KBorderlessButton *mSequenceLabel_2;
    KLabel *mTrialLabel;
    KLabel *mAndLabel;
    KLabel *mAgreeLabel;
    QLabel *mHpLabel;
    KLabel *mEducateIconLabel;
    QLabel *mEducateLabel;
    KLabel *mTipLabel;
    KLabel* m_customerInfoLabel = nullptr;
    LightLabel* m_customerInfoValueLabel = nullptr;
    KLabel* m_serviceSupportLabel = nullptr;
    LightLabel* m_serviceSupportValueLabel = nullptr;

    QPushButton *mActivationBtn;
    QPushButton *mHpBtn;
    QPushButton *mEducateBtn;
    KSwitchButton *mPriBtn;

    KBorderlessButton *mTrialBtn;
    KBorderlessButton *mAgreeBtn;

    QSpacerItem *item;

    hp::QRCodeInterface *app; //hp插件

    QString mCopyRightName;
    QString mCopyRightYear;

    bool m_isHostNameEdit = true;
};

#endif // ABOUTUI_H
