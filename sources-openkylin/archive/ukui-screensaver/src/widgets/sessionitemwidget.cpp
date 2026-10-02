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
#include "sessionitemwidget.h"

#include <QPainter>
#include <QPixmap>
#include <QStyleOption>
#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QEvent>
#include <QFile>
#include "commonfunc.h"

#define PREFIX QString(RESOURCE_PATH)
#define IMAGE_DIR PREFIX + "/images/"

SessionItemWidget::SessionItemWidget(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    init();
    initIconLabel();
}

SessionItemWidget::~SessionItemWidget() {}

bool SessionItemWidget::eventFilter(QObject *obj, QEvent *event)
{
    return QWidget::eventFilter(obj, event);
}

void SessionItemWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void SessionItemWidget::init()
{
    QHBoxLayout *layout = new QHBoxLayout();
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setSpacing(4);
    m_labelImg = new QLabel(this);
    m_labelName = new QLabel(this);
    m_labelName->setStyleSheet("color: white");
    m_labelName->setFixedSize(364, 32);
    layout->addWidget(m_labelImg);
    layout->addWidget(m_labelName);
    setLayout(layout);
}

void SessionItemWidget::initIconLabel()
{
    m_labelImg->setFixedSize(32, 32);
}

void SessionItemWidget::setSessionName(QString name)
{
    QString sessionPrefix = name.left(name.indexOf('-'));
    if (name == "ukui-wayland")
        sessionPrefix = "ukui_wayland";
    if (name == "kylin-wlcom")
        sessionPrefix = "kylin_wlcom";
    m_strImgPath = IMAGE_DIR + QString("badges/%1_badge.svg").arg(sessionPrefix.toLower());
    QFile iconFile(m_strImgPath);
    if (!iconFile.exists()) {
        m_strImgPath = IMAGE_DIR + QString("badges/unknown_badge.svg");
    }
    qDebug() << "setSessionName icon:" << m_strImgPath;
    QPixmap ssIcon(m_strImgPath);
    ssIcon = scaledPixmap(ssIcon);
    ssIcon = ssIcon.scaled(32, 32, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    ssIcon = PixmapToRound(ssIcon, 16);
    m_labelImg->setPixmap(ssIcon);
    m_strSessionName = name;
    m_labelName->setText(m_strSessionName);
}

void SessionItemWidget::setSessionType(int nType)
{
    m_nSessionType = nType;
}
