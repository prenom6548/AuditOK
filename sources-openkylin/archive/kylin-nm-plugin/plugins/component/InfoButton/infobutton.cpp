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

#include "infobutton.h"
#include <QEvent>
#include <QPainter>
#include <QApplication>
#include <QDebug>
#include <QPainterPath>

#define BUTTON_SIZE 36,36
#define ICON_SIZE 16,16
#define BACKGROUND_COLOR QColor(0,0,0,0)
#define OUTER_PATH 8,8,16,16
#define INNER_PATH 9,9,14,14
#define TEXT_POS 14,5,16,16,0

#define THEME_SCHAME "org.ukui.style"
#define COLOR_THEME "styleName"

InfoButton::InfoButton(QWidget *parent) : QPushButton(parent)
{
    this->setFixedSize(BUTTON_SIZE);
    initUI();
    const QByteArray style_id(THEME_SCHAME);
    if (QGSettings::isSchemaInstalled(style_id)) {
        m_styleGsettings = new QGSettings(style_id);
        connect(m_styleGsettings, &QGSettings::changed, this, &InfoButton::onGSettingChaned);
    } else {
        qDebug() << "Gsettings interface \"org.ukui.style\" is not exist!";
    }
}

void InfoButton::updateForegroundColor()
{
    // 在OOBE模式下始终显示为黑色以确保在白色背景下清晰可见
    if (m_isOobe) {
        m_foregroundColor =  Qt::white;
    } else {
        // 其他模式下使用默认的文本颜色
        m_foregroundColor = qApp->palette().text().color();
    }
}

void InfoButton::initUI()
{
    this->setFixedSize(BUTTON_SIZE);
    m_backgroundColor = BACKGROUND_COLOR;
    // 初始化时根据当前背景和插件类型来设置前景色
    updateForegroundColor();
}

void InfoButton::onGSettingChaned(const QString &key)
{
    if (key == COLOR_THEME) {
        updateForegroundColor();
        this->repaint();
    }
}

void InfoButton::paintEvent(QPaintEvent *event)
{
    // 动态更新前景色以适应背景
    updateForegroundColor();
    
    QPalette pal = this->palette();
    pal.setColor(QPalette::Base, m_backgroundColor);
    pal.setColor(QPalette::Text, m_foregroundColor);

    QPainterPath cPath;
    cPath.addRect(0, 0, ICON_SIZE);
    cPath.addEllipse(0, 0, ICON_SIZE);

    QPainterPath outerPath;
    outerPath.addEllipse(OUTER_PATH);

    QPainterPath innerPath;
    innerPath.addEllipse(INNER_PATH);
    outerPath -= innerPath;

    QPainter painter(this);
    painter.setRenderHint(QPainter:: Antialiasing, true);  //设置渲染,启动反锯齿
    painter.setPen(Qt::NoPen);

    painter.setBrush(pal.color(QPalette::Base));
    painter.drawPath(cPath);

    painter.fillPath(outerPath, pal.color(QPalette::Text));
    painter.setPen(m_foregroundColor);
    QFont font("Noto Sans CJK SC", 11, QFont::Normal, false);
    painter.setFont(font);
    painter.drawText(TEXT_POS, "i");
}

void InfoButton::enterEvent(QEvent *event)
{
    if (m_isOobe) {
        m_foregroundColor = Qt::white;
    } else {
        m_foregroundColor = QColor(55, 144, 250, 255); // 默认蓝色
    }
    this->repaint();
    return QPushButton::enterEvent(event);
}

void InfoButton::leaveEvent(QEvent *event)
{
    // 离开时恢复基于背景色的自适应颜色
    updateForegroundColor();
    this->repaint();
    return QPushButton::leaveEvent(event);
}

void InfoButton::mousePressEvent(QMouseEvent *event)
{
    if (m_isOobe) {
        m_foregroundColor = Qt::white;
    } else {
        m_foregroundColor = QColor(36, 109, 212, 255); // 默认深蓝色
    }
    this->repaint();
    return QPushButton::mousePressEvent(event);
}

void InfoButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_isOobe) {
        m_foregroundColor = Qt::white; // 在OOBE模式下恢复白色
    } else {
        m_foregroundColor = QColor(55, 144, 250, 255); // 默认蓝色
    }
    this->repaint();
    return QPushButton::mouseReleaseEvent(event);
}