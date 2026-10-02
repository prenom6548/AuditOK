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
#include "chartitemwidget.h"

#include <QtMath>
#include <QPainter>
#include <QApplication>

#include "commoninfo.h"

const int margin = 6;
const int spacing = 6;
const int sectionSize = 6;
const int TextSpacing = 12;
const int allDatecount = 30;

//DWIDGET_USE_NAMESPACE
ChartItemWidget::ChartItemWidget(ChartViewTypes chartType, QWidget *parent, const QString &mac)
    : m_chartType(chartType),
      QWidget(parent)
{
    m_pChartView = new KChartView(this);
    switch (m_chartType) {
    case ChartViewTypes::CPU_CHART:
        m_data1Color = QColor("#3790FA");
        m_pChartView->setAxisTitle("100%");
        m_pChartView->setMaxData(100);
        setData1Color(m_data1Color);
        break;
    case ChartViewTypes::MEM_CHART:
        break;
    case ChartViewTypes::NET_CHART:
        m_data1Title = tr("Receive");
        m_data2Title = tr("Send");
        m_data1Color = QColor("#B37FEB");
        m_data2Color = QColor("#FF7875");
        m_chartName = mac;
        m_pChartView->setAxisTitle("0.0 b/s");
        setData1Color(m_data1Color);
        setData2Color(m_data2Color);
        break;
    case ChartViewTypes::BLOCK_CHART:
        m_data1Title = tr("Disk Read");
        m_data2Title = tr("Disk Write");
        m_data1Color = QColor("#3790FA");
        m_data2Color = QColor("#F1852B");
        m_pChartView->setAxisTitle("0.0 b/s");
        setData1Color(m_data1Color);
        setData2Color(m_data2Color);
        break;
    default:
        break;
    }
    updateWidgetGeometry();
}

void ChartItemWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    int curXMargin = m_mode == TITLE_HORIZONTAL ? 0 : margin;
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setFont(m_font);

    QString strRecvData = QString("%1 %2").arg(m_data1Title).arg(m_recv_bps);
    QString strSentData = QString("%1 %2").arg(m_data2Title).arg(m_sent_bps);

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

    QRect ifnameRect(0,0,0,0);

    QString language = QLocale::system().name();
    if (!m_chartName.isEmpty()) {
        // 维吾尔 哈萨克斯 柯尔克孜 语言控件翻转
        if (language == "ug_CN" || language == "kk_KZ" || language == "ky_KG") {
            ifnameRect = QRect(this->width() - painter.fontMetrics().width(m_chartName) - curXMargin, margin / 2, painter.fontMetrics().width(m_chartName), painter.fontMetrics().height());
            painter.drawText(ifnameRect, Qt::AlignRight | Qt::AlignVCenter, m_chartName);
        } else {
            ifnameRect = QRect(curXMargin, margin / 2, painter.fontMetrics().width(m_chartName), painter.fontMetrics().height());
            painter.drawText(ifnameRect, Qt::AlignLeft | Qt::AlignVCenter, m_chartName);
        }
    }

    painter.setPen(textColor);

    // 绘制文字
    // 维吾尔 哈萨克斯 柯尔克孜 语言控件翻转
    QRect recvRect;
    if (language == "ug_CN" || language == "kk_KZ" || language == "ky_KG") {
        recvRect = QRect(this->width() - painter.fontMetrics().width(strRecvData) - (margin + spacing + sectionSize), ifnameRect.bottom(), painter.fontMetrics().width(strRecvData), painter.fontMetrics().height());
        painter.drawText(recvRect, Qt::AlignRight | Qt::AlignVCenter, strRecvData);
    } else {
        recvRect = QRect(margin + spacing + sectionSize, ifnameRect.bottom(), painter.fontMetrics().width(strRecvData), painter.fontMetrics().height());
        painter.drawText(recvRect, Qt::AlignLeft | Qt::AlignVCenter, strRecvData);
    }

    QRect sentRect;
    if (!m_data2Title.isEmpty()) {
        // 维吾尔 哈萨克斯 柯尔克孜 语言控件翻转
        if (language == "ug_CN" || language == "kk_KZ" || language == "ky_KG") {
            sentRect = QRect(this->width() - painter.fontMetrics().width(strSentData) - (margin + spacing + sectionSize), recvRect.bottom(), painter.fontMetrics().width(strSentData), painter.fontMetrics().height());
            painter.drawText(sentRect, Qt::AlignRight | Qt::AlignVCenter, strSentData);
        } else {
            sentRect = QRect(margin + spacing + sectionSize, recvRect.bottom(), painter.fontMetrics().width(strSentData), painter.fontMetrics().height());
            painter.drawText(sentRect, Qt::AlignLeft | Qt::AlignVCenter, strSentData);
        }
    }

    // 绘制圆点
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_data1Color);
    // 维吾尔 哈萨克斯 柯尔克孜 语言控件翻转
    if (language == "ug_CN" || language == "kk_KZ" || language == "ky_KG") {
        painter.drawEllipse(this->width() - sectionSize - margin, recvRect.y() + qCeil((recvRect.height() - sectionSize) / 2.0), sectionSize, sectionSize);
    } else {
        painter.drawEllipse(margin, recvRect.y() + qCeil((recvRect.height() - sectionSize) / 2.0), sectionSize, sectionSize);
    }

    if (m_data2Color.isValid()) {
        painter.setBrush(m_data2Color);
        if (language == "ug_CN" || language == "kk_KZ" || language == "ky_KG") {
            painter.drawEllipse(this->width() - sectionSize - margin, sentRect.y() + qCeil((sentRect.height() - sectionSize) / 2.0), sectionSize, sectionSize);
        } else {
            painter.drawEllipse(margin, sentRect.y() + qCeil((sentRect.height() - sectionSize) / 2.0), sectionSize, sectionSize);
        }

    }
}

void ChartItemWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateWidgetGeometry();
}

void ChartItemWidget::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    emit clicked(m_chartName);
}

void ChartItemWidget::fontChanged(const QFont &font)
{
    m_font = font;
    this->setFont(m_font);
    m_pChartView->fontChanged(m_font);
    updateWidgetGeometry();
}

void ChartItemWidget::setMode(int mode)
{
    m_mode = mode;
    this->update();
}

void ChartItemWidget::updateData(const NetifInfo &netifInfo)
{
    // 更新图表
    updateChartDate(netifInfo.sendBytes, netifInfo.receiveBytes);

    if (!netifInfo.netName.isEmpty()) {
        m_chartName = netifInfo.netName;
    }

    m_recv_bps = commom::format::formatNetworkBrandWidth(netifInfo.receiveBytes, false);
    m_sent_bps = commom::format::formatNetworkBrandWidth(netifInfo.sendBytes, false);

    this->update();
}

void ChartItemWidget::updateData(const QString &deviceName)
{
    m_chartName = deviceName;

    double readSpeed = DiskSpeedInfo::instance()->readSpeed(deviceName);
    double writeSpeed = DiskSpeedInfo::instance()->writeSpeed(deviceName);

    m_recv_bps = DiskSpeedInfo::instance()->readSpeedStr(deviceName);
    m_sent_bps = DiskSpeedInfo::instance()->writeSpeedStr(deviceName);

    updateChartDate(writeSpeed, readSpeed);

    this->update();
}

void ChartItemWidget::updateCpuData(const QString &deviceName, qreal value)
{
    m_data1Title = deviceName;

    updateChartDate(value);

    this->update();
}

void ChartItemWidget::updateActiveStatus(bool active)
{
    m_isActive = active;
    update();
}

bool ChartItemWidget::isActiveItem()
{
    return  m_isActive;
}

void ChartItemWidget::updateWidgetGeometry()
{
    m_font = QApplication::font();

    int fontHeight = QFontMetrics(m_font).height();
    int curXMargin = m_mode == TITLE_HORIZONTAL ? 0 : margin;
    if (m_mode == TITLE_HORIZONTAL) {
        m_pChartView->setGeometry(curXMargin, TextSpacing, this->width() - 2 * curXMargin, this->height() - TextSpacing - margin);
    } else {
        if (m_data1Title.isEmpty() || m_data2Title.isEmpty()) {
            m_pChartView->setGeometry(curXMargin, 0, this->width() - 2 * curXMargin, this->height() - TextSpacing - margin);
        } else {
            m_pChartView->setGeometry(curXMargin, fontHeight * 2, this->width() - 2 * curXMargin, this->height() - fontHeight * 2 - TextSpacing - margin);
        }
    }
}

void ChartItemWidget::setData1Color(const QColor &color)
{
    m_data1Color = color;
    KChartAttr attr1;
    attr1.colorForeground = m_data1Color;
    attr1.colorAlpha = 154;
    m_pChartView->addItemAttr(attr1);
}

void ChartItemWidget::setData2Color(const QColor &color)
{
    m_data2Color = color;
    KChartAttr attr2;
    attr2.colorForeground = m_data2Color;
    attr2.colorAlpha = 154;
    m_pChartView->addItemAttr(attr2);
}

void ChartItemWidget::updateChartDate(qreal sendRate, qreal recvRate)
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

    m_pChartView->updateAxisRTitle(commom::format::formatNetworkBrandWidth(m_maxData, false));

    qreal newSendMaxData = m_pChartView->getRealMaxData(sendRate, 1);
    qreal newRecvMaxData = m_pChartView->getRealMaxData(recvRate, 0);
    qreal newMaxData = commom::format::chkNewMaxData(newSendMaxData>newRecvMaxData?newSendMaxData:newRecvMaxData);
    m_pChartView->setMaxData(newMaxData);

    m_pChartView->onUpdateData(sendRate, 1);
    m_pChartView->onUpdateData(recvRate, 0);

    this->update();
}

void ChartItemWidget::updateChartDate(qreal sendRate)
{
    m_pChartView->onUpdateData(sendRate, 0);
    this->update();
}

