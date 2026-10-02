/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
#include "promptwidget.h"
#include <QDebug>
#define UKUI_STYLE "org.ukui.style"
#define UKUI_FONT_SIZE "systemFontSize"
PromptWidget::PromptWidget(QWidget *parent) : QWidget(parent)
{
    // 设置白色长条形状，宽度与主窗口一致(900px)，高度50px
    this->setFixedSize(900, 50);

    // 创建水平布局，包含图标和标签
    m_networkErrorLayout = new QHBoxLayout(this);
    m_networkErrorLayout->setContentsMargins(20, 10, 20, 10); // 设置边距
    m_networkErrorLayout->setSpacing(5); // 设置图标和文字间距

    // 创建图标标签
    m_networkWarnIcon = new QPushButton();
    m_networkWarnIcon->setAccessibleName("kylin-weather_QPushButton_PromptWidget_networkWarnIcon");
    m_networkWarnIcon->setFlat(true);
    m_networkWarnIcon->setFocusPolicy(Qt::NoFocus);
    m_networkWarnIcon->setFixedSize(24, 24); // 设置图标大小
    m_networkWarnIcon->setIcon(QIcon::fromTheme("dialog-warning"));

    // 创建错误信息标签
    m_networkErrorCode = new QLabel;
    m_networkErrorCode->setAccessibleName("kylin-weather_QLabel_PromptWidget_networkErrorCode");
    m_networkErrorCode->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_networkErrorCode->setStyleSheet("font-size: 12px;"); // 设置文字样式

    addItems();
    initConnect();
}

void PromptWidget::initConnect()
{
    connect(ViewVar::getInstance(), &ViewVar::setFontSizeSignal, this, &PromptWidget::lable2SetFontSizeSlot);
}


void PromptWidget::isFullScreen(int m_isFullScreen)
{
    m_judgeFullScreen = m_isFullScreen;
    // 长条形状不需要根据全屏状态改变布局
}

void PromptWidget::lable2SetFontSizeSlot(int size)
{
    // 长条形状不需要根据字体大小改变布局
    Q_UNUSED(size);
}

void PromptWidget::initLayout(QString netError, int m_isFullScreen)
{    
    // 设置错误信息文本
    m_networkErrorCode->setText(netError);
}

void PromptWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing); // 反锯齿
    
    // 获取并设置背景
    QBrush brush;
    if (qApp && qApp->property("kcontain-secondary-normal").isValid() && qApp->property("kcontain-secondary-normal").canConvert<QBrush>()){
        brush = qApp->property("kcontain-secondary-normal").value<QBrush>();
    } else {
        QColor backgroundCorlor = qApp->palette().color(QPalette::Base);
        brush = QBrush(backgroundCorlor);
    }

    painter.setBrush(brush);
    painter.setPen(Qt::NoPen);
    
    // 绘制圆角矩形背景
    int radius = 0; // 圆角半径
    QRect rect = this->rect();
    painter.drawRoundedRect(rect, radius, radius);
}

void PromptWidget::addItems()
{
    // 添加图标
    m_networkErrorLayout->addWidget(m_networkWarnIcon);
    
    // 添加错误信息标签
    m_networkErrorLayout->addWidget(m_networkErrorCode);
    
    // 添加弹性空间，让内容左对齐
    m_networkErrorLayout->addStretch();
    
    // 设置布局
    this->setLayout(m_networkErrorLayout);
}
