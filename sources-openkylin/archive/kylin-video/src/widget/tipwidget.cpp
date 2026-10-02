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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */
#include "tipwidget.h"

#include <QDebug>
#include <QLabel>
#include <QTimer>
#include <QGSettings>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>

#include "global/global.h"

using namespace Global;

TipWidget::TipWidget(QWidget *parent) : QDialog(parent)
{
    w = new QWidget(this);
    w->setAccessibleName("kylin-video_QWidget_w");
    QGraphicsDropShadowEffect * effect = new QGraphicsDropShadowEffect(w);
    effect->setOffset(0, 0);//设置阴影距离
    effect->setColor(QColor(0,0,0,90));//设置阴影颜色
    effect->setBlurRadius(6);//设置阴影圆角
    w->setStyleSheet("QWidget{background-color:#FFFFFF;border-radius:6px;}");
    w->setGraphicsEffect(effect);

    setWindowFlag(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
}

TipWidget::~TipWidget()
{
    if (w) {
        delete w;
        w = nullptr;
    }
}

void TipWidget::setLightTheme()
{
    w->setStyleSheet("QWidget{background-color:#FFFFFF;border-radius:6px;}");
}

void TipWidget::setBlackTheme()
{
    w->setStyleSheet("QWidget{background-color:#262626;border-radius:6px;}");
}

void TipWidget::showTip(QString tip, int show_time, QSize size, QWidget *parent, TipWidget::TipType type)
{
    // 相对于屏幕坐标
    TipWidget tw(parent);
    tw.setModal(true);
    // tw.setFixedSize(size);

    if(g_gsettings->get("style-name").toString() == STYLE_UKUI_LIGHT)
        tw.setLightTheme();
    else
        tw.setBlackTheme();

    QWidget icon(tw.getWidget());
    icon.setFixedSize(18, 18);
    switch (type) {
    case WARNING:
        icon.setStyleSheet(QString("border-image:url(:/ico/icon-warning.png);"));
        break;
    default:
        break;
    }

    QLabel lab_text(tw.getWidget());
    lab_text.setText(tip);

    QFontMetrics fontWidth(lab_text.font());
    tw.setFixedSize(fontWidth.width(tip) + 50, fontWidth.height() + 8);

    QHBoxLayout hb;
    hb.setContentsMargins(10, 0, 0, 0);
    hb.addWidget(&icon);
    hb.addWidget(&lab_text);
    hb.addSpacing(10);
    hb.addStretch(1);
    tw.getWidget()->setLayout(&hb);

    QTimer::singleShot(show_time, [&tw](){
        tw.accept();
    });

    tw.show();
    if (parent) {
        QSize tmp_size = (parent->size() - tw.size()) / 2;
        tw.move(parent->mapToGlobal(QPoint(0, 0)) + QPoint(tmp_size.width(), tmp_size.height()));
    }
    tw.exec();
}

void TipWidget::resizeEvent(QResizeEvent *event)
{
    w->setGeometry(2, 2, size().width() - 4, size().height() - 4);
    return QDialog::resizeEvent(event);
}
