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
#include "watermarkdialog.h"
#include "Qt/windowmanage.hpp"
#include <kysdk/applications/gsettingmonitor.h>

WatermarkDialog::WatermarkDialog(QWidget *parent) : QDialog(parent)
  , m_closeButton (new QPushButton())
  , m_titleHBoxLayout (new QHBoxLayout())
  , m_infoTextLabel (new QLabel())
  , m_watermarkLineEdit (new QLineEdit())
  , m_watermarkHBoxLayout (new QHBoxLayout())
  , m_cancelButton (new kdk::KPushButton())
  , m_confirmButton (new kdk::KPushButton())
  , m_buttonsHBoxLayout (new QHBoxLayout())
  , m_mainVBoxLayout (new QVBoxLayout())
{
    m_themeData = new QGSettings(UKUI_THEME_GSETTING_PATH);

    installEventFilter(this);
    initWindow();

    initLayout();

    initConnect();
}

void WatermarkDialog::initWindow()
{
    WindowManage::removeHeader(this);

    setWindowTitle(tr("Scanner"));
    setFixedSize(QSize(380, 176));
}

void WatermarkDialog::initLayout()
{

    m_closeButton->setIcon(QIcon::fromTheme(ICON_THEME_CLOSE));
    m_closeButton->setToolTip(tr("Close"));
    m_closeButton->setFixedSize(30, 30);
    m_closeButton->setIconSize (QSize(16, 16));
    m_closeButton->setProperty("isWindowButton", 0x2);
    m_closeButton->setProperty("useIconHighlightEffect", 0x8);
    m_closeButton->setFlat(true);

    m_titleHBoxLayout->addStretch();
    m_titleHBoxLayout->addWidget(m_closeButton);
    m_titleHBoxLayout->setContentsMargins(0, 5, 4, 5);

    m_infoTextLabel->setText(tr("Add watermark"));
    QFont ft;
    ft.setPixelSize(14);
    m_infoTextLabel->setFont(ft);
    m_infoTextLabel->setFixedSize(QSize(56, 22));

    m_watermarkLineEdit->setText("");
    m_watermarkLineEdit->setFixedSize(QSize(268, 36));

    m_watermarkHBoxLayout->setSpacing(0);
    m_watermarkHBoxLayout->addSpacing(24);
    m_watermarkHBoxLayout->addWidget(m_infoTextLabel);
    m_watermarkHBoxLayout->addSpacing(8);
    m_watermarkHBoxLayout->addWidget(m_watermarkLineEdit);
    m_watermarkHBoxLayout->setContentsMargins(0, 0, 24, 0);

    m_cancelButton->setText(tr("Cancel"));
    m_cancelButton->setMinimumSize(QSize(96, 36));

    m_confirmButton->setText(tr("Confirm"));
    m_confirmButton->setStyleSheet("color:white;");
    m_confirmButton->setBackgroundColorHighlight(true);
    m_confirmButton->setMinimumSize(QSize(96, 36));

    m_buttonsHBoxLayout->setSpacing(0);
    m_buttonsHBoxLayout->addWidget(m_cancelButton);
    m_buttonsHBoxLayout->addSpacing(12);
    m_buttonsHBoxLayout->addWidget(m_confirmButton);
    m_buttonsHBoxLayout->setAlignment(Qt::AlignRight);
    m_buttonsHBoxLayout->setContentsMargins(0, 0, 24, 24);

    m_mainVBoxLayout->setSpacing(0);
    m_mainVBoxLayout->addLayout(m_titleHBoxLayout);
    m_mainVBoxLayout->addSpacing(16);
    m_mainVBoxLayout->addLayout(m_watermarkHBoxLayout);
    m_mainVBoxLayout->addSpacing(24);
    m_mainVBoxLayout->addLayout(m_buttonsHBoxLayout);
    m_mainVBoxLayout->setContentsMargins(0, 0, 0, 0);

    this->setLayout(m_mainVBoxLayout);
    m_confirmButton->setDefault(true);

}

void WatermarkDialog::initConnect()
{
    connect(m_closeButton, &QPushButton::clicked, [=](){
        this->close();
    });
    connect(m_cancelButton, &QPushButton::clicked,  [=](){
        this->close();
    });
    connect(m_confirmButton, &QPushButton::clicked, this, &WatermarkDialog::confirmButtonClickedSlot);
    connect(m_themeData, &QGSettings::changed, this, &WatermarkDialog::fontSizeChangedSlot);

}

QString WatermarkDialog::getLineEditText()
{
    return m_watermarkLineEdit->text();
}

void WatermarkDialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect(), 0, 0);
    QStyleOption opt;
    opt.init(this);

    QColor mainColor;
    mainColor = opt.palette.color(QPalette::Window);

    p.fillPath(rectPath, QBrush(mainColor));
    p.end();
}

void WatermarkDialog::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        m_confirmButton->click();
        break;
    case Qt::Key_Escape:
        m_cancelButton->click();
        break;
    }
}

void WatermarkDialog::confirmButtonClickedSlot()
{
    if (m_watermarkLineEdit->text() == "") {
        this->reject();
    } else {
        this->accept();
    }

}

void WatermarkDialog::fontSizeChangedSlot()
{
    float systemFontSize = kdk::GsettingMonitor::getSystemFontSize().toFloat();
    QString fontType = m_themeData->get("systemFont").toString();
    QFont font(fontType, systemFontSize);
    m_confirmButton->setFont(font);
}
