/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
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
#include "ukccplugin.h"

#include <ukcc/widgets/switchbutton.h>
#include <ukcc/widgets/settinggroup.h>
#include <ukcc/widgets/comboxwidget.h>
#include <ukcc/widgets/lineeditwidget.h>
#include <ukcc/widgets/pushbuttonwidget.h>
#include <ukcc/widgets/radiobuttonwidget.h>
#include <ukcc/widgets/sliderwidget.h>
#include <ukcc/widgets/switchwidget.h>


UkccPlugin::UkccPlugin() : mFirstLoad(true)
{
    QTranslator *translator = new QTranslator(this);
    translator->load("/usr/share/plugin-test/translations/" + QLocale::system().name());
    QApplication::installTranslator(translator);

    pluginName = tr("UkccPlugin");
    pluginType = SYSTEM;
}

UkccPlugin::~UkccPlugin()
{

}

QString UkccPlugin::plugini18nName()
{
    return pluginName;
}

int UkccPlugin::pluginTypes()
{
    return pluginType;
}

QWidget *UkccPlugin::pluginUi()
{
    // 需要加上这个判断标志位，否则每次点击都会新建一个QWidget(只加载一次)
    if (mFirstLoad) {
        widget = new QWidget;
        mFirstLoad = false;

        settingWidgetTest();
    }
    return widget;
}

bool UkccPlugin::isEnable() const
{
    return true;
}

const QString UkccPlugin::name() const
{
    return QStringLiteral("UkccPlugin");
}

bool UkccPlugin::isShowOnHomePage() const
{
    return true;
}

QIcon UkccPlugin::icon() const
{
    return QIcon();
}

QString UkccPlugin::translationPath() const
{
    return "/usr/share/plugin-test/translations/%1.ts";
}

void UkccPlugin::initSearchText()
{
    //~ contents_path /UkccPlugin/UkccPlugin
    tr("UkccPlugin");
    //~ contents_path /UkccPlugin/ukccplugin test
    tr("ukccplugin test");
}

void UkccPlugin::settingWidgetTest()
{
    QVBoxLayout *Vlayout = new QVBoxLayout(this->widget);
    Vlayout->setContentsMargins(0, 0, 0, 0);

    SettingGroup *settingGroup = new SettingGroup();

    // test ComboxWidget
    ComboxWidget *comboxWidgetTest = new ComboxWidget(tr("test for ComboxWidget"));
    comboxWidgetTest->setObjectName("test ComboxWidget");
    qDebug() << "title = " << comboxWidgetTest->title();
    comboxWidgetTest->setTitleFixedWidth(220);;
    comboxWidgetTest->setCurrentText(tr("test 0"));
    comboxWidgetTest->comboBox()->insertItem(1, tr("test 1"));
    comboxWidgetTest->setCurrentIndex(0);
    comboxWidgetTest->installEventFilter(this);
    connect(comboxWidgetTest, &ComboxWidget::currentIndexChanged, this, [=](int index){
        qDebug() << "IndexChanged, index = " << index;
    });

    connect(comboxWidgetTest, &ComboxWidget::currentTextChanged, this, [=](QString text){
        qDebug() << "TextChanged, text = " << text;
    });

    // test LineEditWidget
    LineEditWidget *lineEditWidgetTest = new LineEditWidget(tr("test for LineEditWidget"), settingGroup, UkccFrame::None);
    lineEditWidgetTest->setTitleFixedWidth(220);
    lineEditWidgetTest->setObjectName("test LineEditWidget");
    qDebug() << "title = " << lineEditWidgetTest->title();

    connect(lineEditWidgetTest->lineEdit(), &QLineEdit::textEdited, this, [=](QString text){
         qDebug() << "LineEditWidget textEdited, text = " << text;
    });

    // test PushButtonWidget
    PushButtonWidget *pushButtonWidgetTest = new PushButtonWidget(tr("test for PushButtonWidget"));
    pushButtonWidgetTest->setButtonText(tr("button text"));
    pushButtonWidgetTest->setButtonFixedWidth(200);
    pushButtonWidgetTest->setTitleFixedWidth(108);
    connect(pushButtonWidgetTest, &PushButtonWidget::clicked, this, [=](bool checked){
        qDebug() << "PushButtonWidget clicked " << checked;
    });

    // test RadioButtonWidget
    RadioButtonWidget *radioButtonWidgetTest = new RadioButtonWidget(tr("test for PushButtonWidget"));
    QRadioButton *leftRadio = new QRadioButton(tr("Left"));
    QRadioButton *rightRadio = new QRadioButton(tr("Right"));
    radioButtonWidgetTest->addButton(leftRadio, 0);
    radioButtonWidgetTest->addButton(rightRadio, 1, true);
    radioButtonWidgetTest->setButtonSpacing(24);

    connect(radioButtonWidgetTest, &RadioButtonWidget::buttonClicked, this, [=](int id){
        qDebug() << "RadioButtonWidget buttonClicked, id = " << id;
    });

    // test SliderWidget
    SliderWidget *sliderWidgetTest = new SliderWidget(tr("test for SliderWidget"), true, settingGroup);
    sliderWidgetTest->setLeftPixmap(QIcon::fromTheme("document-edit-symbolic").pixmap(sliderWidgetTest->leftLabel()->size()));
    sliderWidgetTest->setRightText(tr("Fast"));
    sliderWidgetTest->slider()->setSliderType(KSliderType::NodeSlider);
    sliderWidgetTest->slider()->setRange(1,10);
    sliderWidgetTest->slider()->setTickInterval(1);
    sliderWidgetTest->slider()->setPageStep(1);
    sliderWidgetTest->slider()->installEventFilter(this);
    connect(sliderWidgetTest, &SliderWidget::valueChanged, this, [=](int value){
        qDebug() << "SliderWidget valueChanged, value = " << value;
    });

    // test
    SwitchWidget *switchWidgetTest = new SwitchWidget(tr("test for SwitchWidget"));
    QPushButton *button = new QPushButton(tr("QPushButton"));
    switchWidgetTest->insertWidget(switchWidgetTest->layout()->count()-1, button);
    connect(switchWidgetTest, &SwitchWidget::stateChanged, this, [=](bool checked){
        qDebug() << "SwitchWidget clicked " << checked;
    });

    settingGroup->addWidget(comboxWidgetTest);
    settingGroup->addWidget(lineEditWidgetTest);
    settingGroup->addWidget(pushButtonWidgetTest);
    settingGroup->addWidget(radioButtonWidgetTest);
    settingGroup->insertWidget(0,switchWidgetTest);

    Vlayout->addWidget(settingGroup);
    Vlayout->addStretch();

}

