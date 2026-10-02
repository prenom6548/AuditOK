/*
 *
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Sisheng He <hesisheng@kylinos.cn>
 *
 */

#include "widget.h"

#include "ktabbar.h"
#include <QBoxLayout>
#include <QSlider>
#include <QTabWidget>

Widget::Widget(QWidget *parent)
    : KWidget(parent)
{
    initUI(this);
}

void Widget::initUI(QWidget *parent)
{
    setMinimumWidth(900);
    KPushButtonFrame *pushbutton = new KPushButtonFrame(tr("pushbutton frame"));
    pushbutton->pushButton()->setText(tr("click"));

    KSwitchButtonFrame *switchbutton = new KSwitchButtonFrame(tr("switchbutton frame"));
    connect(switchbutton->switchButton(), &KSwitchButton::clicked, this, [=](bool checked) {
        pushbutton->setVisible(checked);
    });

    KComboBoxFrame *comboboxbutton = new KComboBoxFrame(tr("comboboxbutton frame"));
    comboboxbutton->comboBox()->addItem(tr("you can add item by addItem"));
    comboboxbutton->comboBox()->addItem(tr("you can add item by addItem1"));
    connect(comboboxbutton->comboBox(), &QComboBox::currentTextChanged, this, [=](QString text) {
        qDebug() << "selected combox item = " << text;
    });

    KLineEditFrame *lineeditbutton = new KLineEditFrame(tr("lineeditbutton frame"));

    KSliderFrame *sliderbutton = new KSliderFrame(tr("sliderbutton frame"), false);
    sliderbutton->leftLabel()->setText(tr("small"));
    sliderbutton->rightLabel()->setText(tr("large"));
    sliderbutton->slider()->setRange(1, 10);
    sliderbutton->slider()->setValue(4);

    connect(sliderbutton->slider(), &KSlider::valueChanged, this, [=](int value) {
        qDebug() << "slider value = " << value;
    });

    KRadioButtonFrame *radiobutton = new KRadioButtonFrame(tr("radiobutton frame"));
    QRadioButton *button1 = new QRadioButton(tr("radio1"));
    QRadioButton *button2 = new QRadioButton(tr("radio2"));
    QRadioButton *button3 = new QRadioButton(tr("radio3"));
    radiobutton->addButton(button1);
    radiobutton->addButton(button2);
    radiobutton->addButton(button3);
    connect(radiobutton->buttonGroup(), QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), [=](QAbstractButton *button) {
        qDebug() << "radio button = " << button->text();
    });

    KAddButton *addUserBtn = new KAddButton();
    addUserBtn->setObjectName("addUser");
    addUserBtn->setRadiusType(KBaseFrame::BorderRadiusStyle::Around);

    QHBoxLayout *addUserHorLayout = new QHBoxLayout();
    addUserHorLayout->setContentsMargins(0, 0, 0, 0);
    addUserHorLayout->addWidget(addUserBtn);

    KBaseFrame *addUserFrame = new KBaseFrame(this, KBaseFrame::BorderRadiusStyle::Around, true);
    addUserFrame->setContentsMargins(0, 0, 0, 0);
    addUserFrame->setObjectName("continue");
    addUserFrame->setLayout(addUserHorLayout);

    KSettingGroup *settinggroup = new KSettingGroup(parent);

    settinggroup->addWidget(pushbutton);
    settinggroup->addWidget(switchbutton);
    settinggroup->addWidget(comboboxbutton);
    settinggroup->addWidget(lineeditbutton);
    settinggroup->addWidget(sliderbutton);
    settinggroup->addWidget(radiobutton);
    settinggroup->addWidget(addUserFrame);

    QVBoxLayout *mainLayout = new QVBoxLayout(parent);
    mainLayout->addWidget(settinggroup);

    this->baseBar()->setLayout(mainLayout);
    this->baseBar()->setBackgroundRole(QPalette::Window);

    this->setLayoutType(LayoutType::HorizontalType);
}

Widget::~Widget()
{
}
