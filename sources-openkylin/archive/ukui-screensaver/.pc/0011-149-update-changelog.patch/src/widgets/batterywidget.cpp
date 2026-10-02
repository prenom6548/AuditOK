/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
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
#include "batterywidget.h"
#include <QVBoxLayout>
#include <QDBusReply>
#include <QDebug>
#include <QStyleOption>
#include <QPainter>
#include <QEvent>

BatteryWidget::BatteryWidget(LockDialogModel *model, QWidget *parent) : QWidget(parent), m_modelLockDialog(model)
{
    //    setWindowOpacity(0);
    setAttribute(Qt::WA_TranslucentBackground);
    initUi();
    setupComponent();
}

void BatteryWidget::initUi()
{
    QVBoxLayout *Lyt = new QVBoxLayout(this);
    Lyt->setContentsMargins(16, 16, 16, 16);
    Lyt->setSpacing(8);
    mModeLabel = new QLabel(this);
    mModeLabel->setFixedHeight(24);
    mModeLabel->setStyleSheet("QLabel{font-size: 16px;font-family: NotoSansCJKsc-Bold, NotoSansCJKsc;"
                              "font-weight: bold;}");

    QHBoxLayout *Lyt_1 = new QHBoxLayout();
    Lyt_1->setSpacing(4);

    mIconBtn = new QPushButton(this);
    mIconBtn->setFixedSize(48, 32);
    mIconBtn->setStyleSheet("QPushButton{\
                            color: rgba(255, 255, 255, 255);\
                            border: none;\
                            border-radius: 4px;\
                            outline: none;\
                        }");

    mValueLabel = new QLabel(this);
    mValueLabel->setFixedHeight(48);
    mValueLabel->setMidLineWidth(48);

    mStatusLabel = new QLabel(this);
    mStatusLabel->setFixedHeight(36);
    mStatusLabel->setAlignment(Qt::AlignRight);

    Lyt_1->addWidget(mIconBtn);
    Lyt_1->addWidget(mValueLabel);
    Lyt_1->addStretch();
    Lyt_1->addWidget(mStatusLabel);

    Lyt->addWidget(mModeLabel);
    Lyt->addLayout(Lyt_1);
    Lyt->addStretch();
}

void BatteryWidget::setupComponent()
{
    mIconBtn->setIconSize(QSize(48, 32));
    mIconBtn->setFocusPolicy(Qt::NoFocus);
    mIconBtn->setIcon(QIcon::fromTheme(m_modelLockDialog->getBatteryIconName()));

    onBatteryChanged(m_modelLockDialog->getBatteryArgs());

    connect(m_modelLockDialog, &LockDialogModel::batteryStatusChanged, this, &BatteryWidget::onBatteryStatusChanged);

    connect(m_modelLockDialog, &LockDialogModel::batteryChanged, this, &BatteryWidget::onBatteryChanged);
}

void BatteryWidget::setPoint(QPoint point)
{
    mPoint = point;
}

void BatteryWidget::paintEvent(QPaintEvent *event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.save();
    p.setBrush(opt.palette.color(QPalette::Base));
    p.setPen(Qt::transparent);
    p.setOpacity(0.75);
    p.drawRoundedRect(this->rect(), 16, 16);
    p.restore();
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void BatteryWidget::onBatteryChanged(QStringList args)
{
    m_nCurBatteryValue = args.at(0).toInt();
    m_nCurBatteryState = args.at(1).toInt();
    if (m_nCurBatteryState == 1 || m_nCurBatteryState == 5) {
        mStatusLabel->setText(tr("Charging..."));
    } else if (m_nCurBatteryState == 4) {
        mStatusLabel->setText(tr("fully charged"));
    }
    if (m_nCurBatteryState == 4 || m_nCurBatteryState == 1 || m_nCurBatteryState == 5) {
        mStatusLabel->setVisible(true);
        mModeLabel->setText(tr("PowerMode"));
    } else {
        mStatusLabel->setVisible(false);
        mModeLabel->setText(tr("BatteryMode"));
    }

    mValueLabel->setText(QString("<font size='5';font color=#262626>%1</font>%").arg(m_nCurBatteryValue));
}

void BatteryWidget::onBatteryStatusChanged(QString iconName)
{
    mIconBtn->setIcon(QIcon::fromTheme(iconName));
}

void BatteryWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        refreshTranslate();
    }
}

void BatteryWidget::refreshTranslate()
{
    if (m_nCurBatteryState == 1 || m_nCurBatteryState == 5) {
        mStatusLabel->setText(tr("Charging..."));
    } else if (m_nCurBatteryState == 4) {
        mStatusLabel->setText(tr("fully charged"));
    }
    if (m_nCurBatteryState == 4 || m_nCurBatteryState == 1 || m_nCurBatteryState == 5) {
        mStatusLabel->setVisible(true);
        mModeLabel->setText(tr("PowerMode"));
    } else {
        mStatusLabel->setVisible(false);
        mModeLabel->setText(tr("BatteryMode"));
    }

    mValueLabel->setText(QString("<font size='5';font color=#262626>%1</font>%").arg(m_nCurBatteryValue));
}
