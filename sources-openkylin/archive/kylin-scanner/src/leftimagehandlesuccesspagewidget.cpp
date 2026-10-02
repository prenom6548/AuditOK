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
#include "leftimagehandlesuccesspagewidget.h"

LeftImageHandleSuccessPageWidget::LeftImageHandleSuccessPageWidget(QWidget *parent) : QWidget(parent),
    m_thumbnailWidget(new ThumbnailWidget(this)),
    m_showImageWidget(new ShowImageWidget(this)),
    m_showOcrWidget(new showOcrWidget(this)),
    m_showImageOrOcrStackWidget(new QStackedWidget(this)),
    m_leftImageHandleSuccessPageHLayout(new QHBoxLayout(this))
{
    setupGui();
    initConnect();
}

void LeftImageHandleSuccessPageWidget::setupGui()
{
    setMinimumSize(LeftImageHandleSuccessPageWidth, LeftImageHandleSuccessPageHeight);

    m_showImageOrOcrStackWidget->addWidget(m_showImageWidget);
    m_showImageOrOcrStackWidget->addWidget(m_showOcrWidget);
    m_showOcrWidget->setParent(m_showImageOrOcrStackWidget);
    m_showImageOrOcrStackWidget->setCurrentWidget(m_showImageWidget);

    m_leftImageHandleSuccessPageHLayout->setSpacing(0);
    m_leftImageHandleSuccessPageHLayout->addWidget(m_thumbnailWidget);
    m_leftImageHandleSuccessPageHLayout->addWidget(m_showImageOrOcrStackWidget);
    m_leftImageHandleSuccessPageHLayout->setContentsMargins(0, 0, 0, 0);

    setLayout(m_leftImageHandleSuccessPageHLayout);
}

void LeftImageHandleSuccessPageWidget::initConnect()
{
    connect(g_user_signal, &GlobalUserSignal::switchPageSig, this, &LeftImageHandleSuccessPageWidget::showOcrWidgetSlot);
    connect(m_showOcrWidget, &showOcrWidget::updateTextEdit, this, &LeftImageHandleSuccessPageWidget::updatOcrText);
    connect(g_user_signal, &GlobalUserSignal::exitOcrWhenScanSignal, [=](){
        if(m_showImageOrOcrStackWidget->currentIndex() != 0){
            m_showImageOrOcrStackWidget->setCurrentWidget(m_showImageWidget);
        }
    });
}

void LeftImageHandleSuccessPageWidget::showOcrWidgetSlot()
{
    if (g_sane_object->ocrFlag == 0) {
        m_showImageOrOcrStackWidget->setCurrentWidget(m_showOcrWidget);
        qDebug() << "After switch to ocr page, ocrFlag = " << g_sane_object->ocrFlag;
    } else {
        m_showImageOrOcrStackWidget->setCurrentWidget(m_showImageWidget);
        g_user_signal->stopOcrTimer();
    }
}

void LeftImageHandleSuccessPageWidget::showImageWidgetSlot()
{
    m_showImageOrOcrStackWidget->setCurrentWidget(m_showImageWidget);

    g_user_signal->stopOcrTimer();
}

void LeftImageHandleSuccessPageWidget::updatOcrText()
{
    m_showOcrWidget->setOcrText();
}
