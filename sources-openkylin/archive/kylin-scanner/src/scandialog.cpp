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
#include "scandialog.h"
#include <kysdk/applications/gsettingmonitor.h>
#include "Qt/windowmanage.hpp"
#include "globalsignal.h"
#include "include/theme.h"

ScanDialog::ScanDialog(QWidget *parent) : QDialog(parent),
    m_titleLabel(new QLabel()),
    m_closeButton(new QPushButton()),
    m_iconLabel(new QLabel()),
    m_msgLabel(new QLabel()),
    m_cancelButton(new CustomPushButton()),
    m_titleWidget(new QWidget(this)),
    m_titleHBoxLayout(new QHBoxLayout(m_titleWidget)),
    m_msgWidget(new QWidget(this)),
    m_msgHBoxLayout(new QHBoxLayout(m_msgWidget)),
    m_cancelWidget(new QWidget(this)),
    m_cancelHBoxLayout(new QHBoxLayout(m_cancelWidget)),
    m_mainVBoxLayout(new QVBoxLayout(this))
{
    setupGui();
    initConnect();
}

void ScanDialog::setupGui()
{
    WindowManage::removeHeader(this);
    setWindowModality(Qt::ApplicationModal);
    this->setWindowTitle(QApplication::tr("Scanner"));
    this->setFixedSize(ScanDialogWindowSize);

    if(isDarkTheme()){
        QPalette pal(palette());
        pal.setColor(QPalette::Background, QColor(0, 0, 0));
        setAutoFillBackground(true);
        setPalette(pal);
    }else{
        QPalette pal(palette());
        pal.setColor(QPalette::Background, QColor(255, 255, 255));
        setAutoFillBackground(true);
        setPalette(pal);
    }

    m_titleLabel->setText(tr("Scanner"));
    m_titleLabel->setFixedHeight(ScanDialogLabelHeight);

    m_closeButton->setIcon(QIcon::fromTheme ("window-close-symbolic"));
    m_closeButton->setToolTip(tr("Close"));
    m_closeButton->setFixedSize(30, 30);
    m_closeButton->setIconSize(QSize(16, 16));
    m_closeButton->setProperty("isWindowButton", 0x2);
    m_closeButton->setProperty("useIconHighlightEffect", 0x8);
    m_closeButton->setFlat(true);
    m_titleWidget->setFixedHeight(34);

    m_titleHBoxLayout->addSpacing(12);
    m_titleHBoxLayout->addWidget(m_titleLabel);
    m_titleHBoxLayout->addStretch();
    m_titleHBoxLayout->addWidget(m_closeButton);
    m_titleHBoxLayout->setContentsMargins(0, 4, 4, 4);
    m_titleHBoxLayout->setAlignment(Qt::AlignCenter);
    m_titleWidget->setLayout(m_titleHBoxLayout);

    m_iconLabel->setPixmap(QPixmap(":/scanIcon.svg"));
    m_iconLabel->setFixedSize(ScanDialogIconSize);

    m_pageNumber = 1;
    m_scanMsg = tr("Number of pages scanning: ") + QString::number(m_pageNumber);
    m_msgLabel->setText(m_scanMsg);
    m_msgLabel->setFixedHeight(ScanDialogLabelHeight);
    m_msgWidget->setFixedHeight(40);

    m_msgHBoxLayout->addSpacing(24);
    m_msgHBoxLayout->addWidget(m_iconLabel);
    m_msgHBoxLayout->addSpacing(8);
    m_msgHBoxLayout->addWidget(m_msgLabel);
    m_msgHBoxLayout->addStretch();
    m_msgWidget->setLayout(m_msgHBoxLayout);

    m_cancelButton->setText(tr("Cancel"));
    m_cancelButton->setFixedSize(ScanDialogButtonSize);
    m_cancelButton->clearFocus();
    m_cancelWidget->setFixedHeight(45);

    m_cancelHBoxLayout->addSpacing(220);
    m_cancelHBoxLayout->addWidget(m_cancelButton);
    m_cancelWidget->setLayout(m_cancelHBoxLayout); 
    m_mainVBoxLayout->addWidget(m_titleWidget);
    m_mainVBoxLayout->addStretch();
    m_mainVBoxLayout->addWidget(m_msgWidget);
    m_mainVBoxLayout->addSpacing(32);
    m_mainVBoxLayout->addStretch();
    m_mainVBoxLayout->addWidget(m_cancelWidget);
    m_mainVBoxLayout->addSpacing(24);
    m_mainVBoxLayout->setContentsMargins(0, 0, 0, 20);


    this->setLayout(m_mainVBoxLayout);
    m_cancelButton->setDefault(true);
    m_cancelButton->setFocus();
}

bool ScanDialog::isDarkTheme()
{
    QString systemTheme = kdk::GsettingMonitor::getSystemTheme().toString();

    if (systemTheme == STYLE_NAME_KEY_DARK || systemTheme == STYLE_NAME_KEY_BLACK) {
        return true;
    } else {
        return false;
    }
}
void ScanDialog::initConnect()
{
    connect(kdk::GsettingMonitor::getInstance(), &kdk::GsettingMonitor::systemThemeChange, this, [=]{
        if(isDarkTheme()){
            this->setStyleSheet("background: #262626;");
        }else{
            this->setStyleSheet("background: #FFFFFF;");
        }
    });

    connect(m_closeButton, &QPushButton::clicked, this, [=](){
        this->reject();
    });

    connect(m_cancelButton, &QPushButton::clicked, this, [=]{
        cancelOpration();
        emit g_user_signal->cancelScanning();

    });
    connect(g_sane_object, &SaneObject::updatePageNum, this, &ScanDialog::updatePageNumberWhileScanning);
    connect(g_user_signal, &GlobalUserSignal::cancelOprationOKSignal, this, &ScanDialog::updatePageNumberWhileStopScanning);
}
void ScanDialog::cancelOpration(){
    m_msgLabel->setText(tr("Canceling scan，please wait a moment!"));
    m_cancelButton->hide();
}
int ScanDialog::pageNumber() const
{
    return m_pageNumber;
}

void ScanDialog::setPageNumber(int pageNumber)
{
    m_pageNumber = pageNumber;
}

QString ScanDialog::scanMsg() const
{
    return m_scanMsg;
}

void ScanDialog::setScanMsg(const QString &scanMsg)
{
    m_scanMsg = scanMsg;
}

void ScanDialog::updatePageNumberWhileScanning()
{
    QString text = g_sane_object->userInfo.pageNumber;

    if (text.compare("Multiple", Qt::CaseInsensitive) == 0
            || text.compare("多页扫描", Qt::CaseInsensitive) == 0
            || text.compare(tr("Multiple"), Qt::CaseInsensitive) == 0) {
        m_pageNumber += 1;
        m_scanMsg = tr("Number of pages scanning: ") + QString::number(m_pageNumber);
        m_msgLabel->setText(m_scanMsg);
    } else {
       // this->close();
    }
}

void ScanDialog::updatePageNumberWhileStopScanning()
{
    m_pageNumber = 1;
    this->close();

    g_user_signal->stopOrFinishedScan();

}
