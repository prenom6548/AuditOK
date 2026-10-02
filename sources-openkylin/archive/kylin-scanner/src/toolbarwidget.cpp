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
#include "toolbarwidget.h"
#include <buried_point.hpp>
#include <kysdk/applications/gsettingmonitor.h>

#include "include/theme.h"
#include <QGraphicsDropShadowEffect>
#include <QMessageBox>
#include "globalsignal.h"
#include "buriedpoint.hpp"
#include <libkydatacollect.h>
#include <QPainterPath>

ToolBarWidget::ToolBarWidget(QWidget *parent) : QWidget(parent),
    m_beautyButton(new kdk::KBorderlessButton),
    m_rectifyButton(new kdk::KBorderlessButton),
    m_ocrButton(new kdk::KBorderlessButton),
    m_leftFrame(new QFrame),
    m_cropButton(new kdk::KBorderlessButton),
    m_rotateButton(new kdk::KBorderlessButton),
    m_mirrorButton(new kdk::KBorderlessButton),
    m_watermarkButton(new kdk::KBorderlessButton),
    m_rightFrame(new QFrame),
    m_zoomOutButton(new kdk::KBorderlessButton),
    m_percentageLabel(new QLabel),
    m_zoomInButton(new kdk::KBorderlessButton),
    m_mutex(new QMutex()),
    m_mainHLayout(new QHBoxLayout(this))
{
    setupGui();
    initConnect();
}

void ToolBarWidget::setupGui()
{
    m_beautyButton->setObjectName("m_beautyButton");
    m_rectifyButton->setObjectName("m_rectifyButton");
    m_ocrButton->setObjectName("m_ocrButton");
    m_leftFrame->setObjectName("m_leftFrame");
    m_cropButton->setObjectName("m_cropButton");
    m_rotateButton->setObjectName("m_rotateButton");
    m_mirrorButton->setObjectName("m_mirrorButton");
    m_watermarkButton->setObjectName("m_watermarkButton");
    m_zoomOutButton->setObjectName("m_zoomOutButton");
    m_zoomInButton->setObjectName("m_zoomInButton");
    this->setAttribute(Qt::WA_TranslucentBackground); // set background translucent，combine with overriding paintEvent to let setStyleSheet work
    this->setWindowFlags(Qt::FramelessWindowHint);   // set frameless window
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);
    deviceMode currentMode = m_mode.defaultModeCapture();
    if(currentMode != PCMode){
        this->setFixedSize(ToolBarWidgetTableModeSize);

        m_beautyButton->setIcon(QIcon::fromTheme(":/toolbar/beauty.svg"));
        m_beautyButton->setFixedSize(ToolBarWidgetTableModeButtonSize);
        m_beautyButton->setIconSize(ToolBarWidgetTableModeButtonSize);
        m_beautyButton->setToolTip(tr("Beauty"));
        m_beautyButton->setFocusPolicy(Qt::NoFocus);

        m_rectifyButton->setIcon(QIcon::fromTheme(":/toolbar/rectify.svg"));
        m_rectifyButton->setFixedSize(ToolBarWidgetTableModeButtonSize);
        m_rectifyButton->setIconSize(ToolBarWidgetTableModeButtonSize);
        m_rectifyButton->setToolTip(tr("Rectify"));
        m_rectifyButton->setFocusPolicy(Qt::NoFocus);

        m_ocrButton->setIcon(QIcon::fromTheme(":/toolbar/ocr.svg"));
        m_ocrButton->setFixedSize(ToolBarWidgetTableModeButtonSize);
        m_ocrButton->setIconSize(ToolBarWidgetTableModeButtonSize);
        m_ocrButton->setToolTip(tr("OCR"));
        m_ocrButton->setFocusPolicy(Qt::NoFocus);

        m_leftFrame->setFixedSize(ToolBarWidgetFrameSize);

        m_cropButton->setIcon(QIcon::fromTheme(":/toolbar/crop.svg"));
        m_cropButton->setFixedSize(ToolBarWidgetTableModeButtonSize);
        m_cropButton->setIconSize(ToolBarWidgetTableModeButtonSize);
        m_cropButton->setToolTip(tr("Crop"));
        m_cropButton->setFocusPolicy(Qt::NoFocus);

        m_rotateButton->setIcon(QIcon::fromTheme(":/toolbar/rotate.svg"));
        m_rotateButton->setFixedSize(ToolBarWidgetTableModeButtonSize);
        m_rotateButton->setIconSize(ToolBarWidgetTableModeButtonSize);
        m_rotateButton->setToolTip(tr("Rotate"));
        m_rotateButton->setFocusPolicy(Qt::NoFocus);

        m_mirrorButton->setIcon(QIcon::fromTheme(":/toolbar/mirror.svg"));
        m_mirrorButton->setFixedSize(ToolBarWidgetTableModeButtonSize);
        m_mirrorButton->setIconSize(ToolBarWidgetTableModeButtonSize);
        m_mirrorButton->setToolTip(tr("Mirror"));
        m_mirrorButton->setFocusPolicy(Qt::NoFocus);

        m_watermarkButton->setIcon(QIcon::fromTheme(":/toolbar/watermark.svg"));
        m_watermarkButton->setFixedSize(ToolBarWidgetTableModeButtonSize);
        m_watermarkButton->setIconSize(ToolBarWidgetTableModeButtonSize);
        m_watermarkButton->setToolTip(tr("Watermark"));
        m_watermarkButton->setFocusPolicy(Qt::NoFocus);

        m_rightFrame->setFixedSize(ToolBarWidgetFrameSize);

        QFont m_percentageFontSize;
        m_percentageFontSize.setPixelSize(14);
        m_percentageLabel->setText("100%");
        m_percentageLabel->setFont(m_percentageFontSize);

        m_zoomOutButton->setIcon(QIcon::fromTheme(":/toolbar/zoomout.svg"));
        m_zoomOutButton->setFixedSize(ToolBarWidgetTableModeZoomButtonSize);
        m_zoomOutButton->setIconSize(ToolBarWidgetTableModeZoomButtonSize);
        m_zoomOutButton->setToolTip(tr("ZoomOut"));
        m_zoomOutButton->setFocusPolicy(Qt::NoFocus);

        m_zoomInButton->setIcon(QIcon::fromTheme(":/toolbar/zoomin.svg"));
        m_zoomInButton->setFixedSize(ToolBarWidgetTableModeZoomButtonSize);
        m_zoomInButton->setIconSize(ToolBarWidgetTableModeZoomButtonSize);
        m_zoomInButton->setToolTip(tr("ZoomIn"));
        m_zoomInButton->setFocusPolicy(Qt::NoFocus);

        m_mainHLayout->setSpacing(0);
        m_mainHLayout->addWidget(m_beautyButton);
        m_mainHLayout->addSpacing(4);
        m_mainHLayout->addWidget(m_rectifyButton);
        m_mainHLayout->addSpacing(4);
        m_mainHLayout->addWidget(m_ocrButton);
        m_mainHLayout->addSpacing(8);
        m_mainHLayout->addWidget(m_leftFrame);
        m_mainHLayout->addSpacing(4);
        m_mainHLayout->addWidget(m_cropButton);
        m_mainHLayout->addSpacing(4);
        m_mainHLayout->addWidget(m_rotateButton);
        m_mainHLayout->addSpacing(4);
        m_mainHLayout->addWidget(m_mirrorButton);
        m_mainHLayout->addSpacing(3);
        m_mainHLayout->addWidget(m_watermarkButton);
        m_mainHLayout->addSpacing(8);
        m_mainHLayout->addWidget(m_rightFrame);
        m_mainHLayout->addSpacing(16);
        m_mainHLayout->addWidget(m_zoomOutButton, 0, Qt::AlignCenter);
        m_mainHLayout->addSpacing(6);
        m_mainHLayout->addWidget(m_percentageLabel, 0, Qt::AlignCenter);
        m_mainHLayout->addSpacing(4);
        m_mainHLayout->addWidget(m_zoomInButton, 0, Qt::AlignCenter);
        m_mainHLayout->setContentsMargins(8, 0, 16, 0);

        this->setLayout(m_mainHLayout);
    }else{
    this->setMinimumSize(ToolBarWidgetFixedSize);

    m_beautyButton->setIcon(QIcon::fromTheme(":/toolbar/beauty.svg"));
    m_beautyButton->setFixedSize(ToolBarWidgetButtonSize);
    m_beautyButton->setIconSize(ToolBarWidgetButtonSize);
    m_beautyButton->setToolTip(tr("Beauty"));
    m_beautyButton->setFocusPolicy(Qt::NoFocus);

    m_rectifyButton->setIcon(QIcon::fromTheme(":/toolbar/rectify.svg"));
    m_rectifyButton->setFixedSize(ToolBarWidgetButtonSize);
    m_rectifyButton->setIconSize(ToolBarWidgetButtonSize);
    m_rectifyButton->setToolTip(tr("Rectify"));
    m_rectifyButton->setFocusPolicy(Qt::NoFocus);

    m_ocrButton->setIcon(QIcon::fromTheme(":/toolbar/ocr.svg"));
    m_ocrButton->setFixedSize(ToolBarWidgetButtonSize);
    m_ocrButton->setIconSize(ToolBarWidgetButtonSize);
    m_ocrButton->setToolTip(tr("OCR"));
    m_ocrButton->setFocusPolicy(Qt::NoFocus);

    m_leftFrame->setFixedSize(ToolBarWidgetFrameSize);

    m_cropButton->setIcon(QIcon::fromTheme(":/toolbar/crop.svg"));
    m_cropButton->setFixedSize(ToolBarWidgetButtonSize);
    m_cropButton->setIconSize(ToolBarWidgetButtonSize);
    m_cropButton->setToolTip(tr("Crop"));
    m_cropButton->setFocusPolicy(Qt::NoFocus);

    m_rotateButton->setIcon(QIcon::fromTheme(":/toolbar/rotate.svg"));
    m_rotateButton->setFixedSize(ToolBarWidgetButtonSize);
    m_rotateButton->setIconSize(ToolBarWidgetButtonSize);
    m_rotateButton->setToolTip(tr("Rotate"));
    m_rotateButton->setFocusPolicy(Qt::NoFocus);

    m_mirrorButton->setIcon(QIcon::fromTheme(":/toolbar/mirror.svg"));
    m_mirrorButton->setFixedSize(ToolBarWidgetButtonSize);
    m_mirrorButton->setIconSize(ToolBarWidgetButtonSize);
    m_mirrorButton->setToolTip(tr("Mirror"));
    m_mirrorButton->setFocusPolicy(Qt::NoFocus);

    m_watermarkButton->setIcon(QIcon::fromTheme(":/toolbar/watermark.svg"));
    m_watermarkButton->setFixedSize(ToolBarWidgetButtonSize);
    m_watermarkButton->setIconSize(ToolBarWidgetButtonSize);
    m_watermarkButton->setToolTip(tr("Watermark"));
    m_watermarkButton->setFocusPolicy(Qt::NoFocus);

    m_rightFrame->setFixedSize(ToolBarWidgetFrameSize);

    QFont m_percentageFontSize;
    m_percentageFontSize.setPixelSize(12);
    m_percentageLabel->setText("100%");
    m_percentageLabel->setFont(m_percentageFontSize);

    m_zoomOutButton->setIcon(QIcon::fromTheme(":/toolbar/zoomout.svg"));
    m_zoomOutButton->setFixedSize(ToolBarWidgetZoomButtonSize);
    m_zoomOutButton->setIconSize(ToolBarWidgetZoomButtonSize);
    m_zoomOutButton->setToolTip(tr("ZoomOut"));
    m_zoomOutButton->setFocusPolicy(Qt::NoFocus);

    m_zoomInButton->setIcon(QIcon::fromTheme(":/toolbar/zoomin.svg"));
    m_zoomInButton->setFixedSize(ToolBarWidgetZoomButtonSize);
    m_zoomInButton->setIconSize(ToolBarWidgetZoomButtonSize);
    m_zoomInButton->setToolTip(tr("ZoomIn"));
    m_zoomInButton->setFocusPolicy(Qt::NoFocus);


    m_mainHLayout->setSpacing(0);
    m_mainHLayout->addWidget(m_beautyButton);
    m_mainHLayout->addSpacing(4);
    m_mainHLayout->addWidget(m_rectifyButton);
    m_mainHLayout->addSpacing(4);
    m_mainHLayout->addWidget(m_ocrButton);
    m_mainHLayout->addSpacing(8);
    m_mainHLayout->addWidget(m_leftFrame);
    m_mainHLayout->addSpacing(4);
    m_mainHLayout->addWidget(m_cropButton);
    m_mainHLayout->addSpacing(4);
    m_mainHLayout->addWidget(m_rotateButton);
    m_mainHLayout->addSpacing(4);
    m_mainHLayout->addWidget(m_mirrorButton);
    m_mainHLayout->addSpacing(3);
    m_mainHLayout->addWidget(m_watermarkButton);
    m_mainHLayout->addSpacing(8);
    m_mainHLayout->addWidget(m_rightFrame);
    m_mainHLayout->addSpacing(16);
    m_mainHLayout->addWidget(m_zoomOutButton, 0, Qt::AlignCenter);
    m_mainHLayout->addSpacing(6);
    m_mainHLayout->addWidget(m_percentageLabel, 0, Qt::AlignCenter);
    m_mainHLayout->addSpacing(4);
    m_mainHLayout->addWidget(m_zoomInButton, 0, Qt::AlignCenter);
    m_mainHLayout->setContentsMargins(8, 0, 16, 0);

        this->setLayout(m_mainHLayout);
    }

    drawFrame();
    qDebug() << "toolbarwidget size: " << this->size();
}

void ToolBarWidget::initConnect()
{
    connect(g_user_signal, &GlobalUserSignal::toolbarPercentageChangedSignel, this, &ToolBarWidget::percentageChangedSlot);
    connect(m_beautyButton, &QPushButton::clicked, this, &ToolBarWidget::beautyButtonClickedSlot);
    connect(m_rectifyButton, &QPushButton::clicked, this, &ToolBarWidget::rectifyButtonClickedSlot);
    connect(m_ocrButton, &QPushButton::clicked, this, &ToolBarWidget::ocrButtonClickedSlot);
    connect(m_cropButton, &QPushButton::clicked, this, &ToolBarWidget::cropButtonClickedSlot);
    connect(m_rotateButton, &QPushButton::clicked, this, &ToolBarWidget::rotateButtonClickedSlot);
    connect(m_mirrorButton, &QPushButton::clicked, this, &ToolBarWidget::mirrorButtonClickedSlot);
    connect(m_watermarkButton, &QPushButton::clicked, this, &ToolBarWidget::watermarkButtonClickedSlot);
    connect(m_zoomOutButton, &QPushButton::clicked, this, &ToolBarWidget::zoomoutButtonClickedSlot);
    connect(m_zoomInButton, &QPushButton::clicked, this, &ToolBarWidget::zoominButtonClickedSlot);
    connect(GlobalUserSignal::getInstance(), &GlobalUserSignal::rotationChangedSig, this, &ToolBarWidget::rotateChangedSlot);
}

void ToolBarWidget::rotateChangedSlot(bool isPCMode)
{
    if(!isPCMode){
        this->setFixedSize(ToolBarWidgetTableModeSize);

        m_beautyButton->setFixedSize(ToolBarWidgetTableModeButtonSize);
        m_beautyButton->setIconSize(ToolBarWidgetTableModeButtonSize);

        m_rectifyButton->setFixedSize(ToolBarWidgetTableModeButtonSize);
        m_rectifyButton->setIconSize(ToolBarWidgetTableModeButtonSize);

        m_ocrButton->setFixedSize(ToolBarWidgetTableModeButtonSize);
        m_ocrButton->setIconSize(ToolBarWidgetTableModeButtonSize);

        m_leftFrame->setFixedSize(ToolBarWidgetFrameSize);

        m_cropButton->setFixedSize(ToolBarWidgetTableModeButtonSize);
        m_cropButton->setIconSize(ToolBarWidgetTableModeButtonSize);

        m_rotateButton->setFixedSize(ToolBarWidgetTableModeButtonSize);
        m_rotateButton->setIconSize(ToolBarWidgetTableModeButtonSize);

        m_mirrorButton->setFixedSize(ToolBarWidgetTableModeButtonSize);
        m_mirrorButton->setIconSize(ToolBarWidgetTableModeButtonSize);

        m_watermarkButton->setFixedSize(ToolBarWidgetTableModeButtonSize);
        m_watermarkButton->setIconSize(ToolBarWidgetTableModeButtonSize);

        m_rightFrame->setFixedSize(ToolBarWidgetFrameSize);

        QFont m_percentageFontSize;
        m_percentageFontSize.setPixelSize(12);
        m_percentageLabel->setFont(m_percentageFontSize);

        m_zoomOutButton->setFixedSize(ToolBarWidgetTableModeZoomButtonSize);
        m_zoomOutButton->setIconSize(ToolBarWidgetTableModeZoomButtonSize);

        m_zoomInButton->setFixedSize(ToolBarWidgetTableModeZoomButtonSize);
        m_zoomInButton->setIconSize(ToolBarWidgetTableModeZoomButtonSize);
    }else{
        this->setFixedSize(ToolBarWidgetFixedSize);

        m_beautyButton->setFixedSize(ToolBarWidgetButtonSize);
        m_beautyButton->setIconSize(ToolBarWidgetButtonSize);

        m_rectifyButton->setFixedSize(ToolBarWidgetButtonSize);
        m_rectifyButton->setIconSize(ToolBarWidgetButtonSize);

        m_ocrButton->setFixedSize(ToolBarWidgetButtonSize);
        m_ocrButton->setIconSize(ToolBarWidgetButtonSize);

        m_leftFrame->setFixedSize(ToolBarWidgetFrameSize);

        m_cropButton->setFixedSize(ToolBarWidgetButtonSize);
        m_cropButton->setIconSize(ToolBarWidgetButtonSize);

        m_rotateButton->setFixedSize(ToolBarWidgetButtonSize);
        m_rotateButton->setIconSize(ToolBarWidgetButtonSize);

        m_mirrorButton->setFixedSize(ToolBarWidgetButtonSize);
        m_mirrorButton->setIconSize(ToolBarWidgetButtonSize);

        m_watermarkButton->setFixedSize(ToolBarWidgetButtonSize);
        m_watermarkButton->setIconSize(ToolBarWidgetButtonSize);

        m_rightFrame->setFixedSize(ToolBarWidgetFrameSize);

        QFont m_percentageFontSize;
        m_percentageFontSize.setPixelSize(14);
        m_percentageLabel->setFont(m_percentageFontSize);

        m_zoomOutButton->setFixedSize(ToolBarWidgetZoomButtonSize);
        m_zoomOutButton->setIconSize(ToolBarWidgetZoomButtonSize);

        m_zoomInButton->setFixedSize(ToolBarWidgetZoomButtonSize);
        m_zoomInButton->setIconSize(ToolBarWidgetZoomButtonSize);
    }
}

void ToolBarWidget::drawFrame()
{
    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(this);
    effect->setOffset(0, 0); //设置向哪个方向产生阴影效果(dx,dy)，特别地，(0,0)代表向四周发散
    effect->setColor(QColor(220,220,220));       //设置阴影颜色，也可以setColor()
    effect->setBlurRadius(8); //设定阴影的模糊半径，数值越大越模糊
    this->setGraphicsEffect(effect);
}

void ToolBarWidget::ocrOtherIconState(bool flag)
{
    m_beautyButton->setDisabled(flag);
    m_rectifyButton->setDisabled(flag);
    m_cropButton->setDisabled(flag);
    m_rotateButton->setDisabled(flag);
    m_mirrorButton->setDisabled(flag);
    m_watermarkButton->setDisabled(flag);
    m_zoomOutButton->setDisabled(flag);
    m_zoomInButton->setDisabled(flag);
    m_percentageLabel->setDisabled(flag);
}

void ToolBarWidget::cropOtherIconState(bool flag)
{
    m_beautyButton->setDisabled(flag);
    m_rectifyButton->setDisabled(flag);
    m_ocrButton->setDisabled(flag);
    m_rotateButton->setDisabled(flag);
    m_mirrorButton->setDisabled(flag);
    m_watermarkButton->setDisabled(flag);
    m_zoomOutButton->setDisabled(flag);
    m_zoomInButton->setDisabled(flag);
    m_percentageLabel->setDisabled(flag);
}

/**
 * @brief ToolBarWidget::paintEvent
 * @param event
 * Basically, it only support these stylesheet attributes for QWidget : background, background-clip and background-origin
 */
void ToolBarWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &painter, this);

    QColor mainColor;
    mainColor = opt.palette.color(QPalette::Base);
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect(), 0, 0);
    painter.fillPath(rectPath,QBrush(mainColor));
    painter.end();
}

void ToolBarWidget::keyPressEvent(QKeyEvent *event)
{
    Q_UNUSED(event);

    return;
}

void ToolBarWidget::keyReleaseEvent(QKeyEvent *event)
{
    Q_UNUSED(event);

    return;
}

void ToolBarWidget::percentageChangedSlot()
{
    qDebug() << "percentage = " << g_sane_object->percentage;

    m_percentageLabel->setText(g_sane_object->percentage);
}
void ToolBarWidget::beautyButtonClickedSlot()
{
    qDebug() << "click beaty button.";

    if(g_sane_object->ocrFlag || g_sane_object->cropFlag){
        qDebug() << "do nothing,exit beauty!";
        return;
    }
    g_user_signal->exitWindowWithSaveFlag = true;
    g_user_signal->toolbarBeautyOperationStart();
    qDebug() << "show beauty dialog!";
    qDebug() << "continue!";

    QMap<QString, QString> buried_data;
    buried_data.insert("functionName", QString::fromStdString(kabase::BuriedPoint::convertPTtoString(kabase::BuriedPoint::PT::KylinScannerOneClickBeautification)));
    buried_data.insert("action", "beauty image");
    buried_data.insert("function", "in toolbarwidget.cpp function beautyButtonClickedSlots()");

    int cursor{0};
    KCustomProperty property[buried_data.size()];
    for (auto iter = buried_data.begin(); iter != buried_data.end(); iter++) {
        property[cursor].key = strdup(iter.key().toLocal8Bit().data());
        property[cursor].value = strdup(iter.value().toLocal8Bit().data());
        cursor++;
    }
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CUSTOM);
    kdk_dia_append_custom_property(node, property, buried_data.size());
    kdk_dia_upload_default(node, "click_beautification", const_cast<char *>("mainPage"));
    kdk_dia_data_free(node);
}

void ToolBarWidget::rectifyButtonClickedSlot()
{
    m_rectifyButton->setEnabled(false);
    qDebug() << "click rectify button.";
    if(g_sane_object->ocrFlag || g_sane_object->cropFlag){
        qDebug() << "do nothing,exit rectify!";
        return;
    }
    g_user_signal->exitWindowWithSaveFlag = true;
    g_user_signal->toolbarRectifyOperationStart();
    QTimer::singleShot(800, [=](){m_rectifyButton->setEnabled(true);});

    QMap<QString, QString> buried_data;
    buried_data.insert("functionName", QString::fromStdString(kabase::BuriedPoint::convertPTtoString(kabase::BuriedPoint::PT::KylinScannerRectification)));
    buried_data.insert("action", "image rectify");
    buried_data.insert("function", "in toolbarwidget.cpp function rectifyButtonClickedSlot()");

    int cursor{0};
    KCustomProperty property[buried_data.size()];
    for (auto iter = buried_data.begin(); iter != buried_data.end(); iter++) {
        property[cursor].key = strdup(iter.key().toLocal8Bit().data());
        property[cursor].value = strdup(iter.value().toLocal8Bit().data());
        cursor++;
    }
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CUSTOM);
    kdk_dia_append_custom_property(node, property, buried_data.size());
    kdk_dia_upload_default(node, "rectification", const_cast<char *>("mainPage"));
    kdk_dia_data_free(node);

}

void ToolBarWidget::ocrButtonClickedSlot()
{
    qDebug() << "click ocr button.";

    if(g_sane_object->cropFlag){
        qDebug() << "do nothing,exit ocr!";
        return;
    }
    emit g_user_signal->switchPageSig();
    if(g_sane_object->ocrFlag){
        g_user_signal->setAddDevStatusSignal(true);
        qDebug() << "exit OCR page...";
        emit g_user_signal->exitOCR();
        g_sane_object->ocrFlag = 0;
        ocrOtherIconState(true);

    }else{
        QMap<QString, QString> buried_data;
        buried_data.insert("functionName", QString::fromStdString(kabase::BuriedPoint::convertPTtoString(kabase::BuriedPoint::PT::KylinScannerTextRecognition)));
        buried_data.insert("action", "OCR");
        buried_data.insert("function", "in toolbarwidget.cpp function ocrButtonClickedSlot()");

        int cursor{0};
        KCustomProperty property[buried_data.size()];
        for (auto iter = buried_data.begin(); iter != buried_data.end(); iter++) {
            property[cursor].key = strdup(iter.key().toLocal8Bit().data());
            property[cursor].value = strdup(iter.value().toLocal8Bit().data());
            cursor++;
        }
        KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CUSTOM);
        kdk_dia_append_custom_property(node, property, buried_data.size());
        kdk_dia_upload_default(node, "text_recognition", const_cast<char *>("mainPage"));
        kdk_dia_data_free(node);

        ocrOtherIconState(false);
        g_user_signal->toolbarOcrOperationStart();
        g_sane_object->ocrFlag = 1;
    }
}

void ToolBarWidget::cropButtonClickedSlot()
{
    qDebug() << "click crop button.";

    if(g_sane_object->ocrFlag){
        qDebug() << "do nothing,exit crop!";
        return;
    }

    g_user_signal->exitWindowWithSaveFlag = true;
    g_user_signal->toolbarCropOperation();

    QMap<QString, QString> buried_data;
    buried_data.insert("functionName", QString::fromStdString(kabase::BuriedPoint::convertPTtoString(kabase::BuriedPoint::PT::KylinScannerCutting)));
    buried_data.insert("action", "image cutting");
    buried_data.insert("function", "in toolbarwidget.cpp function cropButtonClickedSlot()");

    int cursor{0};
    KCustomProperty property[buried_data.size()];
    for (auto iter = buried_data.begin(); iter != buried_data.end(); iter++) {
        property[cursor].key = strdup(iter.key().toLocal8Bit().data());
        property[cursor].value = strdup(iter.value().toLocal8Bit().data());
        cursor++;
    }
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CUSTOM);
    kdk_dia_append_custom_property(node, property, buried_data.size());
    kdk_dia_upload_default(node, "cutting", const_cast<char *>("mainPage"));
    kdk_dia_data_free(node);

}

void ToolBarWidget::rotateButtonClickedSlot()
{
    qDebug() << "click rotate button.";

    if(g_sane_object->ocrFlag || g_sane_object->cropFlag){
        qDebug() << "do nothing,exit rotate!";
        return;
    }

    g_user_signal->exitWindowWithSaveFlag = true;
    g_user_signal->toolbarRotateOperation();

    QMap<QString, QString> buried_data;
    buried_data.insert("functionName", QString::fromStdString(kabase::BuriedPoint::convertPTtoString(kabase::BuriedPoint::PT::KylinScannerRotate)));
    buried_data.insert("action", "image rotate");
    buried_data.insert("function", "in toolbarwidget.cpp function rotateButtonClickedSlot()");

    int cursor{0};
    KCustomProperty property[buried_data.size()];
    for (auto iter = buried_data.begin(); iter != buried_data.end(); iter++) {
        property[cursor].key = strdup(iter.key().toLocal8Bit().data());
        property[cursor].value = strdup(iter.value().toLocal8Bit().data());
        cursor++;
    }
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CUSTOM);
    kdk_dia_append_custom_property(node, property, buried_data.size());
    kdk_dia_upload_default(node, "rotate", const_cast<char *>("mainPage"));
    kdk_dia_data_free(node);
}

void ToolBarWidget::mirrorButtonClickedSlot()
{
    qDebug() << "click mirror button.";

    if(g_sane_object->ocrFlag || g_sane_object->cropFlag){
        qDebug() << "do nothing,exit mirror!";
        return;
    }

    g_user_signal->exitWindowWithSaveFlag = true;
    g_user_signal->toolbarMirrorOperation();

    QMap<QString, QString> buried_data;
    buried_data.insert("functionName", QString::fromStdString(kabase::BuriedPoint::convertPTtoString(kabase::BuriedPoint::PT::KylinScannerMirror)));
    buried_data.insert("action", "image mirror");
    buried_data.insert("function", "in toolbarwidget.cpp function mirrorButtonClickedSlot()");

    int cursor{0};
    KCustomProperty property[buried_data.size()];
    for (auto iter = buried_data.begin(); iter != buried_data.end(); iter++) {
        property[cursor].key = strdup(iter.key().toLocal8Bit().data());
        property[cursor].value = strdup(iter.value().toLocal8Bit().data());
        cursor++;
    }
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CUSTOM);
    kdk_dia_append_custom_property(node, property, buried_data.size());
    kdk_dia_upload_default(node, "mirror", const_cast<char *>("mainPage"));
    kdk_dia_data_free(node);
}

void ToolBarWidget::watermarkButtonClickedSlot()
{
    qDebug() << "click watermark button.";

    if(g_sane_object->ocrFlag || g_sane_object->cropFlag){
        qDebug() << "do nothing,exit watermark!";
        return;
    }

    g_user_signal->exitWindowWithSaveFlag = true;
    g_user_signal->toolbarWatermarkOperation();

    QMap<QString, QString> buried_data;
    buried_data.insert("functionName", QString::fromStdString(kabase::BuriedPoint::convertPTtoString(kabase::BuriedPoint::PT::KylinScannerAddWatermark)));
    buried_data.insert("action", "image add watermark");
    buried_data.insert("function", "in toolbarwidget.cpp function watermarkButtonClickedSlot()");


    int cursor{0};
    KCustomProperty property[buried_data.size()];
    for (auto iter = buried_data.begin(); iter != buried_data.end(); iter++) {
        property[cursor].key = strdup(iter.key().toLocal8Bit().data());
        property[cursor].value = strdup(iter.value().toLocal8Bit().data());
        cursor++;
    }
    KTrackData *node = kdk_dia_data_init(KEVENTSOURCE_DESKTOP, KEVENT_CUSTOM);
    kdk_dia_append_custom_property(node, property, buried_data.size());
    kdk_dia_upload_default(node, "add_watermark", const_cast<char *>("mainPage"));
    kdk_dia_data_free(node);
}

void ToolBarWidget::zoomoutButtonClickedSlot()
{
    qDebug() << "click zoomout button.";

    if(g_sane_object->ocrFlag || g_sane_object->cropFlag){
        qDebug() << "do nothing,exit zooout!";
        return;
    }

    g_user_signal->toolbarZoomoutOperation();
}

void ToolBarWidget::zoominButtonClickedSlot()
{
    qDebug() << "click zoomin button.";

    if(g_sane_object->ocrFlag || g_sane_object->cropFlag){
        qDebug() << "do nothing,exit zoomin!";
        return;
    }

    g_user_signal->toolbarZoominOperation();
}

void ToolBarWidget::initTimer()
{
    m_timer = new QTimer;
    connect(m_timer, SIGNAL(timeout()), this, SLOT(setRectifyButtonEnable()));
    m_timer->start(1500);
}

void ToolBarWidget::setRectifyButtonEnable()
{
    m_timer->stop();
    m_rectifyButton->setEnabled(true);
}
