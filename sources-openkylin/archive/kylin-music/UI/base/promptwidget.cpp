/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#include <QGuiApplication>
#include <QScreen>
#include <QRect>
#include <QPainter>
#include "UI/base/xatom-helper.h"
#include "UI/base/widgetstyle.h"


PromptWidget::PromptWidget(QWidget *parent) : QWidget(parent)
{
    this->setAutoFillBackground(true);
    this->setBackgroundRole(QPalette::Base);

    m_radius = 8;

    initUi();

    MotifWmHints hintv;
    hintv.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
    hintv.functions = MWM_FUNC_ALL;
    hintv.decorations = MWM_DECOR_BORDER;
    XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hintv);
}

void PromptWidget::initUi()
{
    this->setFixedSize(134, 40);

    HLayout = new QHBoxLayout();
    promptLabel = new QLabel(this);
    promptLabel->setAccessibleName("kylin-music_QLabel_promptLabel");
    promptLabel->setText("歌曲已存在");
    promptLabel->setFixedSize(80, 30);
    promptLabel->setAlignment(Qt::AlignCenter);

    iconLabel = new QLabel(this);
    iconLabel->setAccessibleName("kylin-music_QLabel_iconLabel");
    iconLabel->setFixedSize(30, 30);
    QPixmap pixmap(":/img/default/ukui-dialog-warning-24.svg"); // 从文件加载图片
    iconLabel->setPixmap(pixmap.scaled(iconLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation)); // 设置label的pixmap并填充整个label

    HLayout->addWidget(iconLabel, 0);
    HLayout->addWidget(promptLabel, Qt::AlignRight);

    HLayout->setMargin(0);
    HLayout->setSpacing(0);
    HLayout->setAlignment(Qt::AlignCenter);

    this->setLayout(HLayout);

    int sw = QGuiApplication::primaryScreen()->availableGeometry().width();
    int sh = QGuiApplication::primaryScreen()->availableGeometry().height();
    this->move((sw-this->width())/2, (sh-this->height())/2);

    initColor();
}
#ifndef WHITE_TEST
bool PromptWidget::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(result);

    if(eventType != "xcb_generic_event_t") {
        return false;
    }

    xcb_generic_event_t *event = (xcb_generic_event_t*)message;

    switch (event->response_type & ~0x80) {
        case XCB_FOCUS_OUT:
            QRect rect(volunmPosX, volunmPosY, volunmPosWidth, volunmPosHeight);
            if(rect.contains(QCursor::pos(), false)) {
                return 0;
            } else {
                this->hide();
                break;
            }
    }

    return false;
}

void PromptWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);  // 抗锯齿;
    QRect rect = this->rect();
    painter.setBrush(QBrush(m_color));
    painter.setPen(Qt::transparent);
    painter.drawRoundedRect(rect, m_radius, m_radius);

    QWidget::paintEvent(event);
}
#endif
void PromptWidget::initColor()
{
    if (WidgetStyle::themeColor == 1) {
        m_color = QColor(38, 38, 38, 240);

    } else {
        m_color = QColor(249, 249, 249, 240);
    }
}
