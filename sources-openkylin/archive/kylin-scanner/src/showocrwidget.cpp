/*
* Copyright (C) 2021, KylinSoft Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/
#include "showocrwidget.h"
#include <QDebug>
#include <kysdk/applications/gsettingmonitor.h>
#include <QStackedWidget>

showOcrWidget::showOcrWidget(QWidget *parent) : QLabel(parent)
  , m_ocrImageLabel (new QLabel())
  , m_ocrImageHLayout (new QHBoxLayout())
  , m_ocrTextEdit (new QTextEdit())
  , m_ocrTextEditHLayout (new QHBoxLayout())
  , m_toolbarWidget (new ToolBarWidget())
  , m_ocrVLayout (new QVBoxLayout())
  , m_ocrTextEditContainer(new QWidget())
{
    setupGui();
    initConnect();
}

void showOcrWidget::setupGui()
{
    this->setMinimumSize(QSize(ShowOcrWidgetWidth, ShowOcrWidgetHeight));

    m_ocrImageLabel->setMinimumSize(QSize(108, 150));

    m_ocrImageHLayout->setSpacing(0);
    m_ocrImageHLayout->addStretch();
    m_ocrImageHLayout->addWidget(m_ocrImageLabel, 0, Qt::AlignCenter | Qt::AlignVCenter | Qt::AlignHCenter);
    m_ocrImageHLayout->addStretch();
    m_ocrImageHLayout->setContentsMargins(0, 0, 0, 0);

    m_ocrTextEdit->setText(tr("The document is in character recognition ..."));
    m_ocrTextEdit->setReadOnly(true);
    m_ocrTextEdit->setMinimumSize(QSize(580-AddWidthForLargeFontSize, 370));

    m_ocrTextEditHLayout->addWidget(m_ocrTextEdit, 0, Qt::AlignCenter | Qt::AlignVCenter |Qt::AlignHCenter);
    m_ocrTextEditHLayout->setContentsMargins(12, 12, 12, 12);

    m_ocrTextEditContainer->setLayout(m_ocrTextEditHLayout);
    style_changed();

    m_toolbarWidget->ocrOtherIconState(true);
    m_ocrVLayout->setSpacing(0);
    m_ocrVLayout->addSpacing(12);
    m_ocrVLayout->addLayout(m_ocrImageHLayout);
    m_ocrVLayout->addSpacing(12);
    m_ocrVLayout->addStretch();
    m_ocrVLayout->addWidget(m_ocrTextEditContainer);
    m_ocrVLayout->addSpacing(12);
    m_ocrVLayout->addStretch();
    m_ocrVLayout->addWidget(m_toolbarWidget, 0, Qt::AlignCenter |Qt::AlignVCenter |Qt::AlignHCenter);
    m_ocrVLayout->addSpacing(17);
    m_ocrVLayout->setContentsMargins(20, 0, 20, 0);

    this->setLayout(m_ocrVLayout);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &showOcrWidget::showScanLine);
}

bool showOcrWidget::isDarkTheme()
{
    QString systemTheme = kdk::GsettingMonitor::getSystemTheme().toString();

    if (systemTheme == STYLE_NAME_KEY_DARK || systemTheme == STYLE_NAME_KEY_BLACK) {
        return true;
    } else {
        return false;
    }
}

void showOcrWidget::initConnect()
{
    connect(g_user_signal, &GlobalUserSignal::showScanWidgetSignal, this, &showOcrWidget::startScanSlot);
    connect(g_user_signal, &GlobalUserSignal::toolbarOcrOperationFinishedSignal, this, &showOcrWidget::updateOcrTextEdit);
    connect(g_user_signal, &GlobalUserSignal::stopOcrTimerSignal, this, &showOcrWidget::stopScanStop);
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this, &showOcrWidget::style_changed);
}
void showOcrWidget::style_changed(){
     QPalette pal(m_ocrTextEditContainer->palette());
    if(isDarkTheme()){
        pal.setColor(QPalette::Background,Qt::black);

    }else{
        pal.setColor(QPalette::Background,Qt::white);
    }
    m_ocrTextEditContainer->setAutoFillBackground(true);
    m_ocrTextEditContainer->setPalette(pal);

}

void showOcrWidget::setOcrText()
{
    m_ocrTextEdit->setText(g_sane_object->ocrOutputText);
}

void showOcrWidget::resizeEvent(QResizeEvent *event)
{
    // todo: auto update size of m_ocrImageLabel, m_ocrTextEdit, m_toolbarWidget

    QWidget::resizeEvent(event);
}
void showOcrWidget::showScanLine()
{
//    qDebug() << "showScanLine";
    scanHeight += 1;

    if (scanHeight >=m_ocrImageLabel->height()) {
        scanHeight = 0;
    }

    QPixmap pix;
    pix.load(pixFileName);
    pix = pix.scaled(m_ocrImageLabel->width(), m_ocrImageLabel->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_ocrImageLabel->setPixmap(pix);

    QPixmap pixScan(":/scan-line.png");
    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::transparent);
    painter.drawPixmap(0, scanHeight, m_ocrImageLabel->width(), 30, pixScan);

    m_ocrImageLabel->setPixmap(pix);
}

void showOcrWidget::loadScanningPicture()
{
    QPixmap pix;
    pix.load(pixFileName);
    pix = pix.scaled(m_ocrImageLabel->width(), m_ocrImageLabel->height(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_ocrImageLabel->setPixmap(pix);
}

void showOcrWidget::startScanSlot(QString pic_path)
{
    if (g_sane_object->cropFlag == 1) {
        return;
    }
    pixFileName = pic_path;
    qDebug() << "start ocr scan";
    m_ocrTextEdit->setText(tr("The document is in character recognition ..."));
    timer->start(10);
    scanHeight = 0;

}

void showOcrWidget::stopScanStop()
{
    m_ocrTextEdit->setText(tr("The document is in character recognition ..."));
    timer->stop();
    scanHeight = 0;

    loadScanningPicture();
}

void showOcrWidget::updateOcrTextEdit()
{
    qDebug() << "ocrTextEdit = " << g_sane_object->ocrOutputText;
    timer->stop();
    scanHeight = 0;

    loadScanningPicture();

    emit updateTextEdit();
//    m_ocrTextEdit->setText(g_sane_object->ocrOutputText);

}
