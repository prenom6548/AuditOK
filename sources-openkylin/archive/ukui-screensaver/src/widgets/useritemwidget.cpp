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
#include "useritemwidget.h"

#include <QPainter>
#include <QPixmap>
#include <QStyleOption>
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QEvent>
#include "commonfunc.h"
#include "customlabel.h"

UserItemWidget::UserItemWidget(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    init();
    initIconLabel();
}

UserItemWidget::~UserItemWidget() {}

bool UserItemWidget::eventFilter(QObject *obj, QEvent *event)
{
    return QWidget::eventFilter(obj, event);
}

void UserItemWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void UserItemWidget::init()
{
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(4);
    m_labelHeadImg = new QLabel(this);
    m_labelNickName = new CustomLabel(this);
    m_labelNickName->setStyleSheet("color: white");
    m_labelNickName->setFixedSize(364, 32);
    layout->addWidget(m_labelHeadImg);
    layout->addWidget(m_labelNickName);
    setLayout(layout);
}

void UserItemWidget::initIconLabel()
{
    m_labelHeadImg->setFixedSize(32, 32);
    QVBoxLayout *layoutImg = new QVBoxLayout(m_labelHeadImg);
    layoutImg->setContentsMargins(18, 0, 0, 18);
    layoutImg->setAlignment(Qt::AlignCenter);
    m_labelLoggedIn = new QLabel(this);
    layoutImg->addWidget(m_labelLoggedIn);
}

void UserItemWidget::setUserName(QString name)
{
    m_strUserName = name;
}

QString UserItemWidget::getUserName()
{
    return m_strUserName;
}

void UserItemWidget::setUserNickName(QString strNickName)
{
    m_strNickName = strNickName;
    m_labelNickName->setText(strNickName);
}

void UserItemWidget::setUserPixmap(QPixmap userIcon)
{
    userIcon = scaledPixmap(userIcon);
    userIcon = userIcon.scaled(32, 32, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    userIcon = PixmapToRound(userIcon, 16);
    m_labelHeadImg->setPixmap(userIcon);
}

void UserItemWidget::setUserStatus(bool status)
{
    if (status) {
        QPixmap status(":/image/assets/selected.svg");
        status = scaledPixmap(status);
        status = status.scaled(14, 14, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        status = PixmapToRound(status, 7);
        m_labelLoggedIn->setPixmap(status);
    }
}

void UserItemWidget::setFontFamily(QString fontFamily)
{
    m_labelNickName->setFontFamily(fontFamily);
}

void UserItemWidget::setFontSize(double fontSize)
{
    m_labelNickName->setFontSize(fontSize);
}
