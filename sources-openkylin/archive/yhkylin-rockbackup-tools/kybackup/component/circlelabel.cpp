

/*
*  Copyright (c) KylinSoft Co., Ltd. [Year].All rights reserved.
*  yhkylin-backup-tools is licensed under GPL-2+
*  See LICENSE file for more details.
*/

#include "circlelabel.h"
#include <QPainter>
#include <QBrush>
#include <QPalette>
#include "../globalbackupinfo.h"
#include "../gsettingswrapper.h"

CircleLable::CircleLable(const QString& text, QWidget* parent /*= nullptr*/, int size /*= 24*/, QColor backgroundColor /*= QColor(0xCC, 0xCC, 0xCC)*/) :
    QLabel(parent),
    m_text(text),
    m_textColor(Qt::white)
{
    m_oldTextColor = m_textColor;
    setFixedSize(QSize(size, size));
    setBackgroundColor(backgroundColor);
    connect(GlobelBackupInfo::instance().getGlobalSignals(), &GlobalSignals::styleNameChanged, this, [=](bool isDark) {
        if (!m_bAutoTheme) {
            if (isDark) {
                this->setBackgroundColor(Qt::white);
            } else {
                this->setBackgroundColor(Qt::black);
            }
        }
    });
}

CircleLable::~CircleLable()
{}

/**
 * @brief 设置圆形label的背景色
 * @param backgroundColor 背景色，建议使用Qt::black、Qt::white等这种颜色表示法
 */
void CircleLable::setBackgroundColor(QColor backgroundColor)
{
    // 只有黑白两色不自动跟随主题，需手动设置背景色
    if (backgroundColor == QColor(Qt::black)) {
        m_bAutoTheme = false;
        if (g_GSettingWrapper.isDarkTheme()) {
            backgroundColor = QColor(Qt::white);
        }
    } else if (backgroundColor == QColor(Qt::white)) {
        m_bAutoTheme = false;
        if (!g_GSettingWrapper.isDarkTheme()) {
            backgroundColor = QColor(Qt::black);
        }
    } else {
        m_bAutoTheme = true;
    }

    m_backgroundColor = backgroundColor;
    repaint();
}

void CircleLable::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    // 深浅主题切换
    if (g_GSettingWrapper.isDarkTheme()) {
        if (m_backgroundColor == QColor(COLOR_GRAY)) {
            m_backgroundColor = QColor(COLOR_BLACK_GRAY);
            m_textColor = QColor(COLOR_DISABLE_GRAY);
        } else if (m_backgroundColor == QColor(COLOR_LIGHT_DARK)) {
            m_backgroundColor = QColor(COLOR_DARK_GRAY);
            m_textColor = QColor(COLOR_DISABLE_GRAY);
        }
    } else {
        if (m_backgroundColor == QColor(COLOR_BLACK_GRAY)) {
            m_backgroundColor = QColor(COLOR_GRAY);
            m_textColor = m_oldTextColor;
        } else if (m_backgroundColor == QColor(COLOR_DARK_GRAY)) {
            m_backgroundColor = QColor(COLOR_LIGHT_DARK);
            m_textColor = m_oldTextColor;
        }
    }

    // 如果设置的是蓝色，则使用调色板里的高亮色
    if (m_backgroundColor == QColor(COLOR_BLUE)) {
        painter.setBrush(QBrush(this->palette().highlight().color()));
    } else {
        painter.setBrush(QBrush(m_backgroundColor));
    }
    painter.setPen(Qt::NoPen);
    QRect rect = this->rect();
    // 也可用QPainterPath 绘制代替 painter.drawRoundedRect(rect, 15, 15); { QPainterPath painterPath; painterPath.addRoundedRect(rect, 15, 15); p.drawPath(painterPath); }
    painter.drawRoundedRect(rect, rect.width()/2, rect.width()/2);

    // drawText
    rect.setHeight(rect.height() - 2);
    painter.setPen(m_textColor);
    painter.drawText(rect, Qt::AlignCenter, m_text);

    QLabel::paintEvent(event);
}
