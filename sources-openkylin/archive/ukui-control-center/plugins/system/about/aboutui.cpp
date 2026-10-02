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
#include "aboutui.h"
#include <QGridLayout>
#include <QSvgRenderer>
#include <QPluginLoader>
#include <QDebug>
#include <QDate>
#include <kysdk/applications/accessinfohelper.h>
#include "ukccframe.h"
#ifndef OPENKYLIN
#include <kimclient.h>
#endif

using namespace kdk;

AboutUi::AboutUi(QWidget *parent) : QWidget(parent)
{
    initCopyRightName();
    initUi();
    initSearchText();
    setComponent();
}

void AboutUi::initUi()
{
    QVBoxLayout *mverticalLayout = new QVBoxLayout(this);
    mverticalLayout->setSpacing(0);
    mverticalLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *Aboutwidget = new QWidget(this);
    Aboutwidget->setMinimumSize(QSize(550, 0));
    Aboutwidget->setMaximumSize(QSize(16777215, 16777215));

    QVBoxLayout *AboutLayout = new QVBoxLayout(Aboutwidget);
    AboutLayout->setContentsMargins(0, 0, 0, 0);
    AboutLayout->setSpacing(8);

    mTitleLabel = new KLabel(Aboutwidget);
    mTitleLabel->setContentsMargins(16, 0, 0, 0);
    KDK_EXTEND_ALL_INFO_FORMAT(mTitleLabel, "About", "", "");
    //~ contents_path /About/System Summary
    mTitleLabel->setText(tr("System Summary"));
    mPriTitleLabel = new KLabel(Aboutwidget);
    mPriTitleLabel->setContentsMargins(16, 0, 0, 0);
    KDK_EXTEND_ALL_INFO_FORMAT(mPriTitleLabel, "About", "", "");
    //~ contents_path /About/Privacy And Agreement
    mPriTitleLabel->setText(tr("Privacy And Agreement"));
    mHoldTitleLabel = new KLabel(Aboutwidget);
    KDK_EXTEND_ALL_INFO_FORMAT(mHoldTitleLabel, "About", "", "");
    mHoldTitleLabel->setContentsMargins(16, 0, 0, 0);
    mHoldTitleLabel->setText(tr("Support"));

    /* 计算机重命名 */
    m_hostNameRenameLabel = new KLabel(this);
    // 勿 KDK_EXTEND：会冲掉 setBuddy 的 LABELLED_BY
    //~ contents_path /About/HostName:
    m_hostNameRenameLabel->setText(tr("HostName:"));

    m_hostNameRenameValueLabel = new KLabel;
    KDK_EXTEND_ALL_INFO_FORMAT(m_hostNameRenameValueLabel, "About", "", "");
    m_hostNameRenameValueLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

    m_hostNameRenameButton = new KPushButton(this);
    KDK_EXTEND_ALL_INFO_FORMAT(m_hostNameRenameButton, "About", "", "");
    m_hostNameRenameButton->setText(tr("Rename"));
    // 行标题只走 setBuddy（LABELLED_BY）；accessibleName 含主机名+重命名，避免「计算机名称」读两遍
    m_hostNameRenameLabel->setAccessibleName(tr("HostName:"));
    m_hostNameRenameLabel->setBuddy(m_hostNameRenameButton);
    refreshHostNameRenameAccessibleName();

    m_hostNameRenameFrame = new UkccFrame(Aboutwidget, UkccFrame::BorderRadiusStyle::Around, true);
    QHBoxLayout *hostNameHlayout = new QHBoxLayout(m_hostNameRenameFrame);
    hostNameHlayout->setContentsMargins(16, 0, 16, 0);
    hostNameHlayout->addWidget(m_hostNameRenameLabel);
    hostNameHlayout->addWidget(m_hostNameRenameValueLabel);
    hostNameHlayout->addStretch();
    hostNameHlayout->addWidget(m_hostNameRenameButton);

    mInformationFrame = new QFrame(Aboutwidget);
    setShape(mInformationFrame, QFrame::Box);

    QVBoxLayout *mInformationLayout = new QVBoxLayout(mInformationFrame);
    int space = UkccCommon::isTablet() ? 32 : 16;
    mInformationLayout->setContentsMargins(space, space, space, space);

    /* LOGO */
    mLogoLabel = new QLabel(mInformationFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(mLogoLabel, "About", "", "");
    mLogoLabel->setFixedSize(130, 50);
    mLogoLabel->installEventFilter(this);

     /* 版本名称 */
    mVersionLabel_1 = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mVersionLabel_1, "About", "", "");
    mVersionLabel_1->setText(tr("Version"));
    mVersionLabel_1->setAccessibleName(tr("Version"));
    mVersionLabel_1->setAccessibleDescription(QString());
    mVersionLabel_2 = new LightLabel;
    KDK_EXTEND_ALL_INFO_FORMAT(mVersionLabel_2, "About", "", "");
    mVersionFrame = createFrame(mVersionLabel_1, mVersionLabel_2);

     /* 版本号 */
    mVersionNumberLabel_1 = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mVersionNumberLabel_1, "About", "", "");
    mVersionNumberLabel_1->setText(tr("Version Number"));
    mVersionNumberLabel_1->setAccessibleName(tr("Version Number"));
    mVersionNumberLabel_1->setAccessibleDescription(QString());
    mVersionNumberLabel_2 = new LightLabel;
    KDK_EXTEND_ALL_INFO_FORMAT(mVersionNumberLabel_2, "About", "", "");
    mVersionNumberFrame = createFrame(mVersionNumberLabel_1, mVersionNumberLabel_2);

    /* 系统补丁版本号 */
    mPatchVersionLabel_1 = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mPatchVersionLabel_1, "About", "", "");
    mPatchVersionLabel_1->setText(tr("Patch Version"));
    mPatchVersionLabel_1->setAccessibleName(tr("Patch Version"));
    mPatchVersionLabel_1->setAccessibleDescription(QString());
    mPatchVersionLabel_2 = new LightLabel;
    KDK_EXTEND_ALL_INFO_FORMAT(mPatchVersionLabel_2, "About", "", "");
    mPatchVersionFrame = createFrame(mPatchVersionLabel_1, mPatchVersionLabel_2);

    /* 安装日期 */
    mInstallDateLabel_1 = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mInstallDateLabel_1, "About", "", "");
    mInstallDateLabel_1->setText(tr("Installed Date"));
    mInstallDateLabel_1->setAccessibleName(tr("Installed Date"));
    mInstallDateLabel_1->setAccessibleDescription(QString());
    mInstallDateLabel_2 = new LightLabel;
    KDK_EXTEND_ALL_INFO_FORMAT(mInstallDateLabel_2, "About", "", "");
    mInstallDateFrame = createFrame(mInstallDateLabel_1, mInstallDateLabel_2);

    /* 系统更新时间 */
    mUpgradeDateLabel_1 = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mUpgradeDateLabel_1, "About", "", "");
    mUpgradeDateLabel_1->setText(tr("Upgrade Date"));
    mUpgradeDateLabel_1->setAccessibleName(tr("Upgrade Date"));
    mUpgradeDateLabel_1->setAccessibleDescription(QString());
    mUpgradeDateLabel_2 = new LightLabel;
    KDK_EXTEND_ALL_INFO_FORMAT(mUpgradeDateLabel_2, "About", "", "");
    mUpgradeDateFrame = createFrame(mUpgradeDateLabel_1, mUpgradeDateLabel_2);

    /* 主机名 */
    mHostNameLabel_1 = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mHostNameLabel_1, "About", "", "");
    //~ contents_path /About/HostName
    mHostNameLabel_1->setText(tr("HostName"));
    mHostNameLabel_1->setAccessibleName(tr("HostName"));
    mHostNameLabel_1->setAccessibleDescription(QString());
    mHostNameLabel_2 = new LightLabel;
    KDK_EXTEND_ALL_INFO_FORMAT(mHostNameLabel_2, "About", "", "");
    mHostNameLabel_2->installEventFilter(this);
    mHostNameFrame = createFrame(mHostNameLabel_1, mHostNameLabel_2);

#ifndef OPENKYLIN
    if (check_domain_installed() == 1) {
        m_hostNameRenameButton->setEnabled(false);
        m_isHostNameEdit = false;
    }
#endif

    /* 内核 */
    mKernelLabel_1 = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mKernelLabel_1, "About", "", "");
    mKernelLabel_1->setText(tr("Kernel"));
    mKernelLabel_1->setAccessibleName(tr("Kernel"));
    mKernelLabel_1->setAccessibleDescription(QString());
    mKernelLabel_2 = new LightLabel;
    KDK_EXTEND_ALL_INFO_FORMAT(mKernelLabel_2, "About", "", "");
    mKernelFrame = createFrame(mKernelLabel_1, mKernelLabel_2);

    /* CPU */
    mCpuLabel_1 = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mCpuLabel_1, "About", "", "");

    mCpuLabel_1->setText(tr("CPU"));
    mCpuLabel_1->setAccessibleName(tr("CPU"));
    mCpuLabel_1->setAccessibleDescription(QString());
    mCpuLabel_2 = new LightLabel;
    KDK_EXTEND_ALL_INFO_FORMAT(mCpuLabel_2, "About", "", "");
    mCpuFrame = createFrame(mCpuLabel_1, mCpuLabel_2);

    /* 内存 */
    mMemoryLabel_1 = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mMemoryLabel_1, "About", "", "");
    mMemoryLabel_1->setText(tr("Memory"));
    mMemoryLabel_1->setAccessibleName(tr("Memory"));
    mMemoryLabel_1->setAccessibleDescription(QString());
    mMemoryLabel_2 = new LightLabel;
    KDK_EXTEND_ALL_INFO_FORMAT(mMemoryLabel_2, "About", "", "");
    mMemoryFrame = createFrame(mMemoryLabel_1, mMemoryLabel_2);

    /* 硬盘 */
    mDiskLabel_1 = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mDiskLabel_1, "About", "", "");
    mDiskLabel_1->setText(tr("Disk"));
    mDiskLabel_1->setAccessibleName(tr("Disk"));
    mDiskLabel_1->setAccessibleDescription(QString());
    mDiskLabel_2 = new LightLabel;
    KDK_EXTEND_ALL_INFO_FORMAT(mDiskLabel_2, "About", "", "");
    mDiskFrame = createFrame(mDiskLabel_1, mDiskLabel_2);

    /* 桌面 */
    mDesktopLabel_1 = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mDesktopLabel_1, "About", "", "");
    mDesktopLabel_1->setText(tr("Desktop"));
    mDesktopLabel_1->setAccessibleName(tr("Desktop"));
    mDesktopLabel_1->setAccessibleDescription(QString());
    mDesktopLabel_2 = new LightLabel;
    KDK_EXTEND_ALL_INFO_FORMAT(mDesktopLabel_2, "About", "", "");
    mDesktopFrame = createFrame(mDesktopLabel_1, mDesktopLabel_2);

    /* 用户名 */
    mUsernameLabel_1 = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mUsernameLabel_1, "About", "", "");
    mUsernameLabel_1->setText(tr("User"));
    mUsernameLabel_1->setAccessibleName(tr("User"));
    mUsernameLabel_1->setAccessibleDescription(QString());
    mUsernameLabel_2 = new LightLabel;
    KDK_EXTEND_ALL_INFO_FORMAT(mUsernameLabel_2, "About", "", "");
    mUsernameFrame = createFrame(mUsernameLabel_1, mUsernameLabel_2);

    /* 激活信息 */
    mActivationFrame = initActiveUi();

    /* 版权所有 */
    mTipLabel = new KLabel(Aboutwidget);
    KDK_EXTEND_ALL_INFO_FORMAT(mTipLabel, "About", "", "");
    mTipLabel->setText(QString(tr("Copyright ©%1 %2. All rights reserved.").arg(mCopyRightYear).arg(mCopyRightName)));
    mTipLabel->setAccessibleName(mTipLabel->text());
    mTipLabel->setAccessibleDescription(QString());
    mTipLabel->setContentsMargins(16, 0, 0, 0);

    /* 隐私和协议 */
    mPrivacyFrame = initPrivacyUi();

    /* 授权信息和隐私协议弹窗 */
    mBtnFrame = initInfoBtnUi();

    /* Intel手册 */
    mHoldWidget = initHoldUi();

    item = new QSpacerItem(10 ,32);
    mInformationLayout->addWidget(mLogoLabel);
    mInformationLayout->addSpacing(24);
    mInformationLayout->addWidget(mVersionFrame);
    mInformationLayout->addWidget(mVersionNumberFrame);
    mInformationLayout->addWidget(mPatchVersionFrame);
    mInformationLayout->addWidget(mInstallDateFrame);
    mInformationLayout->addWidget(mUpgradeDateFrame);
    mInformationLayout->addWidget(mHostNameFrame);
    mInformationLayout->addWidget(mKernelFrame);
    mInformationLayout->addWidget(mCpuFrame);
    mInformationLayout->addWidget(mMemoryFrame);
    mInformationLayout->addWidget(mDiskFrame);
    mInformationLayout->addWidget(mDesktopFrame);
    mInformationLayout->addWidget(mUsernameFrame);

    AboutLayout->addWidget(mTitleLabel);
    AboutLayout->addWidget(m_hostNameRenameFrame);
    AboutLayout->addWidget(mInformationFrame);
    AboutLayout->addWidget(mActivationFrame);
    AboutLayout->addWidget(mTipLabel);
    AboutLayout->addSpacerItem(item);
    AboutLayout->addWidget(mPriTitleLabel);
    AboutLayout->addWidget(mPrivacyFrame);
    AboutLayout->addSpacing(0);
    AboutLayout->addWidget(mBtnFrame);
    AboutLayout->addWidget(mHoldTitleLabel);
    AboutLayout->addWidget(mHoldWidget);

    mverticalLayout->addWidget(Aboutwidget);
    mverticalLayout->addStretch();
}

bool AboutUi::eventFilter(QObject *object, QEvent *event)
{
    if (object == mLogoLabel && event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            qDebug() << Q_FUNC_INFO << "mLogoLabel click";
            emit onLabelClicked();
            return true;
        }
    }
    return QObject::eventFilter(object, event);
}

void AboutUi::initCopyRightName()
{
    mCopyRightName = tr("KylinSoft");
    mCopyRightYear = "2025";
    if (UkccCommon::isOpenkylin()) {
        mCopyRightName = tr("openKylin");
        mCopyRightYear = "2022";
    }
}

QFrame *AboutUi::createFrame(KLabel *label_1, LightLabel *label_2, LightLabel *label_3)
{
    QFrame *frame = new QFrame;
    frame->installEventFilter(this);
    setShape(frame, QFrame::NoFrame);

    QHBoxLayout *layout = new QHBoxLayout(frame);
    layout->setContentsMargins(0, 0, 16, 0);

    label_1->setFixedSize(180,30);

    label_2->setTextInteractionFlags(Qt::TextSelectableByMouse);
    label_2->setCursor(Qt::IBeamCursor);
    label_2->setFixedHeight(30);

    layout->addWidget(label_1);
    layout->addSpacing(16);
    if (label_3) {
        layout->addWidget(label_2);
        layout->addWidget(label_3);
        layout->addStretch();
    } else {
        layout->addWidget(label_2, Qt::AlignLeft);
    }

    return frame;
}

void AboutUi::setShape(QFrame *frame, QFrame::Shape shape)
{
    frame->setMinimumSize(QSize(550, shape == QFrame::Box ? 0 : 30));
    frame->setMaximumSize(QSize(16777215, shape == QFrame::Box ? 16777215 : 30));
    frame->setFrameShape(shape);
}

QFrame *AboutUi::initHoldUi()
{
    QFrame *frame = new QFrame;
    frame->setMinimumSize(QSize(550, 120));
    frame->setMaximumSize(QSize(16777215, 120));
    frame->setFrameShape(QFrame::Box);

    QHBoxLayout *mHoldLayout = new QHBoxLayout(frame);
    mHoldLayout->setSpacing(8);
    mHoldLayout->setContentsMargins(16, 0, 8, 0);

    mQrCodeWidget = new QWidget;
    mQrCodeWidget->setFixedSize(104, 104);
    QHBoxLayout *qrhLyt = new QHBoxLayout(mQrCodeWidget);
    qrhLyt->setContentsMargins(0, 4, 0, 0);

    mHpLabel = new QLabel;
    KDK_EXTEND_ALL_INFO_FORMAT(mHpLabel, "About", "", "");
    mEducateLabel = new QLabel;
    KDK_EXTEND_ALL_INFO_FORMAT(mEducateLabel, "About", "", "");
    mEducateIconLabel = new KLabel;
    KDK_EXTEND_ALL_INFO_FORMAT(mEducateIconLabel, "About", "", "");
    mEducateIconLabel->setFixedSize(96,96);

    mHpLabel->setWordWrap(true);
    mEducateLabel->setWordWrap(true);
    mHpLabel->setText(tr("Wechat code scanning obtains HP professional technical support"));
    mEducateLabel->setText(tr("See more about Kylin Tianqi edu platform"));

    mHpBtn = new QPushButton;
    KDK_EXTEND_ALL_INFO_FORMAT(mHpBtn, "About", "", "");
    mEducateBtn = new QPushButton;
    KDK_EXTEND_ALL_INFO_FORMAT(mEducateBtn, "About", "", "");

#ifdef WIN32
    QPluginLoader loader("../HpQRCodePlugin/hp-qrcode-plugind.dll");
#else
    QPluginLoader loader("/usr/lib/x86_64-linux-gnu/hp-qrcode-plugin/libhp-qrcode-plugin.so");

#endif
    QObject *plugin = loader.instance();
    if (plugin) {
        app = qobject_cast<hp::QRCodeInterface*>(plugin);
        mQrCodeWidget->layout()->addWidget(app->createWidget(mQrCodeWidget));
    }
    else{
        qDebug() << "加载插件失败";
    }

    mHpBtn->setText(tr("Learn more HP user manual>>"));
    mHpBtn->setStyleSheet("background: transparent;color:#2FB3E8;font-family:Microsoft YaHei;"
                  "border-width:1px;text-decoration:underline;border-style:none none none;border-color:#2FB3E8;text-align: left");

    mEducateIconLabel->setPixmap(loadSvg(":/help-app.svg", mEducateIconLabel->size().width(), mEducateIconLabel->size().height()));
    mEducateBtn->setText(tr("See user manual>>"));
    mEducateBtn->setStyleSheet("background: transparent;color:#2FB3E8;font-size;font-family:Microsoft YaHei;"
                  "border-width:1px;text-decoration:underline;border-style:none none none;border-color:#2FB3E8;text-align: left");

    QVBoxLayout *hpLayout = new QVBoxLayout();
    hpLayout->setContentsMargins(0, 8, 0, 8);
    hpLayout->setSpacing(4);
    hpLayout->addStretch();
    hpLayout->addWidget(mHpLabel);
    hpLayout->addWidget(mHpBtn);
    hpLayout->addStretch();

    QVBoxLayout *eduLayout = new QVBoxLayout();
    eduLayout->setContentsMargins(0, 8, 0, 8);
    eduLayout->setSpacing(4);
    eduLayout->addStretch();
    eduLayout->addWidget(mEducateLabel);
    eduLayout->addWidget(mEducateBtn);
    eduLayout->addStretch();

    mHoldLayout->addWidget(mQrCodeWidget);
    mHoldLayout->addLayout(hpLayout);
    mHoldLayout->addSpacing(16);
    mHoldLayout->addStretch(1);
    mHoldLayout->addWidget(mEducateIconLabel);
    mHoldLayout->addLayout(eduLayout);
    mHoldLayout->addStretch(1);

    return frame;
}

QFrame *AboutUi::initActiveUi()
{
    QFrame *frame = new QFrame;
    setShape(frame, QFrame::Box);

    QHBoxLayout *mActivationLayout_1 = new QHBoxLayout(frame);
    mActivationLayout_1->setContentsMargins(16, 16, 16, 16);
    mActivationLayout_1->setSpacing(8);

    QGridLayout *mActivationLayout = new QGridLayout();
    mActivationLayout->setVerticalSpacing(8);

    mStatusLabel_1 = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mStatusLabel_1, "About", "", "");
    mStatusLabel_1->setFixedSize(196, 30);
    mStatusLabel_2 = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mStatusLabel_2, "About", "", "");
    mSequenceLabel_1 = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mSequenceLabel_1, "About", "", "");
    mSequenceLabel_1->setFixedSize(196, 30);
    mSequenceLabel_2 = new KBorderlessButton(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mSequenceLabel_2, "About", "", "");
    mSequenceLabel_2->setObjectName("sequence");
    mSequenceLabel_2->installEventFilter(this);
    mActivationBtn = new QPushButton(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mActivationBtn, "About", "", "");
    mActivationBtn->setObjectName("acivation");

    mActivationBtn->setFixedWidth(120);

    m_customerInfoLabel = new KLabel(frame);
    KDK_EXTEND_ALL_INFO_FORMAT(m_customerInfoLabel, "About", "", "");
    //~ contents_path /About/Customer Info
    m_customerInfoLabel->setText(tr("Customer Info"));
    m_customerInfoLabel->setAccessibleName(tr("Customer Info"));
    m_customerInfoLabel->setAccessibleDescription(QString());
    m_customerInfoLabel->setFixedSize(196,30);
    m_customerInfoValueLabel = new LightLabel(frame);
    KDK_EXTEND_ALL_INFO_FORMAT(m_customerInfoValueLabel, "About", "", "");

    m_serviceSupportLabel = new KLabel(frame);
    KDK_EXTEND_ALL_INFO_FORMAT(m_serviceSupportLabel, "About", "", "");
    //~ contents_path /About/Service And Support
    m_serviceSupportLabel->setText(tr("Service And Support"));
    m_serviceSupportLabel->setAccessibleName(tr("Service And Support"));
    m_serviceSupportLabel->setAccessibleDescription(QString());
    m_serviceSupportLabel->setFixedSize(196, 30);
    m_serviceSupportValueLabel = new LightLabel(frame);
    KDK_EXTEND_ALL_INFO_FORMAT(m_serviceSupportValueLabel, "About", "", "");
    m_serviceSupportValueLabel->setText("400-089-1870");

    mActivationLayout->addWidget(mStatusLabel_1, 0, 0, 1, 1);
    mActivationLayout->addWidget(mStatusLabel_2, 0, 1, 1, 3,Qt::AlignLeft);
    mActivationLayout->addWidget(mSequenceLabel_1, 1, 0, 1, 1);
    mActivationLayout->addWidget(mSequenceLabel_2, 1, 1, 1, 3,Qt::AlignLeft);
    mActivationLayout->addWidget(m_customerInfoLabel, 2, 0, 1, 1);
    mActivationLayout->addWidget(m_customerInfoValueLabel, 2, 1, 1, 3, Qt::AlignLeft);
    mActivationLayout->addWidget(m_serviceSupportLabel, 3, 0, 1, 1);
    mActivationLayout->addWidget(m_serviceSupportValueLabel, 3, 1, 1, 3, Qt::AlignLeft);

    mActivationLayout_1->addLayout(mActivationLayout);
    mActivationLayout_1->addWidget(mActivationBtn);

    return frame;
}

QFrame *AboutUi::initPrivacyUi()
{
    QFrame *frame = new QFrame;
    setShape(frame, QFrame::Box);

    QHBoxLayout *mPrivacyLayout = new QHBoxLayout(frame);
    mPrivacyLayout->setContentsMargins(16, 16, 16, 16);
    mPrivacyLayout->setSpacing(48);
    mPriBtn = new KSwitchButton(frame);
    KDK_EXTEND_ALL_INFO_FORMAT(mPriBtn, "About", "", "");
    mPriBtn->setObjectName("privacysettings");

    KLabel *mPriLabel_1 = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mPriLabel_1, "About", "", "");
    //~ contents_path /About/Send optional diagnostic data
    mPriLabel_1->setText(tr("Send optional diagnostic data"));
    KLabel *mPriLabel_2 = new KLabel(this);
    mPriLabel_2->setText(tr("By sending us diagnostic data, improve the system experience and solve your problems faster"));
    mPriLabel_2->setFontColorRole(QPalette::PlaceholderText);
    KDK_EXTEND_ALL_INFO_FORMAT(mPriLabel_2, "About", "", "");
    QVBoxLayout *mverticalLayout_2 = new QVBoxLayout;
    mverticalLayout_2->setSpacing(0);
    mverticalLayout_2->setContentsMargins(0, 0, 0, 0);
    mverticalLayout_2->addWidget(mPriLabel_1);
    mverticalLayout_2->addWidget(mPriLabel_2 );

    mPrivacyLayout->addLayout(mverticalLayout_2);
    mPrivacyLayout->addStretch();
    mPrivacyLayout->addWidget(mPriBtn);

    mPriBtn->setAccessibleName(tr("Send optional diagnostic data"));

    return frame;
}

QFrame *AboutUi::initInfoBtnUi()
{
    QFrame *frame = new QFrame;
    setShape(frame, QFrame::NoFrame);


    QHBoxLayout *mBtnLyt = new QHBoxLayout(frame);
    mBtnLyt->setContentsMargins(16, 0, 0, 0);
    //~ contents_path /About/<<Protocol>>
    mTrialBtn = new KBorderlessButton(tr("<<Protocol>>"));
    KDK_EXTEND_ALL_INFO_FORMAT(mTrialBtn, "About", "", "");
    mAndLabel = new KLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mAndLabel, "About", "", "");
    mAndLabel->setText(tr("and"));
    //~ contents_path /About/<<Privacy>>
    mAgreeBtn = new KBorderlessButton(tr("<<Privacy>>"));
    KDK_EXTEND_ALL_INFO_FORMAT(mAgreeBtn, "About", "", "");
    QFont font = mTrialBtn->font();
    font.setUnderline(true);
    mTrialBtn->setFont(font);
    mAgreeBtn->setFont(font);

    mBtnLyt->addWidget(mTrialBtn);
    mBtnLyt->addWidget(mAndLabel);
    mBtnLyt->addWidget(mAgreeBtn);
    mBtnLyt->addStretch();

    return frame;
}

void AboutUi::initSearchText()
{
    //~ contents_path /About/version
    mVersionLabel_1->setText(tr("Version"));
    mVersionLabel_1->setAccessibleName(tr("Version"));
    mVersionLabel_1->setAccessibleDescription(QString());
    //~ contents_path /About/Kernel
    mKernelLabel_1->setText(tr("Kernel"));
    mKernelLabel_1->setAccessibleName(tr("Kernel"));
    mKernelLabel_1->setAccessibleDescription(QString());
    //~ contents_path /About/CPU
    mCpuLabel_1->setText(tr("CPU"));
    mCpuLabel_1->setAccessibleName(tr("CPU"));
    mCpuLabel_1->setAccessibleDescription(QString());
    //~ contents_path /About/Memory
    mMemoryLabel_1->setText(tr("Memory"));
    mMemoryLabel_1->setAccessibleName(tr("Memory"));
    mMemoryLabel_1->setAccessibleDescription(QString());
    //~ contents_path /About/Desktop
    mDesktopLabel_1->setText(tr("Desktop"));
    mDesktopLabel_1->setAccessibleName(tr("Desktop"));
    mDesktopLabel_1->setAccessibleDescription(QString());
    //~ contents_path /About/User
    mUsernameLabel_1->setText(tr("User"));
    mUsernameLabel_1->setAccessibleName(tr("User"));
    mUsernameLabel_1->setAccessibleDescription(QString());
    //~ contents_path /About/Status
    mStatusLabel_1->setText(tr("Status"));
    mStatusLabel_1->setAccessibleName(tr("Status"));
    mStatusLabel_1->setAccessibleDescription(QString());
    //~ contents_path /About/Active
    mActivationBtn->setText(tr("Active"));
    //~ contents_path /About/Serial
    mSequenceLabel_1->setText(tr("Serial"));
    mSequenceLabel_1->setAccessibleName(tr("Serial"));
    mSequenceLabel_1->setAccessibleDescription(QString());

    m_hostNameRenameLabel->setAccessibleName(tr("HostName:"));
    m_hostNameRenameLabel->setBuddy(m_hostNameRenameButton);
    refreshHostNameRenameAccessibleName();
    mActivationBtn->setAccessibleName(tr("Active"));
    mTrialBtn->setAccessibleName(tr("<<Protocol>>"));
    mAgreeBtn->setAccessibleName(tr("<<Privacy>>"));
    if (mHpBtn) {
        mHpBtn->setAccessibleName(tr("Learn more HP user manual>>"));
    }
    if (mEducateBtn) {
        mEducateBtn->setAccessibleName(tr("See user manual>>"));
    }
    mSequenceLabel_2->setAccessibleName(tr("Serial"));
}

void AboutUi::setComponent()
{
    if (!UkccCommon::isTablet()) {
        mHoldWidget->hide();
        mHoldTitleLabel->hide();
    }  else {
        mPrivacyFrame->hide();
        mHostNameFrame->hide();
        m_hostNameRenameFrame->hide();
        mPriTitleLabel->hide();
        mAndLabel->hide();
        mAgreeBtn->hide();
        mActivationFrame->hide();
        mTipLabel->hide();
        mTrialBtn->hide();
        mPatchVersionFrame->hide();
    }
}

QPixmap AboutUi::loadSvg(const QString &path, int width, int height)
{
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

void AboutUi::refreshHostNameRenameAccessibleName()
{
    if (!m_hostNameRenameButton) {
        return;
    }
    const QString host = m_hostNameRenameValueLabel
        ? m_hostNameRenameValueLabel->text().trimmed()
        : QString();
    // buddy 已提供「计算机名称:」；此处只放主机名+重命名，避免标题读两遍且漏读主机名
    const QString name = host.isEmpty()
        ? tr("Rename")
        : (host + QLatin1Char(' ') + tr("Rename"));
    m_hostNameRenameButton->setAccessibleName(name);
    m_hostNameRenameButton->setAccessibleDescription(QString());
}
