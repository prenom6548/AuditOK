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

void BatteryWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        refreshTranslate();
    }
}

void BatteryWidget::refreshTranslate()
{
    setPowerStateLabelForAcOnlineChanged();
}

void BatteryWidget::setHasBattery(bool hasBattery)
{
    m_hasBattery = hasBattery;
}

void BatteryWidget::setIconName(QString iconName)
{
    m_iconName = iconName;
    mIconBtn->setIcon(QIcon::fromTheme(m_iconName));
}

void BatteryWidget::setAcOnlineState(bool acOnlineState)
{
    if (!mModeLabel->text().isEmpty() && m_isAcOnlineState == acOnlineState) {
        return;
    }
    m_isAcOnlineState = acOnlineState;
    if (true == m_isAcOnlineState) {
        mModeLabel->setText(tr("PowerMode"));
    } else {
        mModeLabel->setText(tr("BatteryMode"));
    }
}

void BatteryWidget::setBatteryState(int iBatteryState)
{
    if (m_nCurBatteryState == iBatteryState) {
        m_batteryStateChanged = false;
    } else {
        m_batteryStateChanged = true;
        m_nCurBatteryState = iBatteryState;
    }
}

void BatteryWidget::setBatteryPercentage(int iBatteryPercentage)
{
    m_nCurBatteryValue = iBatteryPercentage;
    if (m_hasBattery) {
        mValueLabel->setText(QString("%1%").arg(m_nCurBatteryValue));
        mValueLabel->show();
    } else {
        mValueLabel->hide();
    }
}

void BatteryWidget::setShowPowerLeftTime(int iShowPowerLeftTime)
{
    m_showBatteryLeftTime = iShowPowerLeftTime;
}

void BatteryWidget::setBatteryLeftTime(qlonglong timeToFull, qlonglong timeToEmpty)
{
    m_timeToFull = timeToFull;
    m_timeToEmpty = timeToEmpty;
}

void BatteryWidget::setPowerStateLabelForAcOnlineChanged()
{
    // 添加无电池情况的处理
    if (!m_hasBattery) {
        if (m_isAcOnlineState) {
            mStatusLabel->setText(tr("No battery detected"));
        }
        return;
    }

    if (true == m_isAcOnlineState) {
        switch (m_nCurBatteryState) {
            case battery_state_unknown:
                qWarning() << "battery state unknown";
                break;
            case battery_state_fully:
		qWarning() << "battery state fully";
                if (100 == m_nCurBatteryValue) {
                    mStatusLabel->setText(tr("Fully Charged"));
                } else {
                    mStatusLabel->setText(tr("Not Charging"));
                }
                break;
            case battery_state_discharging:
                // todo 有时候电池保护或者充满可能会出现该状态，不确定，不是100%时暂时先显示未充电
                qWarning() << "battery state discharging";
                if (100 <= m_nCurBatteryValue) {
                    mStatusLabel->setText(tr("Fully Charged"));
                } else {
                    mStatusLabel->setText(tr("Not Charging"));
                }
                break;
            case battery_state_charging:
                if (100 <= m_nCurBatteryValue) {
                    mStatusLabel->setText(tr("Fully Charged"));
                } else {
                    mStatusLabel->setText(tr("Charging"));
                }
                break;
            case battery_state_pending_charge:
                mStatusLabel->setText(tr("Not Charging"));
                break;
            case battery_state_empty:
                // todo
                qWarning() << "battery state empty";
                break;
            case battery_state_last:
                // todo
                qWarning() << "battery state last";
                break;
            default:
                break;
        }
    } else {
        mStatusLabel->setText(tr("Discharging"));
    }
}

void BatteryWidget::setPowerStateLabelForBatteryInfoChanged()
{
    // 添加无电池情况的处理
    if (!m_hasBattery) {
        if (m_isAcOnlineState) {
            mStatusLabel->setText(tr("No battery detected"));
        }
        return;
    }

    QString power_state_text("");
    int hour = 0;
    int minute = 0;
    qint64 batteryLeftTime = 0;

    switch (m_nCurBatteryState) {
        case battery_state_unknown:
        case battery_state_fully:
            if (100 == m_nCurBatteryValue) {
                power_state_text = QString(tr("Fully Charged"));
            }
            break;
        case battery_state_charging:
            batteryLeftTime = m_timeToFull;
            if (false == m_showBatteryLeftTime || 0 == batteryLeftTime || true == m_batteryStateChanged
                || m_nCurBatteryValue > 95) {
                if (100 == m_nCurBatteryValue) {
                    power_state_text = QString(tr("Fully Charged"));
                } else {
                    power_state_text = QString(tr("Charging"));
                }
            } else {
                hour = batteryLeftTime / 3600;
                minute = ((batteryLeftTime) % 3600) / 60;
                if (0 == hour) {
                    power_state_text = QString(tr("%1 minutes \nuntil fully charged")).arg(minute);
                } else if (hour > 10) {
                    power_state_text = QString(tr("Charging"));
                } else {
                    if (1 == hour) {
                        if (1 == minute) {
                            power_state_text
                                = QString(tr("%1 hour %2 minute \nuntil fully charged")).arg(hour).arg(minute);
                        } else if (0 == minute) {
                            power_state_text = QString(tr("%1 hour \nuntil fully charged")).arg(hour);
                        } else {
                            power_state_text
                                = QString(tr("%1 hour %2 minutes \nuntil fully charged")).arg(hour).arg(minute);
                        }
                    } else {
                        if (1 == minute) {
                            power_state_text
                                = QString(tr("%1 hours %2 minute \nuntil fully charged")).arg(hour).arg(minute);
                        } else if (0 == minute) {
                            power_state_text = QString(tr("%1 hours \nuntil fully charged")).arg(hour);
                        } else {
                            power_state_text
                                = QString(tr("%1 hours %2 minutes \nuntil fully charged")).arg(hour).arg(minute);
                        }
                    }
                }
            }
            break;
        case battery_state_discharging:
            batteryLeftTime = m_timeToEmpty;
            if (false == m_showBatteryLeftTime || 0 == batteryLeftTime || true == m_batteryStateChanged) {
                power_state_text = QString(tr("Discharging"));
            } else {
                hour = batteryLeftTime / 3600;
                minute = ((batteryLeftTime) % 3600) / 60;
                if (0 == hour) {
                    power_state_text = QString(tr("%1 minutes \nremaining")).arg(minute);
                } else if (hour > 20) {
                    power_state_text = QString(tr("Discharging"));
                } else {
                    if (1 == hour) {
                        if (1 == minute) {
                            power_state_text = QString(tr("%1 hour %2 minute \nremaining")).arg(hour).arg(minute);
                        } else if (0 == minute) {
                            power_state_text = QString(tr("%1 hour \nremaining")).arg(hour);
                        } else {
                            power_state_text = QString(tr("%1 hour %2 minutes \nremaining")).arg(hour).arg(minute);
                        }
                    } else {
                        if (1 == minute) {
                            power_state_text = QString(tr("%1 hours %2 minute \nremaining")).arg(hour).arg(minute);
                        } else if (0 == minute) {
                            power_state_text = QString(tr("%1 hours \nremaining")).arg(hour);
                        } else {
                            power_state_text = QString(tr("%1 hours %2 minutes \nremaining")).arg(hour).arg(minute);
                        }
                    }
                }
            }
            break;
        case battery_state_pending_charge:
            power_state_text = QString(tr("Not Charging"));
            break;
        default:
            break;
    }
    mStatusLabel->setText(power_state_text);
}
