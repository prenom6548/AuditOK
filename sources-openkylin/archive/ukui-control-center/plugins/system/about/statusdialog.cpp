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
#include "statusdialog.h"
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QDBusReply>
#include <QDebug>
#include <kysdk/applications/accessinfohelper.h>
using namespace kdk;

StatusDialog::StatusDialog(QWidget *parent) :
    KDialog(parent)
{
    this->setWindowFlags(Qt::Dialog);
    setWindowTitle(tr("About"));
    setWindowIcon("ukui-control-center");
    initUI();
    getActivationCode();
    m_activeInterface = new QDBusInterface("org.freedesktop.activation",
                                           "/org/freedesktop/activation",
                                           "org.freedesktop.activation.interface",
                                           QDBusConnection::systemBus(), this);
    if (m_activeInterface && m_activeInterface->isValid()) {
        connect(m_activeInterface, SIGNAL(activation_result(int)), this, SLOT(activeSlot(int)));
    } else {
        qDebug() << Q_FUNC_INFO << "create activation QDBusInterface failed";
        return;
    }
    getRegisterCode();
}

StatusDialog::~StatusDialog()
{

}

void StatusDialog::initUI()
{
    this->setFixedSize(450, 462);
    QVBoxLayout *vLyt = new QVBoxLayout(this);
    vLyt->setContentsMargins(24, 16, 24, 40);
    vLyt->setSpacing(8);

    mLogoLabel = new QLabel(this);
    mLogoLabel->setMinimumHeight(60);
    const static int LABEL_FIX_WIDTH = 128;
    const static int FRAME_FIX_HEIGHT = 40;

    // 版本号
    mVersionFrame = new QFrame(this);
    QHBoxLayout *mVersionLyt = new QHBoxLayout(mVersionFrame);
    mVersionLyt->setSpacing(16);
    mVersionFrame->setFixedHeight(FRAME_FIX_HEIGHT);
    mVersionFrame->setFrameShape(QFrame::NoFrame);
    mVersionLabel_1 = new QLabel(mVersionFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(mVersionLabel_1, "About", "versionLabel1", "version label1 of StatusDialog");
    mVersionLabel_2 = new KLabel(mVersionFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(mVersionLabel_2, "About", "versionLabel2", "version label2 of StatusDialog");
    mVersionLabel_1->setFixedWidth(LABEL_FIX_WIDTH);
    mVersionLyt->addWidget(mVersionLabel_1);
    mVersionLyt->addWidget(mVersionLabel_2);

    // 系统状态
    mStatusFrame = new QFrame(this);
    QHBoxLayout *mStatusLyt = new QHBoxLayout(mStatusFrame);
    mStatusLyt->setSpacing(16);
    mStatusFrame->setFixedHeight(FRAME_FIX_HEIGHT);
    mStatusFrame->setFrameShape(QFrame::NoFrame);
    mStatusLabel_1 = new KLabel(mStatusFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(mStatusLabel_1, "About", "statusLabel1", "status label1 of StatusDialog");
    mStatusLabel_2 = new QLabel(mStatusFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(mStatusLabel_2, "About", "statusLabel2", "status label2 of StatusDialog");
    mStatusLabel_1->setFixedWidth(LABEL_FIX_WIDTH);
    mStatusLyt->addWidget(mStatusLabel_1);
    mStatusLyt->addWidget(mStatusLabel_2);

    // 激活码
    mActivationFrame = new QFrame(this);
    QHBoxLayout *mActivationLyt = new QHBoxLayout(mActivationFrame);
    mActivationLyt->setSpacing(16);
    mActivationFrame->setFixedHeight(FRAME_FIX_HEIGHT);
    mActivationFrame->setFrameShape(QFrame::NoFrame);
    mActivationCodeLabel_1 = new KLabel(mActivationFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(mActivationCodeLabel_1, "About", "activationCodeLabel1", "activation code label1 of StatusDialog");
    mActivationCodeLabel_1->setText(tr("Activation Code"));
    mActivationCodeLabel_2 = new KLabel(mActivationFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(mActivationCodeLabel_2, "About", "activationCodeLabel2", "activation code label2 of StatusDialog");
    mActivationCodeLabel_1->setFixedWidth(LABEL_FIX_WIDTH);
    mActivationLyt->addWidget(mActivationCodeLabel_1);
    mActivationLyt->addWidget(mActivationCodeLabel_2);

    // 注册码
    m_registerFrame = new QFrame(this);
    QHBoxLayout *registerLyt = new QHBoxLayout(m_registerFrame);
    registerLyt->setSpacing(16);
    m_registerFrame->setFixedHeight(FRAME_FIX_HEIGHT);
    m_registerFrame->setFrameShape(QFrame::NoFrame);
    m_registerCodeLabel_1 = new KLabel(m_registerFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(m_registerCodeLabel_1, "About", "m_registerCodeLabel_1", "register code label1 of StatusDialog");
    m_registerCodeLabel_1->setText(tr("Register Code"));
    m_registerCodeLabel_2 = new KLabel(m_registerFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(m_registerCodeLabel_2, "About", "m_registerCodeLabel_2", "register code label2 of StatusDialog");
    m_registerCodeLabel_2->setCursor(Qt::IBeamCursor);
    m_registerCodeLabel_2->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_registerCodeLabel_1->setFixedWidth(LABEL_FIX_WIDTH);
    registerLyt->addWidget(m_registerCodeLabel_1);
    registerLyt->addWidget(m_registerCodeLabel_2);
    m_registerFrame->hide();

    // 序列号
    mSerialFrame = new QFrame(this);
    QHBoxLayout *mSerialLyt = new QHBoxLayout(mSerialFrame);
    mSerialLyt->setSpacing(16);
    mSerialFrame->setFixedHeight(FRAME_FIX_HEIGHT);
    mSerialFrame->setFrameShape(QFrame::NoFrame);
    mSerialLabel_1 = new QLabel(mSerialFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(mSerialLabel_1, "About", "serialLabel1", "serialLabel1 of StatusDialog");
    mSerialLabel_2 = new KLabel(mSerialFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(mSerialLabel_2, "About", "serialLabel2", "serialLabel2 of StatusDialog");
    mSerialLabel_1->setFixedWidth(LABEL_FIX_WIDTH);
    mSerialLyt->addWidget(mSerialLabel_1);
    mSerialLyt->addWidget(mSerialLabel_2);

    // 技术服务到期时间
    mTimeFrame = new QFrame(this);
    QHBoxLayout *mTimeLyt = new QHBoxLayout(mTimeFrame);
    mTimeLyt->setSpacing(16);
    mTimeFrame->setFixedHeight(FRAME_FIX_HEIGHT);
    mTimeFrame->setFrameShape(QFrame::NoFrame);
    mTimeLabel_1 = new KLabel(mTimeFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(mTimeLabel_1, "About", "timeLabel1", "timeLabel1 of StatusDialog");
    mTimeLabel_2 = new KLabel(mTimeFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(mTimeLabel_2, "About", "timeLabel2", "timeLabel2 of StatusDialog");
    mTimeLabel_1->setFixedWidth(LABEL_FIX_WIDTH);
    mTimeLyt->addWidget(mTimeLabel_1);
    mTimeLyt->addWidget(mTimeLabel_2);

    mExtentBtn = new QPushButton(this);
    KDK_EXTEND_ALL_INFO_FORMAT(mExtentBtn, "About", "ExtendBtn", "extend button of StatusDialog");
    mExtentBtn->setFixedHeight(40);

    vLyt->addWidget(mLogoLabel);
    vLyt->addSpacing(24);
    vLyt->addWidget(mVersionFrame);
    vLyt->addWidget(mStatusFrame);
    vLyt->addWidget(mActivationFrame);
    vLyt->addWidget(m_registerFrame);
    vLyt->addWidget(mSerialFrame);
    vLyt->addWidget(mTimeFrame);
    vLyt->addStretch();
    vLyt->addWidget(mExtentBtn);
    mainWidget()->setLayout(vLyt);

    mExtentBtn->setAccessibleName(tr("Activation"));
}

void StatusDialog::getActivationCode()
{
    QString filePath = "/etc/.kyactivation";
    QFile file(filePath);
    if (file.exists()) {
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            mActivationFrame->hide();
            return;
        }
        QString str = file.readAll();
        file.close();
        mActivationCodeLabel_2->setText(str);
        mActivationFrame->setVisible(true);
    } else {
        mActivationFrame->hide();
    }
}

void StatusDialog::activeSlot(int activeSignal) {
    if (!activeSignal) {
        getRegisterCode();
    }
}

void StatusDialog::getRegisterCode()
{
    if (!m_activeInterface || !m_activeInterface->isValid()) {
        qDebug() << Q_FUNC_INFO << "m_activeInterface invalid" << m_activeInterface->isValid();
        return;
    }
    QDBusReply<QString> registerInfoReply = m_activeInterface->call("register_number");
    if (!registerInfoReply.isValid()) {
        m_registerFrame->hide();
        qDebug() << Q_FUNC_INFO << "register_number interface invalid";
    } else {
        QString registerInfo = registerInfoReply.value();
        if (registerInfo.isEmpty()) {
            m_registerFrame->hide();
            qDebug() << Q_FUNC_INFO << "register_number is empty";
        } else {
            m_registerFrame->setVisible(true);
            m_registerCodeLabel_2->setText(registerInfo);
            qDebug() << Q_FUNC_INFO << "get register_number" << registerInfo;
        }
    }
}
