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
#include "cpuhistorywidget.h"
#include "../macro.h"

#include <QPainterPath>
#include <QPainter>
#include <QPalette>

#include <kysdk/applications/accessinfohelper.h>
using namespace kdk;

CpuHistoryWidget::CpuHistoryWidget(QWidget *parent)
    : QWidget(parent)
{
    m_curFontSize = DEFAULT_FONT_SIZE;
    setAttribute(Qt::WA_TranslucentBackground);
    this->installEventFilter(this);
    initUI();
    initStyleTheme();
}

CpuHistoryWidget::~CpuHistoryWidget()
{
    if (m_styleSettings) {
        delete m_styleSettings;
        m_styleSettings = nullptr;
    }
}

void CpuHistoryWidget::initUI()
{
    // 初始化布局
    m_mainLayout = new QVBoxLayout();
    m_mainLayout->setContentsMargins(12, 12, 12, 12);
    m_mainLayout->setSpacing(4);
    m_titleLayout = new QHBoxLayout();
    m_titleLayout->setContentsMargins(0, 0, 0, 0);
    m_titleLayout->setSpacing(0);

    //初始化控件
    m_labelTitle = new KLabel();
    m_labelTitle->setText(tr("CPU"));
    m_labelTitle->setFixedWidth(80);
    m_labelTitle->setAlignment(Qt::AlignLeft);
    m_titleLayout->addWidget(m_labelTitle, 1, Qt::AlignLeft|Qt::AlignVCenter);
    m_labelValue = new KLabel();
    m_labelValue->setText("0.0%");
    m_labelValue->setFixedWidth(80);
    m_labelValue->setFontColorRole(QPalette::PlaceholderText);
    m_labelValue->setAlignment(Qt::AlignRight);

    // 维吾尔 哈萨克斯 柯尔克孜 语言控件翻转
    QString language = QLocale::system().name();
    if (language == "ug_CN" || language == "kk_KZ" || language == "ky_KG") {
        m_titleLayout->addWidget(m_labelValue, 1, Qt::AlignLeft|Qt::AlignVCenter);
    } else {
        m_titleLayout->addWidget(m_labelValue, 1, Qt::AlignRight|Qt::AlignVCenter);
    }

    m_progressBar = new QProgressBar();
    KDK_EXTEND_ALL_INFO_FORMAT(m_progressBar, "CpuHistoryWidget", "", "");
    m_progressBar->setTextVisible(false);
    m_progressBar->setMaximum(100);
    m_progressBar->setFixedSize(160, 4);

    m_chartView = new KChartView();
    KChartAttr attr;
    attr.colorForeground = QColor("#3790FA");
    attr.colorAlpha = 154;
    m_chartView->addItemAttr(attr);
    m_chartView->setFixedHeight(60);

    m_mainLayout->addLayout(m_titleLayout);
    m_mainLayout->addWidget(m_progressBar);
    m_mainLayout->addSpacing(4);
    m_mainLayout->addWidget(m_chartView);
    this->setLayout(m_mainLayout);
}

void CpuHistoryWidget::initStyleTheme()
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

void CpuHistoryWidget::onThemeFontChange(float fFontSize)
{
    QFont fontContext;
    fontContext.setPointSize(fFontSize*0.8);
    if (m_labelValue) {
        m_labelValue->setFont(fontContext);
    }
}

void CpuHistoryWidget::onUpdateData(QVector<double> usage)
{
    qreal cpuPercent = 0;
    if (usage.size() > 0) {
        cpuPercent = usage[0];
    }
    m_progressBar->setValue(cpuPercent);
    m_chartView->onUpdateData(cpuPercent, 0);
    QString showValue;
    showValue = QString::number(cpuPercent,'f',1)+"%";
    m_labelValue->setText(showValue);
}

QString CpuHistoryWidget::getCpuOccupancy()
{
    return m_labelValue->text();
}

void CpuHistoryWidget::setClickedFlags(bool clicked)
{
    m_clicked = clicked;
    this->update();
}

void CpuHistoryWidget::paintEvent(QPaintEvent *event)
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
    path.addRoundedRect(this->rect(), radius, radius);
    path.setFillRule(Qt::WindingFill);
    painter.setPen(Qt::transparent);
    if (qApp->property("window-active").canConvert<QBrush>()) {
        QBrush containColor = qApp->property("window-active").value<QBrush>();
        painter.setBrush(containColor);
    }
    painter.drawPath(path);
    QWidget::paintEvent(event);
}

bool CpuHistoryWidget::eventFilter(QObject *watched, QEvent *event)
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
