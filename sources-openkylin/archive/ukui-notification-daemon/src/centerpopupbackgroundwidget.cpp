/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "centerpopupbackgroundwidget.h"
#include <QScreen>
#include <QStyleOption>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>

CenterPopupBackgroundWidget::CenterPopupBackgroundWidget(QWidget *parent)
    :QWidget(parent)
{
    setWidgetAttribute();
    m_sreenInfo = new adaptScreenInfo(this);
    connect(m_sreenInfo, &adaptScreenInfo::updataCenterWidgetSite, this,
            &CenterPopupBackgroundWidget::updataCenterWidgetBackgroundSite);
    systemThemeChanges();
    updataCenterWidgetBackgroundSite();
}

/* 设置窗口属性 */
void CenterPopupBackgroundWidget::setWidgetAttribute()
{
    setWindowFlags(Qt::FramelessWindowHint |
                   Qt::X11BypassWindowManagerHint); //无边框、禁止拖动、禁止改变大小、不受窗管管理
    setAttribute(Qt::WA_TranslucentBackground);                    //透明背景
    setAttribute(Qt::WA_AlwaysShowToolTips);
    setProperty("useSystemStyleBlur",true);
    return;
}

void CenterPopupBackgroundWidget::updataCenterWidgetBackgroundSite()
{
    m_screenXGeometry = m_sreenInfo->m_nScreen_x;
    m_screenYGeometry = m_sreenInfo->m_nScreen_y;
    m_screenWidth = m_sreenInfo->m_screenWidth;
    m_screenHeight = m_sreenInfo->m_screenHeight;
    this->setFixedSize(m_screenWidth, m_screenHeight);
    this->move(m_screenXGeometry, m_screenYGeometry);
    this->show();
}

void CenterPopupBackgroundWidget::systemThemeChanges()
{
    const QByteArray styleId(ORG_UKUI_STYLE);
    if(QGSettings::isSchemaInstalled(styleId)){
        m_themeSettings = new QGSettings(styleId);
        connect(m_themeSettings, &QGSettings::changed, this, [=] (const QString &key){
            if(key == STYLE_NAME) {
                update();
            }
        });
    }
}

void CenterPopupBackgroundWidget::paintEvent(QPaintEvent *event)
{
    //绘制背景
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    QBrush brush(QColor(165,165,165,255*0.8));
    if(m_themeSettings) {
        QString styleName = m_themeSettings->get(STYLE_NAME).toString();
        if(styleName==STYLE_NAME_KEY_DARK || styleName==STYLE_NAME_KEY_BLACK) {
            brush.setColor(QColor(15,15,15,255*0.8));
        } else if(styleName==STYLE_NAME_KEY_LIGHT || styleName==STYLE_NAME_KEY_WHITE || styleName==STYLE_NAME_KEY_DEFAULT) {
            brush.setColor(QColor(165,165,165,255*0.8));
        }
    }
    p.setBrush(brush);
    p.setOpacity(0.8);
    p.setPen(Qt::NoPen);
    QPainterPath path;
    QRect mainRect(0, 0, this->width(), this->height());
    path.addRoundedRect(mainRect, 0, 0);
    p.setRenderHint(QPainter::Antialiasing);
    p.drawRoundedRect(mainRect, 0, 0);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);

    QWidget::paintEvent(event);
}
