/*
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
 *
 */

#include "clickLabel.h"
#include <QGraphicsOpacityEffect>
#define THEME_UKFD_TRANS "org.ukui.control-center.personalise"

ClickLabel::ClickLabel(QWidget *parent)
{
    initOpacityGSettings();
}

ClickLabel::~ClickLabel()
{
}

void ClickLabel::mousePressEvent(QMouseEvent *event){
    if (event->button() == Qt::LeftButton)
        Q_EMIT clicked();
}

void ClickLabel::initOpacityGSettings()
{
    const QByteArray idtrans(THEME_UKFD_TRANS);
    if(QGSettings::isSchemaInstalled(idtrans)) {
        m_gsTransOpacity = new QGSettings(idtrans);
    }

    if (!m_gsTransOpacity) {
        m_curTransOpacity = 1;
        return;
    }

    connect(m_gsTransOpacity, &QGSettings::changed, this, [=](const QString &key) {
        if (key == "transparency") {
            QStringList keys = m_gsTransOpacity->keys();
            if (keys.contains("transparency")) {
                m_curTransOpacity = m_gsTransOpacity->get("transparency").toString().toDouble();
                repaint();
            }
        }
    });

    QStringList keys = m_gsTransOpacity->keys();
    if(keys.contains("transparency")) {
        m_curTransOpacity = m_gsTransOpacity->get("transparency").toString().toDouble();
    }
    QGraphicsOpacityEffect *opacityEffect=new QGraphicsOpacityEffect;
    this->setGraphicsEffect(opacityEffect);

    opacityEffect->setOpacity(m_curTransOpacity);
}

void ClickLabel::setFullOpacity()
{
    QGraphicsOpacityEffect *opacityEffect=new QGraphicsOpacityEffect;
    this->setGraphicsEffect(opacityEffect);

    m_curTransOpacity = 1;
    opacityEffect->setOpacity(m_curTransOpacity);

}

//void ClickLabel::paintEvent(QPaintEvent *event){
////    Q_UNUSED(event)
////    QStyleOption opt;
////    opt.init(this);
////    QPainter p(this);
////    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
//}
