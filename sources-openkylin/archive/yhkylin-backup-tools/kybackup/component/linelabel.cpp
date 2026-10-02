/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "linelabel.h"
#include <QPainter>
#include <QPalette>
#include "../globalbackupinfo.h"
#include "../gsettingswrapper.h"

#define LOCAL_COLOR_DARK_GRAY "#2D2E32"

LineLabel::LineLabel(QWidget* parent /*= nullptr*/, QColor color /*= QColor(0xCC, 0xCC, 0xCC)*/) :
    QLabel(parent),
    m_color(color)
{
    setMaximumHeight(30);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

LineLabel::~LineLabel()
{}

void LineLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.save();
    // 深浅主题切换
    if (g_GSettingWrapper.isDarkTheme()) {
        if (m_color == QColor(COLOR_GRAY)) {
            m_color = QColor(LOCAL_COLOR_DARK_GRAY);
        }
    } else {
        if (m_color == QColor(LOCAL_COLOR_DARK_GRAY)) {
            m_color = QColor(COLOR_GRAY);
        }
    }
    QPen pen = painter.pen();
    // 如果设置的是蓝色，则使用调色板里的高亮色
    if (m_color == QColor(COLOR_BLUE)) {
        pen.setColor(this->palette().highlight().color());
    } else {
        pen.setColor(m_color);
    }
    pen.setWidth(2);
    painter.setPen(pen);
    QRect rect = this->rect();
    painter.drawLine(0, rect.height()/2, rect.width(), rect.height()/2);

    painter.restore();
    QLabel::paintEvent(event);
}
