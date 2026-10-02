/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2025 KylinSoft  Co., Ltd.
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
#ifndef ACCESSIBILITY_UI_H
#define ACCESSIBILITY_UI_H

#include <libukcc/widgets/Label/titlelabel.h>
#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "libukcc/widgets/Label/fixlabel.h"
#include <kysdk/applications/accessinfohelper.h>
#include <klabel.h>
#include <themeController.h>
#include <kysdk/applications/klineframe.h>
#include <kswitchbutton.h>
#include "switchwidget.h"
#include "sliderwidget.h"

using namespace kdk;
QT_BEGIN_NAMESPACE

class Ui_Accessibility
{
public:
    TitleLabel  *m_pTitleLabel;
    TitleLabel  *m_pSetLabel;

    QComboBox   *m_pZoomComboBox;
    QComboBox   *m_pFilterComboBox;

    QPushButton *m_pVocalSetBtn;
    QPushButton *m_pPointerSetBtn;

    QVBoxLayout *m_pVerticalLayout;
    QVBoxLayout *m_pVerticalLayout_3;
    QVBoxLayout *m_pVerticalLayout_4;
    QVBoxLayout *m_pVerticalLayout_5;
    QVBoxLayout *m_pVerticalLayout_6;
    QVBoxLayout *m_pVerticalLayout_7;
    QVBoxLayout *m_pVerticalLayout_8;
    QVBoxLayout *m_pEffectVerticalLayout;

    QHBoxLayout *m_pHorizontalLayout_1;
    QHBoxLayout *m_pHorizontalLayout_2;
    QHBoxLayout *m_pHorizontalLayout_3;
    QHBoxLayout *m_pFilterHorizontalLayout;
    QHBoxLayout *m_pOtherHorizontalLayout_1;
    QHBoxLayout *m_pOtherHorizontalLayout_2;
    QHBoxLayout *m_pUseZoomHorizontalLayout;
    QHBoxLayout *m_pShortcutHorizontalLayout;

    FixLabel    *m_pFilterLabel;
    FixLabel    *m_pEffectLabel;
    FixLabel    *m_pVocalSetLbel;
    FixLabel    *m_pUseZoomLabel;
    FixLabel    *m_pShortcutLabel;
    FixLabel    *m_pPointerSetLbel;
    FixLabel    *m_pEffectPixmapLabel;
    SwitchWidget* m_zoomSwitchWidget = nullptr;
    SwitchWidget* m_screenReaderSwitchWidget = nullptr;
    SliderWidget* m_screenReaderVolumeSliderWidget = nullptr;
    SliderWidget* m_screenReaderSpeedSliderWidget = nullptr;

    QFrame      *m_pLine;
    QFrame      *m_pLine_1;
    QFrame      *m_pLine_2;
    QFrame      *m_pLine_3;
    QFrame      *m_pZoomFrame;
    QFrame      *m_pColorFrame;
    QFrame      *m_pEffecFrame;
    QFrame      *m_pFilterFrame;
    QFrame      *m_pVisionFrame;
    QFrame      *m_pUseZoomFrame;
    QFrame      *m_pShortcutFrame;
    QFrame      *m_pScreenReaderFrame;
    QFrame      *m_pOtherSetFrame;
    QFrame      *m_pScreenReaderLine_1;
    QFrame      *m_pScreenReaderLine_2;

    QSpacerItem *m_pVerticalSpacer;

    void setupUi(QWidget *Accessibility)
    {
        if (Accessibility->objectName().isEmpty())
            Accessibility->setObjectName(QString::fromUtf8("Accessibility"));
        Accessibility->resize(710, 1149);
        Accessibility->setWindowTitle(QString::fromUtf8("Form"));
        m_pVerticalLayout_3 = new QVBoxLayout(Accessibility);
        m_pVerticalLayout_3->setObjectName(QString::fromUtf8("m_pVerticalLayout_3"));
        m_pVerticalLayout_3->setContentsMargins(0, 0, 0, 0);
        m_pTitleLabel = new TitleLabel(Accessibility);
        KDK_EXTEND_ALL_INFO_FORMAT(m_pTitleLabel, "Accessibility", "", "");
        m_pTitleLabel->setObjectName(QString::fromUtf8("m_pTitleLabel"));

        m_pVerticalLayout_3->addWidget(m_pTitleLabel);

        m_pVisionFrame = new QFrame(Accessibility);
        m_pVisionFrame->setObjectName(QString::fromUtf8("m_pVisionFrame"));
        m_pVisionFrame->setMinimumSize(QSize(550, 0));
        m_pVisionFrame->setMaximumSize(QSize(16777215, 16777215));
        m_pVisionFrame->setFrameShape(QFrame::Box);
        m_pVerticalLayout_5 = new QVBoxLayout(m_pVisionFrame);
        m_pVerticalLayout_5->setSpacing(0);
        m_pVerticalLayout_5->setObjectName(QString::fromUtf8("m_pVerticalLayout_5"));
        m_pVerticalLayout_5->setContentsMargins(0, 0, 0, 0);

        m_pZoomFrame = new QFrame(m_pVisionFrame);
        m_pZoomFrame->setMinimumSize(QSize(550, 0));
        m_pZoomFrame->setMaximumSize(QSize(16777215, 16777215));
        m_pZoomFrame->setFrameShape(QFrame::Box);

        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(m_pZoomFrame->sizePolicy().hasHeightForWidth());

        //~ contents_path /Accessibility/Enable magnifying glass: Enlarge the content of the desktop
        m_zoomSwitchWidget = new SwitchWidget(QApplication::translate("Accessibility", "Enable magnifying glass: Enlarge the content of the desktop", nullptr), Accessibility, UkccFrame::BorderRadiusStyle::Around, QApplication::translate("Accessibility", "Press Super + \"+\" to zoom in on the content, Super + \"-\" to zoom out on the content.", nullptr));
        m_zoomSwitchWidget->setMinimumSize(QSize(550, 0));
        m_zoomSwitchWidget->setMaximumSize(QSize(16777215, 16777215));
        m_zoomSwitchWidget->setHeightAdaptive(false);
        KDK_EXTEND_ALL_INFO_FORMAT(m_zoomSwitchWidget->switchButton(), "Accessibility", "", "");

        m_pVerticalLayout_5->addWidget(m_zoomSwitchWidget);

        KHLineFrame *lineFrame = new KHLineFrame(m_pZoomFrame);
        m_pVerticalLayout_5->addWidget(lineFrame);

        m_pUseZoomFrame = new QFrame(m_pVisionFrame);
        m_pUseZoomFrame->setSizePolicy(sizePolicy);
        m_pUseZoomFrame->setMinimumSize(QSize(550, 60));
        m_pUseZoomFrame->setMaximumSize(QSize(16777215, 60));
        m_pUseZoomFrame->setFrameShape(QFrame::Box);
        m_pHorizontalLayout_2 = new QHBoxLayout(m_pUseZoomFrame);
        m_pHorizontalLayout_2->setSpacing(16);
        m_pHorizontalLayout_2->setObjectName(QString::fromUtf8("m_pHorizontalLayout_2"));
        m_pHorizontalLayout_2->setContentsMargins(16, 0, 16, 0);
        m_pUseZoomHorizontalLayout = new QHBoxLayout();
        m_pUseZoomHorizontalLayout->setObjectName(QString::fromUtf8("m_pUseZoomHorizontalLayout"));
        m_pUseZoomLabel = new FixLabel(m_pUseZoomFrame);
        KDK_EXTEND_ALL_INFO_FORMAT(m_pUseZoomLabel, "Accessibility", "", "");
        m_pUseZoomLabel->setObjectName(QString::fromUtf8("m_pUseZoomLabel"));
        m_pUseZoomLabel->setMinimumSize(QSize(230, 0));
        m_pUseZoomLabel->setMaximumSize(QSize(230, 16777215));

        m_pUseZoomHorizontalLayout->addWidget(m_pUseZoomLabel);

        m_pZoomComboBox = new QComboBox(m_pUseZoomFrame);
        KDK_EXTEND_ALL_INFO_FORMAT(m_pZoomComboBox, "Accessibility", "", "");
        m_pZoomComboBox->setObjectName(QString::fromUtf8("m_pZoomComboBox"));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(m_pZoomComboBox->sizePolicy().hasHeightForWidth());
        m_pZoomComboBox->setSizePolicy(sizePolicy1);
        m_pZoomComboBox->setMinimumSize(QSize(0, 0));
        m_pZoomComboBox->setStyleSheet(QString::fromUtf8(""));

        m_pUseZoomHorizontalLayout->addWidget(m_pZoomComboBox);


        m_pHorizontalLayout_2->addLayout(m_pUseZoomHorizontalLayout);


        m_pVerticalLayout_5->addWidget(m_pUseZoomFrame);


        m_pVerticalLayout_3->addWidget(m_pVisionFrame);

        m_pColorFrame = new QFrame(Accessibility);
        m_pColorFrame->setObjectName(QString::fromUtf8("m_pColorFrame"));
        m_pColorFrame->setFrameShape(QFrame::Box);
        m_pColorFrame->setFrameShadow(QFrame::Raised);
        m_pColorFrame->setLineWidth(0);
        m_pVerticalLayout_7 = new QVBoxLayout(m_pColorFrame);
        m_pVerticalLayout_7->setSpacing(0);
        m_pVerticalLayout_7->setObjectName(QString::fromUtf8("m_pVerticalLayout_7"));
        m_pVerticalLayout_7->setContentsMargins(0, 0, 0, 0);
        m_pEffecFrame = new QFrame(m_pColorFrame);
        m_pEffecFrame->setObjectName(QString::fromUtf8("m_pEffecFrame"));
        QSizePolicy sizePolicy2(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(m_pEffecFrame->sizePolicy().hasHeightForWidth());
        m_pEffecFrame->setSizePolicy(sizePolicy2);
        m_pEffecFrame->setFrameShape(QFrame::Box);
        m_pVerticalLayout_4 = new QVBoxLayout(m_pEffecFrame);
        m_pVerticalLayout_4->setSpacing(6);
        m_pVerticalLayout_4->setObjectName(QString::fromUtf8("m_pVerticalLayout_4"));
        m_pVerticalLayout_4->setContentsMargins(16, 16, 16, 16);
        m_pEffectVerticalLayout = new QVBoxLayout();
        m_pEffectVerticalLayout->setObjectName(QString::fromUtf8("m_pEffectVerticalLayout"));
        m_pEffectLabel = new FixLabel(m_pEffecFrame);
        KDK_EXTEND_ALL_INFO_FORMAT(m_pEffectLabel, "Accessibility", "", "");
        m_pEffectLabel->setObjectName(QString::fromUtf8("m_pEffectLabel"));

        m_pEffectVerticalLayout->addWidget(m_pEffectLabel);

        m_pEffectPixmapLabel = new FixLabel(m_pEffecFrame);
        KDK_EXTEND_ALL_INFO_FORMAT(m_pEffectPixmapLabel, "Accessibility", "", "");
        m_pEffectPixmapLabel->setObjectName(QString::fromUtf8("m_pEffectPixmapLabel"));
        sizePolicy2.setHeightForWidth(m_pEffectPixmapLabel->sizePolicy().hasHeightForWidth());
        m_pEffectPixmapLabel->setSizePolicy(sizePolicy2);
        m_pEffectPixmapLabel->setAlignment(Qt::AlignCenter);

        m_pEffectVerticalLayout->addWidget(m_pEffectPixmapLabel);


        m_pVerticalLayout_4->addLayout(m_pEffectVerticalLayout);


        m_pVerticalLayout_7->addWidget(m_pEffecFrame);

        KHLineFrame *lineEffectFrame = new KHLineFrame(m_pEffecFrame);
        m_pVerticalLayout_7->addWidget(lineEffectFrame);

        m_pFilterFrame = new QFrame(m_pColorFrame);
        m_pFilterFrame->setObjectName(QString::fromUtf8("m_pFilterFrame"));
        sizePolicy.setHeightForWidth(m_pFilterFrame->sizePolicy().hasHeightForWidth());
        m_pFilterFrame->setSizePolicy(sizePolicy);
        m_pFilterFrame->setMinimumSize(QSize(550, 60));
        m_pFilterFrame->setMaximumSize(QSize(16777215, 60));
        m_pFilterFrame->setFrameShape(QFrame::Box);
        m_pHorizontalLayout_1 = new QHBoxLayout(m_pFilterFrame);
        m_pHorizontalLayout_1->setSpacing(16);
        m_pHorizontalLayout_1->setObjectName(QString::fromUtf8("m_pHorizontalLayout_1"));
        m_pHorizontalLayout_1->setContentsMargins(16, 0, 16, 0);
        m_pFilterHorizontalLayout = new QHBoxLayout();
        m_pFilterHorizontalLayout->setObjectName(QString::fromUtf8("m_pFilterHorizontalLayout"));
        m_pFilterLabel = new FixLabel(m_pFilterFrame);
        KDK_EXTEND_ALL_INFO_FORMAT(m_pFilterLabel, "Accessibility", "", "");
        m_pFilterLabel->setObjectName(QString::fromUtf8("m_pFilterLabel"));
        m_pFilterLabel->setMinimumSize(QSize(230, 0));
        m_pFilterLabel->setMaximumSize(QSize(230, 16777215));

        m_pFilterHorizontalLayout->addWidget(m_pFilterLabel);

        m_pFilterComboBox = new QComboBox(m_pFilterFrame);
        KDK_EXTEND_ALL_INFO_FORMAT(m_pFilterComboBox, "Accessibility", "", "");
        m_pFilterComboBox->setObjectName(QString::fromUtf8("m_pFilterComboBox"));
        QSizePolicy sizePolicy3(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(100);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(m_pFilterComboBox->sizePolicy().hasHeightForWidth());
        m_pFilterComboBox->setSizePolicy(sizePolicy3);
        m_pFilterComboBox->setMinimumSize(QSize(0, 0));
        m_pFilterComboBox->setStyleSheet(QString::fromUtf8(""));

        m_pFilterHorizontalLayout->addWidget(m_pFilterComboBox);


        m_pHorizontalLayout_1->addLayout(m_pFilterHorizontalLayout);


        m_pVerticalLayout_7->addWidget(m_pFilterFrame);

        KHLineFrame *lineColorFrame = new KHLineFrame(m_pFilterFrame);
        m_pVerticalLayout_7->addWidget(lineColorFrame);

        m_pShortcutFrame = new QFrame(m_pColorFrame);
        m_pShortcutFrame->setObjectName(QString::fromUtf8("m_pShortcutFrame"));
        sizePolicy.setHeightForWidth(m_pShortcutFrame->sizePolicy().hasHeightForWidth());
        m_pShortcutFrame->setSizePolicy(sizePolicy);
        m_pShortcutFrame->setMinimumSize(QSize(550, 60));
        m_pShortcutFrame->setMaximumSize(QSize(16777215, 60));
        m_pShortcutFrame->setFrameShape(QFrame::Box);
        m_pHorizontalLayout_3 = new QHBoxLayout(m_pShortcutFrame);
        m_pHorizontalLayout_3->setSpacing(32);
        m_pHorizontalLayout_3->setObjectName(QString::fromUtf8("m_pHorizontalLayout_3"));
        m_pHorizontalLayout_3->setContentsMargins(16, 0, 16, 0);
        m_pShortcutHorizontalLayout = new QHBoxLayout();
        m_pShortcutHorizontalLayout->setObjectName(QString::fromUtf8("m_pShortcutHorizontalLayout"));
        m_pShortcutLabel = new FixLabel(m_pShortcutFrame);
        KDK_EXTEND_ALL_INFO_FORMAT(m_pShortcutLabel, "Accessibility", "", "");
        m_pShortcutLabel->setObjectName(QString::fromUtf8("m_pShortcutLabel"));

        m_pShortcutHorizontalLayout->addWidget(m_pShortcutLabel);


        m_pHorizontalLayout_3->addLayout(m_pShortcutHorizontalLayout);


        m_pVerticalLayout_7->addWidget(m_pShortcutFrame);


        m_pVerticalLayout_3->addWidget(m_pColorFrame);

        m_pScreenReaderFrame = new QFrame(Accessibility);
        m_pScreenReaderFrame->setObjectName(QString::fromUtf8("m_pScreenReaderFrame"));
        m_pScreenReaderFrame->setMinimumSize(QSize(550, 0));
        m_pScreenReaderFrame->setMaximumSize(QSize(16777215, 16777215));
        m_pScreenReaderFrame->setFrameShape(QFrame::Box);
        m_pScreenReaderFrame->setFrameShadow(QFrame::Raised);
        m_pScreenReaderFrame->setLineWidth(0);
        m_pVerticalLayout_8 = new QVBoxLayout(m_pScreenReaderFrame);
        m_pVerticalLayout_8->setSpacing(0);
        m_pVerticalLayout_8->setObjectName(QString::fromUtf8("m_pVerticalLayout_8"));
        m_pVerticalLayout_8->setContentsMargins(0, 0, 0, 0);

        //~ contents_path /Accessibility/Enable screen reader
        m_screenReaderSwitchWidget = new SwitchWidget(QApplication::translate("Accessibility", "Enable Screen Reader", nullptr),
                                                      m_pScreenReaderFrame,
                                                      UkccFrame::BorderRadiusStyle::Top,
                                                      QApplication::translate("Accessibility", "Press \"Super+Alt+S\" to quickly enable or disable screen reader.", nullptr));
        m_screenReaderSwitchWidget->setObjectName(QString::fromUtf8("m_screenReaderSwitchWidget"));
        m_screenReaderSwitchWidget->setMinimumSize(QSize(550, 0));
        m_screenReaderSwitchWidget->setMaximumSize(QSize(16777215, 16777215));
        m_screenReaderSwitchWidget->setHeightAdaptive(false);
        KDK_EXTEND_ALL_INFO_FORMAT(m_screenReaderSwitchWidget->switchButton(), "Accessibility", "", "");
        m_pVerticalLayout_8->addWidget(m_screenReaderSwitchWidget);

        m_pScreenReaderLine_1 = new KHLineFrame(m_pScreenReaderFrame);
        m_pVerticalLayout_8->addWidget(m_pScreenReaderLine_1);

        //~ contents_path /Accessibility/Screen reader volume
        m_screenReaderVolumeSliderWidget = new SliderWidget(QApplication::translate("Accessibility", "Screen Reader Volume", nullptr),
                                                            false,
                                                            m_pScreenReaderFrame,
                                                            UkccFrame::BorderRadiusStyle::None);
        m_screenReaderVolumeSliderWidget->setObjectName(QString::fromUtf8("m_screenReaderVolumeSliderWidget"));
        m_screenReaderVolumeSliderWidget->setTitleFixedWidth(230);
        m_screenReaderVolumeSliderWidget->setMinimumSize(QSize(550, 60));
        m_screenReaderVolumeSliderWidget->setMaximumSize(QSize(16777215, 60));
        m_screenReaderVolumeSliderWidget->slider()->setAccessibleName(QApplication::translate("Accessibility", "Reader Volume", nullptr));
        m_pVerticalLayout_8->addWidget(m_screenReaderVolumeSliderWidget);

        m_pScreenReaderLine_2 = new KHLineFrame(m_pScreenReaderFrame);
        m_pVerticalLayout_8->addWidget(m_pScreenReaderLine_2);

        //~ contents_path /Accessibility/Screen reader speed
        m_screenReaderSpeedSliderWidget = new SliderWidget(QApplication::translate("Accessibility", "Screen Reader Speed", nullptr),
                                                           false,
                                                           m_pScreenReaderFrame,
                                                           UkccFrame::BorderRadiusStyle::Bottom);
        m_screenReaderSpeedSliderWidget->setObjectName(QString::fromUtf8("m_screenReaderSpeedSliderWidget"));
        m_screenReaderSpeedSliderWidget->setTitleFixedWidth(230);
        m_screenReaderSpeedSliderWidget->setMinimumSize(QSize(550, 60));
        m_screenReaderSpeedSliderWidget->setMaximumSize(QSize(16777215, 60));
        m_screenReaderSpeedSliderWidget->slider()->setAccessibleName(QApplication::translate("Accessibility", "Reader Speed", nullptr));

        m_pVerticalLayout_8->addWidget(m_screenReaderSpeedSliderWidget);

        m_pVerticalLayout_3->addWidget(m_pScreenReaderFrame);

        m_pVerticalLayout = new QVBoxLayout();
        m_pVerticalLayout->setObjectName(QString::fromUtf8("m_pVerticalLayout"));
        m_pVerticalLayout->setContentsMargins(-1, 24, -1, -1);
        m_pSetLabel = new TitleLabel(Accessibility);
        KDK_EXTEND_ALL_INFO_FORMAT(m_pSetLabel, "Accessibility", "", "");
        m_pSetLabel->setObjectName(QString::fromUtf8("m_pSetLabel"));

        m_pVerticalLayout->addWidget(m_pSetLabel);


        m_pVerticalLayout_3->addLayout(m_pVerticalLayout);

        m_pOtherSetFrame = new QFrame(Accessibility);
        m_pOtherSetFrame->setObjectName(QString::fromUtf8("m_pOtherSetFrame"));
        m_pOtherSetFrame->setMinimumSize(QSize(0, 120));
        m_pOtherSetFrame->setMaximumSize(QSize(16777215, 120));
        m_pOtherSetFrame->setFrameShape(QFrame::Box);
        m_pOtherSetFrame->setFrameShadow(QFrame::Raised);
        m_pOtherSetFrame->setLineWidth(0);
        m_pVerticalLayout_6 = new QVBoxLayout(m_pOtherSetFrame);
        m_pVerticalLayout_6->setSpacing(0);
        m_pVerticalLayout_6->setObjectName(QString::fromUtf8("m_pVerticalLayout_6"));
        m_pVerticalLayout_6->setContentsMargins(0, 0, 0, 0);
        m_pOtherHorizontalLayout_1 = new QHBoxLayout();
        m_pOtherHorizontalLayout_1->setObjectName(QString::fromUtf8("m_pOtherHorizontalLayout_1"));
        m_pOtherHorizontalLayout_1->setContentsMargins(16, -1, 16, -1);
        m_pPointerSetLbel = new FixLabel(m_pOtherSetFrame);
        KDK_EXTEND_ALL_INFO_FORMAT(m_pPointerSetLbel, "Accessibility", "", "");
        m_pPointerSetLbel->setObjectName(QString::fromUtf8("m_pPointerSetLbel"));

        m_pOtherHorizontalLayout_1->addWidget(m_pPointerSetLbel);

        m_pPointerSetBtn = new QPushButton(m_pOtherSetFrame);
        KDK_EXTEND_ALL_INFO_FORMAT(m_pPointerSetBtn, "Accessibility", "", "");
        m_pPointerSetBtn->setObjectName(QString::fromUtf8("m_pPointerSetBtn"));
        m_pPointerSetBtn->setMinimumSize(QSize(96, 0));
        m_pPointerSetBtn->setMaximumSize(QSize(96, 16777215));

        m_pOtherHorizontalLayout_1->addWidget(m_pPointerSetBtn);


        m_pVerticalLayout_6->addLayout(m_pOtherHorizontalLayout_1);

        KHLineFrame *lineSetFrame = new KHLineFrame(m_pOtherSetFrame);
        m_pVerticalLayout_6->addWidget(lineSetFrame);

        m_pOtherHorizontalLayout_2 = new QHBoxLayout();
        m_pOtherHorizontalLayout_2->setObjectName(QString::fromUtf8("m_pOtherHorizontalLayout_2"));
        m_pOtherHorizontalLayout_2->setContentsMargins(16, -1, 16, -1);
        m_pVocalSetLbel = new FixLabel(m_pOtherSetFrame);
        KDK_EXTEND_ALL_INFO_FORMAT(m_pVocalSetLbel, "Accessibility", "", "");
        m_pVocalSetLbel->setObjectName(QString::fromUtf8("m_pVocalSetLbel"));

        m_pOtherHorizontalLayout_2->addWidget(m_pVocalSetLbel);

        m_pVocalSetBtn = new QPushButton(m_pOtherSetFrame);
        KDK_EXTEND_ALL_INFO_FORMAT(m_pVocalSetBtn, "Accessibility", "", "");
        m_pVocalSetBtn->setObjectName(QString::fromUtf8("m_pVocalSetBtn"));
        m_pVocalSetBtn->setMinimumSize(QSize(96, 0));
        m_pVocalSetBtn->setMaximumSize(QSize(96, 16777215));

        m_pOtherHorizontalLayout_2->addWidget(m_pVocalSetBtn);


        m_pVerticalLayout_6->addLayout(m_pOtherHorizontalLayout_2);


        m_pVerticalLayout_3->addWidget(m_pOtherSetFrame);

        m_pVerticalSpacer = new QSpacerItem(20, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);

        m_pVerticalLayout_3->addItem(m_pVerticalSpacer);


        retranslateUi(Accessibility);

        QMetaObject::connectSlotsByName(Accessibility);
    } // setupUi

    void retranslateUi(QWidget *Accessibility)
    {

        //~ contents_path /Accessibility/Vision
        m_pTitleLabel->setText(QApplication::translate("Accessibility", "Vision", nullptr));
        //~ contents_path /Accessibility/Zoom
        m_pUseZoomLabel->setText(QApplication::translate("Accessibility", "Zoom", nullptr));
        //~ contents_path /Accessibility/Color Filter Effect
        m_pEffectLabel->setText(QApplication::translate("Accessibility", "Color Filter Effect", nullptr));
        //~ contents_path /Accessibility/Color Filter
        m_pFilterLabel->setText(QApplication::translate("Accessibility", "Color Filter", nullptr));
        //~ contents_path /Accessibility/Press Super + Ctrl + C to turn on or off color filter
        m_pShortcutLabel->setText(QApplication::translate("Accessibility", "Press Super + Ctrl + C to turn on or off color filter", nullptr));
        //~ contents_path /Accessibility/Other Settings
        m_pSetLabel->setText(QApplication::translate("Accessibility", "Other Settings", nullptr));
        //~ contents_path /Accessibility/Pointer Size
        m_pPointerSetLbel->setText(QApplication::translate("Accessibility", "Pointer Size", nullptr));
        m_pPointerSetBtn->setText(QApplication::translate("Accessibility", "Set", nullptr));
        //~ contents_path /Accessibility/Vocal Tract Regulation
        m_pVocalSetLbel->setText(QApplication::translate("Accessibility", "Vocal Tract Regulation", nullptr));
        m_pVocalSetBtn->setText(QApplication::translate("Accessibility", "Set", nullptr));
        Q_UNUSED(Accessibility);
    } // retranslateUi

};

namespace Ui {
    class Accessibility: public Ui_Accessibility {};
} // namespace Ui

QT_END_NAMESPACE

#endif // ACCESSIBILITY_UI_H
