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
#include "detailwidget.h"
#include <QFontMetrics>
#include <QDebug>
#include "../colorpalette.h"

#define ITEM_HEIGHT 36
#define ITEM_MARGINS 18,0,16,0

#define MAX_LABEL_WIDTH 138

DetailWidget::DetailWidget(QWidget *valueWidget, bool isOobe, QWidget *parent)
    : m_valueWidget(valueWidget) , QWidget(parent), m_isOobe(isOobe)
{
//    m_valueWidget = valueWidget;
    initUI();
}

DetailWidget::~DetailWidget()
{

}

void DetailWidget::initUI()
{
    this->setFixedHeight(ITEM_HEIGHT);
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setContentsMargins(ITEM_MARGINS);

    // 不让子控件自己填充背景，让 QListWidget 绘制交替行色。
    // 之前这里设置了自填充背景和窗口色，导致每一行覆盖了 QListWidget 的交替色。
    // 改为透明背景，让父级的 Base/AlternateBase 颜色透出。
    QPalette standardPalette = getDefaultPalette(true);
    this->setAutoFillBackground(false);
    this->setAttribute(Qt::WA_NoSystemBackground, true);

    m_keyLabel = new FixLabel(this);
    m_keyLabel->setMaximumWidth(MAX_LABEL_WIDTH);
    m_keyLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    // 使用色板设置文字颜色。OOBE 模式下使用白色，否则使用标准调色板文本色。
    QPalette labelPalette = m_keyLabel->palette();
    // 始终不让标签自己填充背景，保持透明，以便显示列表的行背景
    if (m_isOobe) {
        labelPalette.setColor(QPalette::WindowText, QColor("#FFFFFF"));
        labelPalette.setColor(QPalette::Text, QColor("#FFFFFF"));
    } else {
        labelPalette.setColor(QPalette::WindowText, standardPalette.color(QPalette::WindowText));
        labelPalette.setColor(QPalette::Text, standardPalette.color(QPalette::Text));
    }
    m_keyLabel->setPalette(labelPalette);
    m_keyLabel->setAutoFillBackground(false);
    m_mainLayout->addWidget(m_keyLabel);
    m_mainLayout->addStretch();
    m_mainLayout->addWidget(m_valueWidget);
    
    // 为值部件也设置色板：值文本在 OOBE 下为白色，value 背景保持透明以显示行背景
    if (m_valueWidget) {
        QPalette valPal = m_valueWidget->palette();
        if (m_isOobe) {
            valPal.setColor(QPalette::WindowText, QColor("#FFFFFF"));
            valPal.setColor(QPalette::Text, QColor("#FFFFFF"));
        } else {
            valPal.setColor(QPalette::WindowText, standardPalette.color(QPalette::WindowText));
            valPal.setColor(QPalette::Text, standardPalette.color(QPalette::Text));
        }
        m_valueWidget->setPalette(valPal);
        // value widget 背景也保持透明以显示行背景
        m_valueWidget->setAutoFillBackground(false);
        if (auto w = qobject_cast<QWidget*>(m_valueWidget)) {
            w->setAttribute(Qt::WA_NoSystemBackground, true);
        }
    }
    
//    m_valueWidget->setMaximumWidth(100000000000);
}

void DetailWidget::setKey(const QString &keyLabel)
{
    m_keyLabel->setText(keyLabel);
}
