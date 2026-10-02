/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2024 KylinSoft  Co., Ltd.
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
#include "chartviewwidget.h"
#include <QLabel>
#include <QHBoxLayout>
#include <QApplication>

const int margin = 6;
const int spacing = 6;
const int sectionSize = 6;
const int TextSpacing = 12;
const int allDatecount = 30;

ChartViewWidget::ChartViewWidget(ChartViewTypes types, QWidget *parent)
    :  m_viewType(types),
      QWidget(parent)
{
    QFont ft1;
    ft1.setPixelSize(14);

    m_font = ft1;

    netChartView = new KChartView(this);
    netChartView->setAxisTitle("0.0 b/s");

    this->updateWidgetGeometry();

}

ChartViewWidget::~ChartViewWidget()
{

}

QString ChartViewWidget::getNCName()
{
    return m_ifname;
}

void ChartViewWidget::setData1Color(const QColor &color)
{
    m_data1Color = color;
    KChartAttr attr1;
    attr1.colorForeground = m_data1Color;
    attr1.colorAlpha = 154;
    netChartView->addItemAttr(attr1);
}

void ChartViewWidget::setData2Color(const QColor &color)
{
    m_data2Color = color;
    KChartAttr attr2;
    attr2.colorForeground = m_data2Color;
    attr2.colorAlpha = 154;
    netChartView->addItemAttr(attr2);
}

void ChartViewWidget::updateChartDate(qreal sendRate, qreal recvRate)
{
    // 获取32次接收与发送速率中的最大值
    m_listData1 << sendRate;
    if (m_listData1.count() > allDatecount + 1) {
        m_listData1.pop_front();
    }
    qreal maxdata1 = *std::max_element(m_listData1.begin(), m_listData1.end());
    if (maxdata1 > 0 && maxdata1 != m_maxData1) {
        m_maxData1 = maxdata1 *1.1;
        m_maxData = qMax(m_maxData1, m_maxData2);
    }

    m_listData2 << recvRate;
    if (m_listData2.count() > allDatecount + 1) {
        m_listData2.pop_front();
    }
    qreal maxdata2 = *std::max_element(m_listData2.begin(), m_listData2.end());
    if (maxdata2 > 0 && maxdata2 != m_maxData2) {
        m_maxData2 = maxdata2 *1.1;
        m_maxData = qMax(m_maxData1, m_maxData2);
    }

    netChartView->updateAxisRTitle(commom::format::formatNetworkBrandWidth(m_maxData, false));

    qreal newSendMaxData = netChartView->getRealMaxData(sendRate, 1);
    qreal newRecvMaxData = netChartView->getRealMaxData(recvRate, 0);
    qreal newMaxData = commom::format::chkNewMaxData(newSendMaxData>newRecvMaxData?newSendMaxData:newRecvMaxData);
    netChartView->setMaxData(newMaxData);

    netChartView->onUpdateData(sendRate, 1);
    netChartView->onUpdateData(recvRate, 0);

    this->update();
}

void ChartViewWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    int curXMargin = m_mode == TITLE_HORIZONTAL ? 0 : margin;
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setFont(m_font);

    const QPalette &palette = QPalette();

    const QColor &textColor = palette.color(QPalette::Text);
    if (m_isActive) {
        painter.setPen(Qt::NoPen);
        QColor selectColor = palette.color(QPalette::Highlight);
        selectColor.setAlphaF(0.1);

        painter.setBrush(selectColor);
        painter.drawRoundedRect(rect(), 8, 8);

        painter.setPen(palette.color(QPalette::Highlight));
    } else {
        painter.setPen(textColor);
    }

    QRect ifnameRect(curXMargin, margin / 2, painter.fontMetrics().width(m_ifname), painter.fontMetrics().height());
    painter.drawText(ifnameRect, Qt::AlignLeft | Qt::AlignVCenter, m_ifname);

    painter.setPen(textColor);

}

void ChartViewWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateWidgetGeometry();
}

void ChartViewWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    emit clicked(m_ifname);
    update();
}

void ChartViewWidget::setMode(int mode)
{
    m_mode = mode;
    this->update();
}

void ChartViewWidget::setActive(bool active)
{
    m_isActive = active;
    this->update();
}

void ChartViewWidget::updateWidgetGeometry()
{
    int fontHeight = QFontMetrics(m_font).height();
    int curXMargin = m_mode == TITLE_HORIZONTAL ? 0 :  margin;
    if (m_mode == TITLE_HORIZONTAL) {
        netChartView->setGeometry(curXMargin, TextSpacing, this->width() - 2 * curXMargin, this->height() - TextSpacing - margin);
    } else {
        netChartView->setGeometry(curXMargin, fontHeight * 2 + TextSpacing, this->width() - 2 * curXMargin, this->height() - fontHeight * 2 - TextSpacing - margin);
    }
    netChartView->update();
}

void ChartViewWidget::setAxisTitle(const QString &text)
{
    m_axisTitle = text;
    update();
}

