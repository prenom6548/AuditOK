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
#include "diskhistorywidget.h"
#include <QPainter>
#include <QPainterPath>
#include <QIcon>
#include <QLocale>
#include <QApplication>
#include "commoninfo.h"

#include "../macro.h"

DiskHistoryWidget::DiskHistoryWidget(QWidget *parent)
    : QWidget(parent)
{
    m_curFontSize = DEFAULT_FONT_SIZE;
    setAttribute(Qt::WA_TranslucentBackground);
    this->installEventFilter(this);
    initUI();
    initStyleTheme();
}

DiskHistoryWidget::~DiskHistoryWidget()
{
    if (m_pStyleSettings) {
        delete m_pStyleSettings;
        m_pStyleSettings = nullptr;
    }
}

void DiskHistoryWidget::initUI()
{
    // 初始化布局
    m_pMainVLayout = new QVBoxLayout();
    m_pMainVLayout->setContentsMargins(12, 12, 12, 12);
    m_pMainVLayout->setSpacing(0);
    m_pReadHLayout = new QHBoxLayout();
    m_pReadHLayout->setContentsMargins(0, 0, 0, 0);
    m_pReadHLayout->setSpacing(3);
    m_pWriteHLayout = new QHBoxLayout();
    m_pWriteHLayout->setContentsMargins(0, 0, 0, 0);
    m_pWriteHLayout->setSpacing(3);

    //初始化控件
    m_pDiskTitleLabel = new KLabel();
    m_pDiskTitleLabel->setText(tr("Disk reading and writing"));
    m_pDiskTitleLabel->setFixedWidth(160);
    m_pDiskTitleLabel->setAlignment(Qt::AlignLeft);

    m_pIconReadLabel = new QLabel();
    m_pIconReadLabel->setPixmap(QIcon::fromTheme(":/img/read.svg").pixmap(14,14));
    m_pReadHLayout->addWidget(m_pIconReadLabel, 0, Qt::AlignLeft|Qt::AlignVCenter);
    m_pReadTitleLabel = new KLabel();
    m_pReadTitleLabel->setFontColorRole(QPalette::PlaceholderText);
    m_pReadTitleLabel->setText(tr("Disk Read"));
    m_pReadTitleLabel->setFixedWidth(60);
    m_pReadTitleLabel->setAlignment(Qt::AlignLeft);
    m_pReadHLayout->addWidget(m_pReadTitleLabel, 1, Qt::AlignLeft|Qt::AlignVCenter);
    m_pReadValueLabel = new KLabel();
    m_pReadValueLabel->setFontColorRole(QPalette::PlaceholderText);
    m_pReadValueLabel->setText("0MB/s");
    m_pReadValueLabel->setFixedWidth(80);

    m_pIconWriteLabel = new QLabel();
    m_pIconWriteLabel->setPixmap(QIcon::fromTheme(":/img/write.svg").pixmap(14,14));
    m_pWriteHLayout->addWidget(m_pIconWriteLabel, 0, Qt::AlignLeft|Qt::AlignVCenter);
    m_pWriteTitleLabel = new KLabel();
    m_pWriteTitleLabel->setFontColorRole(QPalette::PlaceholderText);
    m_pWriteTitleLabel->setText(tr("Disk Write"));
    m_pWriteTitleLabel->setFixedWidth(60);
    m_pWriteTitleLabel->setAlignment(Qt::AlignLeft);
    m_pWriteHLayout->addWidget(m_pWriteTitleLabel, 1, Qt::AlignLeft|Qt::AlignVCenter);
    m_pWriteValueLabel = new KLabel();
    m_pWriteValueLabel->setFontColorRole(QPalette::PlaceholderText);
    m_pWriteValueLabel->setText("0MB/s");
    m_pWriteValueLabel->setFixedWidth(80);
    m_pReadValueLabel->setAlignment(Qt::AlignRight);
    m_pWriteValueLabel->setAlignment(Qt::AlignRight);

    // 维吾尔 哈萨克斯 柯尔克孜 语言控件翻转
    QString language = QLocale::system().name();
    if (language == "ug_CN" || language == "kk_KZ" || language == "ky_KG") {
        m_pReadHLayout->addWidget(m_pReadValueLabel, 1, Qt::AlignLeft|Qt::AlignVCenter);
        m_pWriteHLayout->addWidget(m_pWriteValueLabel, 1, Qt::AlignLeft|Qt::AlignVCenter);
    } else {
        m_pReadHLayout->addWidget(m_pReadValueLabel, 1, Qt::AlignRight|Qt::AlignVCenter);
        m_pWriteHLayout->addWidget(m_pWriteValueLabel, 1, Qt::AlignRight|Qt::AlignVCenter);
    }
    m_pMainVLayout->addWidget(m_pDiskTitleLabel);
    m_pMainVLayout->addSpacing(4);
    m_pMainVLayout->addLayout(m_pReadHLayout);
    m_pMainVLayout->addSpacing(4);
    m_pMainVLayout->addLayout(m_pWriteHLayout);
    this->setLayout(m_pMainVLayout);
}

void DiskHistoryWidget::initStyleTheme()
{
    const QByteArray idd(THEME_QT_SCHEMA);
    if(QGSettings::isSchemaInstalled(idd)) {
        m_pStyleSettings = new QGSettings(idd);
    }
    if (m_pStyleSettings) {
        //监听主题改变
        connect(m_pStyleSettings, &QGSettings::changed, this, [=](const QString &key){
            themeChangedSlot(key);
        });
        auto variant = m_pStyleSettings->get(FONT_SIZE);
        if (variant.isValid()) {
            m_curFontSize = m_pStyleSettings->get(FONT_SIZE).toString().toFloat();
            onThemeFontChange(m_curFontSize);
        }
    }
}

void DiskHistoryWidget::themeChangedSlot(const QString &key)
{
    if("systemFont" == key || "systemFontSize" == key) {
        auto variant = m_pStyleSettings->get(FONT_SIZE);
        if (variant.isValid()) {
            m_curFontSize = m_pStyleSettings->get(FONT_SIZE).toString().toFloat();
            onThemeFontChange(m_curFontSize);
        }
    }
}

void DiskHistoryWidget::onThemeFontChange(float fFontSize)
{
    QFont fontContext;
    fontContext.setPointSize(fFontSize*0.8);
    QFont fontTitle = this->font();
    if (m_pReadValueLabel) {
        m_pReadValueLabel->setFont(fontContext);
    }
    if (m_pWriteValueLabel) {
        m_pWriteValueLabel->setFont(fontContext);
    }
}

void DiskHistoryWidget::onUpdateDiskData()
{
    double allReadSpeed = 0;
    double allWriteSpeed = 0;
    QStringList diskList = diskInfo::getDiskName();
    foreach (QString diskName, diskList) {
        DiskSpeedInfo::instance()->updateDiskSpeed(diskName);
        double readSpeed = DiskSpeedInfo::instance()->readSpeed(diskName);
        double writeSpeed = DiskSpeedInfo::instance()->writeSpeed(diskName);
        allReadSpeed += readSpeed;
        allWriteSpeed += writeSpeed;
    }

    m_pReadValueLabel->setText(commom::format::Format_Memory(allReadSpeed, KDK_KILOBYTE) + "/s");
    m_pWriteValueLabel->setText(commom::format::Format_Memory(allWriteSpeed, KDK_KILOBYTE) + "/s");
}

void DiskHistoryWidget::setClickedFlags(bool clicked)
{
    m_clicked = clicked;
    this->update();
}

void DiskHistoryWidget::paintEvent(QPaintEvent *event)
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

bool DiskHistoryWidget::eventFilter(QObject *watched, QEvent *event)
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
