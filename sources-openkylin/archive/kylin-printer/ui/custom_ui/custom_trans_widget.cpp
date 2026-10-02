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
#include "custom_trans_widget.h"

#include <QPainter>
#include <QPainterPath>
#include <QStyleOption>

#include "common.h"
#include "gsettings_monitor.h"

CustomTransWidget::CustomTransWidget(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_AlwaysShowToolTips);
    this->setProperty("useSystemStyleBlur",true);
    this->setAttribute(Qt::WA_TranslucentBackground,true);
    m_tran = GsettingsMonitor::getInstance()->sysTransparencyGet();
    this->initConnect();
}

CustomTransWidget::~CustomTransWidget()
{
}

void CustomTransWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    // p.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect(), 0, 0); // 左上右下
//    double tran=m_pGsettingControlTrans->get("transparency").toDouble()*255;
    QStyleOption opt;
    opt.init(this);

    QColor mainColor;

    if (QColor(255,255,255) == opt.palette.color(QPalette::Base) || QColor(248,248,248) == opt.palette.color(QPalette::Base)) {
        mainColor = QColor(242, 242, 242, m_tran);
    } else {
        mainColor = QColor(20, 20, 20, m_tran);
    }

    p.fillPath(rectPath,QBrush(mainColor));
    QWidget::paintEvent(event);
}

void CustomTransWidget::transChange(int transparency)
{
    m_tran = transparency * 2.55;
    this->update();
}

//读取主题配置文件
void CustomTransWidget::initConnect()
{
    connect(GsettingsMonitor::getInstance(), &GsettingsMonitor::sysTransparencyUpdate,
            this,                            &CustomTransWidget::transChange);
}