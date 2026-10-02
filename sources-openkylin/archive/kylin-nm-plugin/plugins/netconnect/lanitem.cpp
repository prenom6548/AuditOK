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

#include "lanitem.h"
#include <QPainterPath>
#define FRAME_SPEED 150
#define LIMIT_TIME 60*1000
#define TOTAL_PAGE 8
#define RADIUS 6.0
#define LOADIMG_SIZE 16,16

LanItem::LanItem(bool isSimple, bool isOobe, QWidget *parent)
    : QFrame(parent),isSimple(isSimple), m_isOobe(isOobe)
{
    this->setFixedSize(404, 48);
    QHBoxLayout *mLanLyt = new QHBoxLayout(this);
    mLanLyt->setContentsMargins(16,0,0,0);
    mLanLyt->setSpacing(0);
    radioBtn = new RadioItemButton(this);
    
    // 在OOBE模式下，确保图标以白色显示以获得更好的对比度
    if (isOobe) {
        radioBtn->setOobe(true);
    }
    
    titileLabel = new FixLabel(this);
    titileLabel->setMaximumWidth(MAX_WIDTH_DEACTIVATED);
    m_lbLoadUp = new QLabel(this);
    m_lbLoadDown = new QLabel(this);
    m_lbLoadDownImg = new QLabel(this);
    m_lbLoadUpImg = new QLabel(this);
    m_lbLoadUp->hide();
    m_lbLoadDown->hide();
    m_lbLoadDownImg->hide();
    m_lbLoadUpImg->hide();
    QFont font;
    font.setPointSize(10);
    m_lbLoadUp->setFont(font);
    m_lbLoadDown->setFont(font);
    m_lbLoadUp->setText("0KB/s");
    m_lbLoadDown->setText("0KB/s");
    m_lbLoadDownImg->setPixmap(QPixmap(QLatin1String("://res/icon/load-down.png")));
    m_lbLoadUpImg->setPixmap(QPixmap(QLatin1String("://res/icon/load-up.png")));
    m_lbLoadDownImg->setFixedSize(LOADIMG_SIZE);
    m_lbLoadUpImg->setFixedSize(LOADIMG_SIZE);

    mLanLyt->addWidget(radioBtn);
    mLanLyt->addSpacing(10);
    mLanLyt->addWidget(titileLabel,Qt::AlignLeft);
    mLanLyt->addStretch();
    mLanLyt->addSpacing(8);
    mLanLyt->addWidget(m_lbLoadUpImg);
    mLanLyt->addWidget(m_lbLoadUp);
    mLanLyt->addSpacing(2);
    mLanLyt->addWidget(m_lbLoadDownImg);
    mLanLyt->addWidget(m_lbLoadDown);

   if (!isSimple) {
        infoLabel = new InfoButton(this);
        infoLabel->setIsOobe(m_isOobe);  // 设置是否为OOBE模式
        mLanLyt->addSpacing(8);
        mLanLyt->addWidget(infoLabel);
        connect(infoLabel, &InfoButton::released, this, [&](){
            Q_EMIT infoButtonClick(m_uuid);
        });
   }

    loadIcons.append(QIcon::fromTheme("ukui-loading-1-symbolic"));
    loadIcons.append(QIcon::fromTheme("ukui-loading-2-symbolic"));
    loadIcons.append(QIcon::fromTheme("ukui-loading-3-symbolic"));
    loadIcons.append(QIcon::fromTheme("ukui-loading-4-symbolic"));
    loadIcons.append(QIcon::fromTheme("ukui-loading-5-symbolic"));
    loadIcons.append(QIcon::fromTheme("ukui-loading-6-symbolic"));
    loadIcons.append(QIcon::fromTheme("ukui-loading-7-symbolic"));
    waitTimer = new QTimer(this);
    connect(waitTimer, &QTimer::timeout, this, &LanItem::updateIcon);

    m_menu = new QMenu(this);//右键菜单
    m_menu->setWindowFlag(Qt::X11BypassWindowManagerHint);
    connect(m_menu, &QMenu::triggered, this, &LanItem::itemClick);
}

void LanItem::updateIcon()
{
    if (currentIconIndex > 6) {
        currentIconIndex = 0;
    }
    radioBtn->setButtonIcon(loadIcons.at(currentIconIndex));
    currentIconIndex ++;
}

void LanItem::startLoading()
{
    waitTimer->start(FRAME_SPEED);
    m_loading = true;
}

void LanItem::stopLoading(){
    waitTimer->stop();
    m_loading = false;
}

void LanItem::mousePressEvent(QMouseEvent *event)
{
    return QFrame::mousePressEvent(event);
}
void LanItem::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton) {
        if (!m_menu) {
            return QFrame::mouseReleaseEvent(event);
        }

        m_menu->clear();
        if (m_isAcitve || m_loading) {
            m_menu->addAction(new QAction(tr("Disconnect"), this));
        } else if (!m_isAcitve && !m_loading) {
            m_menu->addAction(new QAction(tr("Connect"), this));
        }
        m_menu->move(cursor().pos());
        m_menu->show();

    } else {
        if (!m_isAcitve && !m_loading) {
           Q_EMIT itemClick();
        }
    }
    return QFrame::mouseReleaseEvent(event);
}
void LanItem::enterEvent(QEvent *event)
{
    m_isIn = true;
    update();
    return QFrame::enterEvent(event);
}
void LanItem::leaveEvent(QEvent *event)
{
    m_isIn = false;
    update();
    return QFrame::leaveEvent(event);
}
void LanItem::paintEvent(QPaintEvent *event)
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
