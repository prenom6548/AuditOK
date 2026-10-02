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
#include "itemframe.h"
#include <QPainter>
#include <QPainterPath>

#define LAYOUT_MARGINS 2,0,12,0
#define MAIN_LAYOUT_MARGINS 0,0,0,0
#define RADIUS 6.0

AddNetItem::AddNetItem(QWidget *parent) : QFrame(parent)
{
    this->setFixedSize(404, 48);
    QHBoxLayout *m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setContentsMargins(0,0,0,0);

    titleLabel = new QLabel(this);
    titleLabel->setText(tr("Add Others..."));

    m_mainLayout->addSpacing(16);
    m_mainLayout->addWidget(titleLabel);
    m_mainLayout->addStretch();

    this->setLayout(m_mainLayout);
}

void AddNetItem::mouseReleaseEvent(QMouseEvent *event)
{
    Q_EMIT itemClick();
    return QFrame::mouseReleaseEvent(event);
}

void AddNetItem::enterEvent(QEvent *event)
{
    m_isIn = true;
    update();
    return QFrame::enterEvent(event);
}
void AddNetItem::leaveEvent(QEvent *event)
{
    m_isIn = false;
    update();
    return QFrame::leaveEvent(event);
}
void AddNetItem::paintEvent(QPaintEvent *event)
{
    QPalette pal = this->palette();

    QPainter painter(this);
    painter.setRenderHint(QPainter:: Antialiasing, true);  //设置渲染,启动反锯齿
    painter.setPen(Qt::NoPen);
    if (m_isIn) {
        QColor color(240, 240, 240);
        color.setAlphaF(0.39);
        painter.setBrush(color);
    }
    else
        painter.setBrush(pal.color(QPalette::Base));

    QRect rect = this->rect();
    QPainterPath path;
    path.addRoundedRect(rect, RADIUS, RADIUS);
    painter.drawPath(path);
    return QFrame::paintEvent(event);
}

ItemFrame::ItemFrame(QString devName, QWidget *parent)
    : m_deviceName(devName), QFrame(parent)
{
    deviceLanLayout = new QVBoxLayout(this);
    deviceLanLayout->setContentsMargins(MAIN_LAYOUT_MARGINS);
    lanItemFrame = new QFrame(this);
    lanItemFrame->setFrameShape(QFrame::Shape::NoFrame);
    lanItemFrame->setContentsMargins(LAYOUT_MARGINS);

    lanItemLayout = new QVBoxLayout(this);
    lanItemLayout->setContentsMargins(LAYOUT_MARGINS);
    lanItemLayout->setSpacing(0);

    deviceLanLayout->setSpacing(0);
    setLayout(deviceLanLayout);
    lanItemFrame->setLayout(lanItemLayout);

    deviceFrame = new DeviceFrame(devName, this);
    m_divider = new Divider(this);
    addNetItem = new AddNetItem(this);
    deviceLanLayout->addWidget(m_divider);
    deviceLanLayout->addWidget(deviceFrame);
    deviceLanLayout->addWidget(lanItemFrame);
    deviceLanLayout->addWidget(addNetItem);

    connect(addNetItem, &AddNetItem::itemClick, this, &ItemFrame::addNetItemClick);

    m_updateNetSpeedThread = new KyNetLoadRateThread(devName);
}

ItemFrame::~ItemFrame()
{
    if (m_updateNetSpeedThread) {
        m_updateNetSpeedThread->quit();
        m_updateNetSpeedThread->wait();
        delete m_updateNetSpeedThread;
        m_updateNetSpeedThread = nullptr;
    }
}

void ItemFrame::showJoinPage(QWidget *widget, bool isOobe)
{
    if (nullptr != joinPage) {
        joinPage->show();
    } else {
        joinPage = new HiddenWiFiPage(deviceFrame->deviceLabel->text(), true, widget);
        joinPage->setOobe(isOobe);
        connect(joinPage, &HiddenWiFiPage::destroyed, [=](){
            joinPage->disconnect(this);
            joinPage = nullptr;
        });
        connect(joinPage, &HiddenWiFiPage::connectHideNormalConnect, this, &ItemFrame::connectHideNormalConnect);
        connect(joinPage, &HiddenWiFiPage::connectHideTtlsConnect, this, &ItemFrame::connectHideTtlsConnect);
        connect(joinPage, &HiddenWiFiPage::connectHidePeapConnect, this, &ItemFrame::connectHidePeapConnect);
        joinPage->show();
    }
}

void ItemFrame::startNetSpeedThread()
{
    m_updateNetSpeedThread->start();
}

void ItemFrame::stopNetSpeedThread()
{
    if (m_updateNetSpeedThread->isRunning()) {
        m_updateNetSpeedThread->quit();
    }
}

void ItemFrame::updateNetSpeed()
{
    if (!m_updateNetSpeedThread->isRunning() || lanItemLayout->count() <= 0) {
        return;
    }

    long int rcvRate;
    long int txRate;
    WlanItem *wlanItem = nullptr;
    wlanItem = (WlanItem *)lanItemLayout->itemAt(0)->widget();

    if (wlanItem->getStatus()) {
        m_updateNetSpeedThread->getNetSpeed(rcvRate, txRate);
        setNetSpeed(rcvRate, txRate, wlanItem);
    }
}

void ItemFrame::setNetSpeed(long rcvRate, long txRate, WlanItem *wlanItem)
{
    if (!wlanItem->getStatus()) {
        return;
    }

    int rcvNum = rcvRate;
    int txNum = txRate;

    QString strRcv = "0";
    QString strTx = "0";
    if (rcvNum < 1024) {
        strRcv = QString::number(rcvNum) + "KB/s";
    } else {
        int remainder;
        if (rcvNum % 1024 < 100) {
            remainder = 0;
        } else {
            remainder = (rcvNum % 1024) / 100;
        }
        strRcv = QString::number(rcvNum / 1024) + "."  + QString::number(remainder) + "MB/s";
    }

    if (txNum < 1024) {
        strTx = QString::number(txNum) + "KB/s";
    } else {
        int remainder;
        if (txNum % 1024 < 100) {
            remainder = 0;
        } else {
            remainder = (txNum % 1024)/100;
        }
        strTx = QString::number(txNum / 1024) + "."  + QString::number(remainder) + "MB/s";
    }
    wlanItem->setNetSpeed(strRcv, strTx);
}
