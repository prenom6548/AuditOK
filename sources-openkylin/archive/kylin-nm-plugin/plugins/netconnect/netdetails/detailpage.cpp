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
#include "detailpage.h"
#include <QPainter>
#include <QListWidget>
#include "../colorpalette.h"

#define MAX_NAME_LENGTH 32
#define MAX_LABEL_WIDTH 250
extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

DetailPage::DetailPage(bool isPppoee, bool isOobe, QWidget *parent)
    : m_isPppoe(isPppoee), m_isOobe(isOobe), QFrame(parent)
{
    this->setFrameShape(QFrame::Shape::StyledPanel);
    this->setMaximumWidth(960);
    initUI();
}

void DetailPage::setSSID(const QString &ssid) {
    this->mSSIDLabel->setText(ssid);
}

void DetailPage::setProtocol(const QString &protocol) {
    this->mProtocol->setText(protocol);
}

void DetailPage::setBandWidth(const QString &brandWidth) {
    this->mBandWidth->setText(brandWidth);
}

void DetailPage::setIpv4(const QString &ipv4) {
    this->mIPV4->setText(ipv4);
}

void DetailPage::setIpv4Dns(const QString &ipv4Dns) {
    this->mIPV4Dns->setText(ipv4Dns);
}

void DetailPage::setIpv6(const QString &ipv6) {
    this->mIPV6->setText(ipv6);
}

void DetailPage::setMac(const QString &mac) {
    this->mMac->setText(mac);
}

void DetailPage::addDetailItem(QListWidget *listWidget, DetailWidget *detailWidget)
{
    QListWidgetItem *listWidgetItem = new QListWidgetItem(listWidget);
    listWidgetItem->setSizeHint(QSize(listWidget->width(),36));
    listWidgetItem->setFlags(Qt::NoItemFlags);
    listWidget->addItem(listWidgetItem);
    listWidget->setItemWidget(listWidgetItem, detailWidget);
}

void DetailPage::initUI() {
    layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);

    QWidget *mDetailFrame = new QFrame(this);
    mDetailLayout = new QVBoxLayout(mDetailFrame);
    mDetailLayout->setContentsMargins(0,0,0,0);

    m_listWidget = new QListWidget(mDetailFrame);
    m_listWidget->setFrameShape(QFrame::Shape::NoFrame);
    m_listWidget->setBackgroundRole(QPalette::Base);
    m_listWidget->setFocusPolicy(Qt::FocusPolicy::NoFocus);
    
    QPalette standardPalette1 = getDefaultPalette(true);
    QPalette listPalette = m_listWidget->palette();
    listPalette.setColor(QPalette::Base, standardPalette1.color(QPalette::Base)); // 使用标准色板中的基础色
    listPalette.setColor(QPalette::AlternateBase, standardPalette1.color(QPalette::AlternateBase)); // 使用标准色板中的交替基色
    listPalette.setColor(QPalette::Text, standardPalette1.color(QPalette::Text)); // 使用标准色板中的文本色
    listPalette.setColor(QPalette::Highlight, standardPalette1.color(QPalette::Highlight)); // 使用标准色板中的高亮色
    listPalette.setColor(QPalette::HighlightedText, standardPalette1.color(QPalette::HighlightedText)); // 使用标准色板中的高亮文本色
    m_listWidget->setPalette(listPalette);
    m_listWidget->setAutoFillBackground(true);
    
    mDetailFrame->setAutoFillBackground(true);
    QPalette framePalette1 = mDetailFrame->palette();
    framePalette1.setColor(QPalette::Window, standardPalette1.color(QPalette::Window)); // 使用标准色板中的窗口色
    mDetailFrame->setPalette(framePalette1);

    mDetailFrame->setAutoFillBackground(true);
    QPalette framePalette = mDetailFrame->palette();
    framePalette.setColor(QPalette::Window, QColor("#FFFFFF"));
    mDetailFrame->setPalette(framePalette);

    mDetailLayout->addWidget(m_listWidget);

    mSSIDLabel = new FixLabel(this);
    mSSIDLabel->setFixedWidth(MAX_LABEL_WIDTH);
    mSSIDLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_ssidWidget = new DetailWidget(qobject_cast<QWidget *>(mSSIDLabel), m_isOobe, m_listWidget);

    //        mSSID->setStyleSheet("background:transparent;border-width:0px;border-style:none");
    //        mSSID->setFocusPolicy(Qt::NoFocus);
    m_ssidWidget->setKey(tr("SSID:"));

    mProtocol = new QLabel(this);
    m_protocolWidget = new DetailWidget(qobject_cast<QWidget *>(mProtocol), m_isOobe, m_listWidget);
    m_protocolWidget->setKey(tr("Protocol:"));

    mBandWidth = new QLabel(this);
    m_bandwidthWidget = new DetailWidget(qobject_cast<QWidget *>(mBandWidth), m_isOobe, m_listWidget);
    m_bandwidthWidget->setKey(tr("BandWidth:"));

    mIPV4 = new QLabel(this);
    m_ipv4Widget = new DetailWidget(qobject_cast<QWidget *>(mIPV4), m_isOobe, m_listWidget);
    m_ipv4Widget->setKey(tr("IPV4:"));

    mIPV4Dns = new QLabel(this);
    m_ipv4DnsWidget = new DetailWidget(qobject_cast<QWidget *>(mIPV4Dns), m_isOobe, m_listWidget);
    m_ipv4DnsWidget->setKey(tr("IPV4 Dns:"));

    if (!m_isPppoe) {
        mIPV6 = new FixLabel(this);
        mIPV6->setFixedWidth(MAX_LABEL_WIDTH);
        mIPV6->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        m_ipv6Widget = new DetailWidget(qobject_cast<QWidget *>(mIPV6), m_isOobe, m_listWidget);
        m_ipv6Widget->setKey(tr("IPV6:"));
    }

    mMac = new QLabel(this);
    m_macWidget = new DetailWidget(qobject_cast<QWidget *>(mMac), m_isOobe, m_listWidget);
    m_macWidget->setKey(tr("Mac:"));

    this->addDetailItem(m_listWidget, m_ssidWidget);
    this->addDetailItem(m_listWidget, m_protocolWidget);
    this->addDetailItem(m_listWidget, m_bandwidthWidget);
    if (!m_isPppoe) {
        this->addDetailItem(m_listWidget, m_ipv6Widget);
    }
    this->addDetailItem(m_listWidget, m_ipv4Widget);
    this->addDetailItem(m_listWidget, m_ipv4DnsWidget);
    this->addDetailItem(m_listWidget, m_macWidget);


    layout->addWidget(mDetailFrame);
}

void DetailPage::setEnableOfSaveBtn() {
    bool saveEnable = true;
    Q_EMIT setDetailPageState(saveEnable);
}
