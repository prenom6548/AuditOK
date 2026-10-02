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
#include "memhistorywidget.h"
#include "../macro.h"
#include "../util.h"

#include <QPainterPath>
#include <QPainter>
#include <QPalette>
#include <kysdk/kysdk-base/kyutils.h>
#include <kysdk/applications/accessinfohelper.h>
using namespace kdk;

inline QString formatMemory(guint64 size)
{
    enum {
        K_INDEX,
        M_INDEX,
        G_INDEX,
        T_INDEX
    };
    char data[128] = {0};
    snprintf(data, sizeof(data), "%lu",size);
    char result_data[128] = {0};
    double m_speed = 0;
    KDKVolumeBaseType m_KDKVolumeBaseType;

    QList<guint64> factorList;
    factorList.append(G_GUINT64_CONSTANT(1) << 10);//KiB
    factorList.append(G_GUINT64_CONSTANT(1) << 20);//MiB
    factorList.append(G_GUINT64_CONSTANT(1) << 30);//GiB
    factorList.append(G_GUINT64_CONSTANT(1) << 40);//TiB

    guint64 factor;
    QString format;
    if (size < factorList.at(M_INDEX)) {
        m_KDKVolumeBaseType = KDK_KILOBYTE;
        factor = factorList.at(K_INDEX);
        format = "KB";
    } else if (size < factorList.at(G_INDEX)) {
        m_KDKVolumeBaseType = KDK_MEGABYTE;
        factor = factorList.at(M_INDEX);
        format = "MB";
    } else if (size < factorList.at(T_INDEX)) {
        m_KDKVolumeBaseType = KDK_GIGABYTE;
        factor = factorList.at(G_INDEX);
        format = "GB";
    } else {
        m_KDKVolumeBaseType = KDK_TERABYTE;
        factor = factorList.at(T_INDEX);
        format = "TB";
    }

    if (size > 1024 && kdkVolumeBaseCharacterConvert(data, m_KDKVolumeBaseType, result_data) == KDK_NOERR){
        QString formatted_result = "";
        sscanf(result_data, "%lf", &m_speed);
        formatted_result.setNum(m_speed, 'f', 1);
        return formatted_result + format;
    } else {
        std::string formatted_result(make_string(g_strdup_printf("%.1f", size / (double)factor)));
        return QString::fromStdString(formatted_result) + format;
    }
}

MemHistoryWidget::MemHistoryWidget(QWidget *parent)
    : QWidget(parent)
{
    m_curFontSize = DEFAULT_FONT_SIZE;
    setAttribute(Qt::WA_TranslucentBackground);
    this->installEventFilter(this);
    initUI();
    initStyleTheme();
}

MemHistoryWidget::~MemHistoryWidget()
{
    if (m_styleSettings) {
        delete m_styleSettings;
        m_styleSettings = nullptr;
    }
}

void MemHistoryWidget::setClickedFlags(bool clicked)
{
    m_clicked = clicked;
    this->update();
}

void MemHistoryWidget::paintEvent(QPaintEvent *event)
{
    QPainterPath path;
    QPainter painter(this);
    int radius = qApp->property("kradius-normal").toInt();

    if (m_clicked) {
        QColor highlightColor = palette().color(QPalette::Highlight);
        QRectF borderRect = rect().adjusted(1, 1, -1, -1);
        path.addRoundedRect(borderRect, radius, radius);
        painter.setClipPath(path);
        painter.setClipping(false);
        QPen pen(highlightColor, 2);
        painter.setPen(pen);
        painter.drawPath(path);
    }

    painter.setOpacity(transparent);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter.setClipping(true);
    painter.setPen(Qt::transparent);
    path.addRoundedRect(this->rect(), radius, radius);
    path.setFillRule(Qt::WindingFill);
    if (qApp->property("window-active").canConvert<QBrush>()) {
        QBrush containColor = qApp->property("window-active").value<QBrush>();
        painter.setBrush(containColor);
    }
    painter.drawPath(path);
    QWidget::paintEvent(event);
}

void MemHistoryWidget::initUI()
{
    // 初始化布局
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setContentsMargins(12, 12, 12, 12);
    m_mainLayout->setSpacing(4);
    m_memTitleLayout = new QHBoxLayout();
    m_memTitleLayout->setContentsMargins(0, 0, 0, 0);
    m_memTitleLayout->setSpacing(0);
    m_memProgLayout = new QHBoxLayout();
    m_memProgLayout->setContentsMargins(0, 0, 0, 0);
    m_memProgLayout->setSpacing(0);
    m_swapTitleLayout = new QHBoxLayout();
    m_swapTitleLayout->setContentsMargins(0, 0, 0, 0);
    m_swapTitleLayout->setSpacing(0);
    m_swapProgLayout = new QHBoxLayout();
    m_swapProgLayout->setContentsMargins(0, 0, 0, 0);
    m_swapProgLayout->setSpacing(0);

    //初始化控件
    m_labelMemTitle = new KLabel();
    m_labelMemTitle->setText(tr("Memory"));
    m_labelMemTitle->setFixedWidth(80);
    m_labelMemTitle->setAlignment(Qt::AlignLeft);
    m_memTitleLayout->addWidget(m_labelMemTitle, 1, Qt::AlignLeft|Qt::AlignVCenter);
    m_labelMemValue = new KLabel();
    m_labelMemValue->setText("0.0KB/0.0KB");
    m_labelMemValue->setFixedWidth(80);
    m_labelMemValue->setFontColorRole(QPalette::PlaceholderText);

    m_labelSwapTitle = new KLabel();
    m_labelSwapTitle->setText(tr("Swap"));
    m_labelSwapTitle->setFixedWidth(80);
    m_labelSwapTitle->setAlignment(Qt::AlignLeft);
    m_swapTitleLayout->addWidget(m_labelSwapTitle, 1, Qt::AlignLeft|Qt::AlignVCenter);
    m_labelSwapValue = new KLabel();
    m_labelSwapValue->setFontColorRole(QPalette::PlaceholderText);
    m_labelSwapValue->setText(tr("Not enabled"));
    m_labelSwapValue->setFixedWidth(80);

    // 维吾尔 哈萨克斯 柯尔克孜 语言控件翻转
    QString language = QLocale::system().name();
    if (language == "ug_CN" || language == "kk_KZ" || language == "ky_KG") {
        m_labelMemValue->setAlignment(Qt::AlignLeft);
        m_memTitleLayout->addWidget(m_labelMemValue, 1, Qt::AlignLeft|Qt::AlignVCenter);
        m_labelSwapValue->setAlignment(Qt::AlignLeft);
        m_swapTitleLayout->addWidget(m_labelSwapValue, 1, Qt::AlignLeft|Qt::AlignVCenter);
    } else {
        m_labelMemValue->setAlignment(Qt::AlignRight);
        m_memTitleLayout->addWidget(m_labelMemValue, 1, Qt::AlignRight|Qt::AlignVCenter);
        m_labelSwapValue->setAlignment(Qt::AlignRight);
        m_swapTitleLayout->addWidget(m_labelSwapValue, 1, Qt::AlignRight|Qt::AlignVCenter);
    }

    m_progressBarMem = new QProgressBar();
    KDK_EXTEND_ALL_INFO_FORMAT(m_progressBarMem, "MemHistoryWidget", "", "");

    m_progressBarMem->setTextVisible(false);
    m_progressBarMem->setMaximum(100);
    m_progressBarMem->setFixedSize(160, 4);
    QBrush mem(QColor("#52C429"));
    m_progressBarMem->setProperty("setContentHightColor", mem);
    m_progressBarMem->setProperty("setContentMidLightColor", mem);

    m_progressBarSwap = new QProgressBar();
    KDK_EXTEND_ALL_INFO_FORMAT(m_progressBarSwap, "MemHistoryWidget", "", "");
    m_progressBarSwap->setTextVisible(false);
    m_progressBarSwap->setMaximum(100);
    m_progressBarSwap->setFixedSize(160, 4);
    QBrush swap(QColor("#F68C27"));
    m_progressBarSwap->setProperty("setContentHightColor", swap);
    m_progressBarSwap->setProperty("setContentMidLightColor", swap);

    m_chartView = new KChartView();
    KChartAttr attr;
    attr.colorForeground = QColor("#52C429");
    attr.colorAlpha = 154;
    m_chartView->addItemAttr(attr);
    attr.colorForeground = QColor("#F68C27");
    attr.colorAlpha = 154;
    m_chartView->addItemAttr(attr);
    m_chartView->setFixedHeight(60);

    m_mainLayout->addLayout(m_memTitleLayout);
    m_mainLayout->addWidget(m_progressBarMem);
    m_mainLayout->addLayout(m_swapTitleLayout);
    m_mainLayout->addWidget(m_progressBarSwap);
    m_mainLayout->addSpacing(4);
    m_mainLayout->addWidget(m_chartView);
    this->setLayout(m_mainLayout);
}

void MemHistoryWidget::initStyleTheme()
{
    const QByteArray idd(THEME_QT_SCHEMA);
    if(QGSettings::isSchemaInstalled(idd)) {
        m_styleSettings = new QGSettings(idd);
    }
    if (m_styleSettings) {
        //监听主题改变
        connect(m_styleSettings, &QGSettings::changed, this, [=](const QString &key) {
            if("systemFont" == key || "systemFontSize" == key) {
                auto variant = m_styleSettings->get(FONT_SIZE);
                if (variant.isValid()) {
                    m_curFontSize = m_styleSettings->get(FONT_SIZE).toString().toFloat();
                    onThemeFontChange(m_curFontSize);
                }
            }
        });
        auto variant = m_styleSettings->get(FONT_SIZE);
        if (variant.isValid()) {
            m_curFontSize = m_styleSettings->get(FONT_SIZE).toString().toFloat();
            onThemeFontChange(m_curFontSize);
        }
    }
}

void MemHistoryWidget::onThemeFontChange(float fFontSize)
{
    QFont fontContext;
    fontContext.setPointSize(fFontSize*0.8);
    if (m_labelMemValue) {
        m_labelMemValue->setFont(fontContext);
    }
    if (m_labelSwapValue) {
        m_labelSwapValue->setFont(fontContext);
    }
}

void MemHistoryWidget::onUpdateMemData(qreal memUsed, qreal memTotal)
{
    qreal memPercent = (memTotal ? memUsed  / memTotal : 0.0f) * 100.0;
    m_progressBarMem->setValue(memPercent);
    m_chartView->onUpdateData(memPercent, 0);
    const QString infoMemory = QString("%1/%2").arg(formatMemory(memUsed * 1024)).arg(formatMemory(memTotal * 1024));
    m_labelMemValue->setText(infoMemory);
}

void MemHistoryWidget::onUpdateSwapData(qreal swapUsed, qreal swapTotal)
{
    qreal swapPercent = (swapTotal ? swapUsed  / swapTotal : 0.0f) * 100.0;
    m_progressBarSwap->setValue(swapPercent);
    m_chartView->onUpdateData(swapPercent, 1);
    const QString infoMemory = QString("%1/%2").arg(formatMemory(swapUsed * 1024)).arg(formatMemory(swapTotal * 1024));
    if (qAbs(swapTotal) < 1e-15) {
        m_labelSwapValue->setText(tr("Not enabled"));
        if (m_progressBarSwap->isVisible()) {
            m_progressBarSwap->setVisible(false);
        }
    } else {
        m_labelSwapValue->setText(infoMemory);
        if (!m_progressBarSwap->isVisible()) {
            m_progressBarSwap->setVisible(true);
        }
    }
    return;
}

QString MemHistoryWidget::getMemOccupancy()
{
    return QString::number(m_progressBarMem->value(), 'f', 1) + "%";
}

bool MemHistoryWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            m_clicked = true;
            this->update();
            Q_EMIT clicked();
        }
    }

    if (event->type() == QEvent::Enter) {
        transparent = 0.6;
        this->update();
    } else if (event->type() == QEvent::Leave) {
        transparent = 0.4;
        this->update();
    }

    return QWidget::eventFilter(watched, event);
}
