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

#include <QEvent>
#include <QDebug>
#include "poweritemwidget.h"
#include "statusbutton.h"
#include "mycircel.h"

#include <kysdk/applications/accessinfohelper.h>
using namespace kdk;

PowerItemWidget::PowerItemWidget(QString powerName, QWidget *parent) : QFrame(parent), m_powerName(powerName)
{
    itemFace = new StatusButton(this, POWERBUTTON);
    KDK_EXTEND_ALL_INFO_FORMAT(itemFace, "StatusButton", "initialButton", "add button of tools");
    itemFace->setFixedSize(130, 130);
    textLabel = new CustomLabel(this);
    KDK_EXTEND_ALL_INFO_FORMAT(textLabel, "CustomLabel", "initialLabel", "add textLabel of tools");
    textLabel->setAlignment(Qt::AlignCenter);
    textLabel->setStyleSheet("QLabel {color: white}; QToolTip{color:black;}");
    textLabel->setMaximumWidth(130);
    textLabel->setFixedHeight(45);
    itemlayout = new QVBoxLayout(this);
    if (m_powerName == QStringLiteral("UpgradeThenRestart") || m_powerName == QStringLiteral("UpgradeThenShutdown")) {
        upgradeLayout = new QHBoxLayout(this);
        upgradeLayout->setContentsMargins(0, 0, 0, 0);
        MyCircel *circel = new MyCircel(10, 10, this);
	circel->setMinimumWidth(12);
        upgradeLayout->addWidget(circel);
        upgradeLayout->addWidget(textLabel);
        upgradeLayout->setContentsMargins(0, 0, 0, 0);
        textLabel->setContentsMargins(0, 0, 20, 2);
        upgradeTextLabel = new CustomLabel(this);
        upgradeTextLabel->setContentsMargins(26, 0, 0, 0);
        upgradeTextLabel->setFixedHeight(20);
    }

    if (upgradeLayout) {
        itemlayout->addSpacing(30);
        itemlayout->addWidget(itemFace);
        itemlayout->addLayout(upgradeLayout);
        itemlayout->addWidget(upgradeTextLabel);
    } else {
        itemlayout->addWidget(itemFace);
        itemlayout->addWidget(textLabel);
    }
    itemlayout->setAlignment(Qt::AlignHCenter);
}

PowerItemWidget::~PowerItemWidget() {}

bool PowerItemWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::ToolTip) {

    } else if (event->type() == QEvent::Leave) {
    }
    return QWidget::eventFilter(obj, event);
}

void PowerItemWidget::setText(QString text)
{
    textLabel->setText(text);
}

void PowerItemWidget::setToolTip(const QString tip)
{
    itemFace->setToolTip(tip);
}

void PowerItemWidget::setIcon(const QIcon &icon)
{
    itemFace->setIcon(icon);
}

void PowerItemWidget::setSelected(int status)
{
    itemFace->setClickedStatus(status);
}

void PowerItemWidget::setFontFamily(QString fontFamily)
{
    textLabel->setFontFamily(fontFamily);
}

void PowerItemWidget::setFontSize(double fontSize)
{
    textLabel->setFontSize(fontSize);
}

void PowerItemWidget::setUpgradeTimeText(QString upgradetime)
{
    QFontMetrics fontWidth(upgradeTextLabel->font());
    QString elideNote = "";

    //当更新传过来值为0时，默认设置为5分钟
    int upgradetimeInt = upgradetime.toInt();
    upgradetimeInt = upgradetimeInt == 0 ? 5 : upgradetimeInt;
    int minutes = upgradetimeInt % 60;
    int hours = upgradetimeInt / 60;
    QString minStr = QString::number(minutes);
    QString hoursStr = QString::number(hours);
    QString text = "";
    if (minutes != 0) {
        if (hours < 1) {
            //小于一小时，预计xx分钟
            //elideNote = "预计30分钟";
            text = tr("Estimated ") + minStr + tr(" minutes");
        } else {
            //大于一小时，预计xx小时xx分钟
            text = tr("Estimated ") + hoursStr + tr(" hours") + minStr + tr(" minutes");
        }
    } else {
        // 60的倍数，预计xx小时
        text = tr("Estimated ") + hoursStr + tr(" hours");
    }
    elideNote = fontWidth.elidedText(text , Qt::ElideRight, 100);
    upgradeTextLabel->setText(elideNote);
    upgradeTextLabel->setToolTip(text);
}

void PowerItemWidget::mousePressEvent(QMouseEvent *event)
{
    QRect currentIconRect = itemFace ? itemFace->geometry() : QRect();
    // 记录是否按在 widget 内
    m_pressedInWidget = rect().contains(event->pos());
    // 记录是否按在 icon 内
    m_pressedInIcon = currentIconRect.contains(event->pos());

    if (m_pressedInIcon) {
        if (itemFace) {
            itemFace->setDown(true);
        }
    }
}

void PowerItemWidget::mouseReleaseEvent(QMouseEvent *event)
{
    QRect currentIconRect = itemFace ? itemFace->geometry() : QRect();
    bool releasedInWidget = rect().contains(event->pos());
    bool releasedInIcon = currentIconRect.contains(event->pos());

    if (m_pressedInIcon && releasedInIcon) {
        Q_EMIT iconClicked();
    } else if (m_pressedInWidget && !m_pressedInIcon) {
        Q_EMIT nonIconAreaClicked();
    }

    // 重置状态
    m_pressedInIcon = false;
    m_pressedInWidget = false;

    if (itemFace) {
        itemFace->setDown(false);
    }
}
