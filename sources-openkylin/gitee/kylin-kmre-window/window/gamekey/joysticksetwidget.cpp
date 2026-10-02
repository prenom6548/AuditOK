/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Zero Liu    liuzenghui1@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "joysticksetwidget.h"
#include "JoystickCommon.h"
#include "widgets/messagebox.h"
#include "joystickbasekey.h"

#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QGridLayout>
#include <QToolButton>
#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>

JoystickSetWidget::JoystickSetWidget(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout* mRightVBoxLayout = new QVBoxLayout();  //右侧的垂直布局


//    QLabel* mKeySetLabel = new QLabel(this);
//    mKeySetLabel->setText(tr("键位设置"));
//    QFont font_label,font_button;
//    font_label.setPointSize(10);
//    font_button.setPointSize(8);
//    mKeySetLabel->setFont(font_label);
//    mKeySetLabel->setStyleSheet("QLabel{background-color:rgb(220,220,220);}");
//    mKeySetLabel->setMargin(5);

    QFont font_button;
    font_button.setPointSize(DEFAULT_BUTTON_LABEL_TEXT_SIZE);

    QPushButton* mAddSingleKeyBtn = new QPushButton(this);
    mAddSingleKeyBtn->setFixedSize(DEFAULT_BUTTON_SIZE,DEFAULT_BUTTON_SIZE);
    mAddSingleKeyBtn->setIcon(QIcon(":/new/res/icon_click2x.png"));
    mAddSingleKeyBtn->setIconSize({DEFAULT_ICON_SIZE,DEFAULT_ICON_SIZE});
//    mAddSingleKeyBtn->setToolTip(tr("Click the button to create a custom game button"));
    mAddSingleKeyBtn->setFocusPolicy(Qt::NoFocus);
    mAddSingleKeyBtn->setBackgroundRole(QPalette::Button);
//    mAddSingleKeyBtn->setAutoFillBackground(true);
//    mAddSingleKeyBtn->setStyleSheet("QPushButton{background-color:lightblue;border-radius:12px;}");
    connect(mAddSingleKeyBtn, &QPushButton::clicked, this, [this] {
        emit addJoystickKey(JoystickBaseKey::SINGLEKEY);

    });

    QLabel* singleKeyLabel = new QLabel(this);
    singleKeyLabel->setText(tr("Click"));
    singleKeyLabel->setFont(font_button);

    QVBoxLayout* singleKeylayout = new QVBoxLayout();
    singleKeylayout->addWidget(mAddSingleKeyBtn);
    singleKeylayout->addSpacing(3);
    singleKeylayout->addWidget(singleKeyLabel);
    singleKeylayout->setAlignment(mAddSingleKeyBtn,Qt::AlignHCenter);
    singleKeylayout->setAlignment(singleKeyLabel,Qt::AlignHCenter);

    QPushButton* mAddComboBtn = new QPushButton(this);
    mAddComboBtn->setFixedSize(DEFAULT_BUTTON_SIZE,DEFAULT_BUTTON_SIZE);
    mAddComboBtn->setIcon(QIcon(":/new/res/icon_combo2x.png"));
    mAddComboBtn->setIconSize({DEFAULT_ICON_SIZE,DEFAULT_ICON_SIZE});
//    mAddComboBtn->setToolTip(tr("Click the button to set auto methed!"));
    mAddComboBtn->setFocusPolicy(Qt::NoFocus);
//    mAddComboBtn->setStyleSheet("QToolButton{background:transparent}");
    connect(mAddComboBtn,&QPushButton::clicked,this,[this](){
       emit addJoystickKey(JoystickBaseKey::COMBOKEY);
    });

    QLabel* comboKeyLabel = new QLabel(this);
    comboKeyLabel->setText(tr("Combo"));
    comboKeyLabel->setFont(font_button);

    QVBoxLayout* comboKeylayout = new QVBoxLayout();
    comboKeylayout->addWidget(mAddComboBtn);
    comboKeylayout->addSpacing(3);
    comboKeylayout->addWidget(comboKeyLabel);
    comboKeylayout->setAlignment(mAddComboBtn,Qt::AlignHCenter);
    comboKeylayout->setAlignment(comboKeyLabel,Qt::AlignHCenter);

    mAddJoystickLeftBtn = new QPushButton(this);
    mAddJoystickLeftBtn->setFixedSize(DEFAULT_BUTTON_SIZE,DEFAULT_BUTTON_SIZE);
    mAddJoystickLeftBtn->setIcon(QIcon(":/new/res/icon_left_joystick2x.png"));
    mAddJoystickLeftBtn->setIconSize({DEFAULT_ICON_SIZE,DEFAULT_ICON_SIZE});
//    mAddJoystickLeftBtn->setToolTip(tr("Click the button to create a custom game button"));
    mAddJoystickLeftBtn->setFocusPolicy(Qt::NoFocus);
//    mAddJoystickLeftBtn->setStyleSheet("QToolButton{background:transparent}");
    connect(mAddJoystickLeftBtn, &QPushButton::clicked, this, [this] {
        emit addJoystickKey(JoystickBaseKey::JOYLEFT);
    });

    QLabel* leftLabel = new QLabel(this);
    leftLabel->setText(tr("L-Joy."));
    leftLabel->setFont(font_button);

    QVBoxLayout* leftJoysticklayout = new QVBoxLayout();
    leftJoysticklayout->addWidget(mAddJoystickLeftBtn);
    leftJoysticklayout->addSpacing(3);
    leftJoysticklayout->addWidget(leftLabel);
    leftJoysticklayout->setAlignment(mAddJoystickLeftBtn,Qt::AlignHCenter);
    leftJoysticklayout->setAlignment(leftLabel,Qt::AlignHCenter);

    mAddJoystickRightBtn = new QPushButton(this);
    mAddJoystickRightBtn->setFixedSize(DEFAULT_BUTTON_SIZE,DEFAULT_BUTTON_SIZE);
    mAddJoystickRightBtn->setIcon(QIcon(":/new/res/icon_right_joystick2x.png"));
    mAddJoystickRightBtn->setIconSize({DEFAULT_ICON_SIZE,DEFAULT_ICON_SIZE});
    mAddJoystickRightBtn->setToolTip(tr("Click the button to create a custom game button"));
    mAddJoystickRightBtn->setFocusPolicy(Qt::NoFocus);
    mAddJoystickRightBtn->setStyleSheet("QToolButton{background:transparent}");
    connect(mAddJoystickRightBtn, &QPushButton::clicked, this, [this] {
        emit addJoystickKey(JoystickBaseKey::JOYRIGHT);
    });

    QLabel* rightLabel = new QLabel(this);
    rightLabel->setText(tr("R-Joy."));
    rightLabel->setFont(font_button);

    QVBoxLayout* rightJoysticklayout = new QVBoxLayout();
    rightJoysticklayout->addWidget(mAddJoystickRightBtn);
    rightJoysticklayout->addSpacing(3);
    rightJoysticklayout->addWidget(rightLabel);
    rightJoysticklayout->setAlignment(mAddJoystickRightBtn,Qt::AlignHCenter);
    rightJoysticklayout->setAlignment(rightLabel,Qt::AlignHCenter);

    QPushButton* mAddAutoMethod = new QPushButton(this);
    mAddAutoMethod->setFixedSize(DEFAULT_BUTTON_SIZE,DEFAULT_BUTTON_SIZE);
    mAddAutoMethod->setIcon(QIcon(":/new/res/icon_smart_attack2x.png"));
    mAddAutoMethod->setIconSize({DEFAULT_ICON_SIZE,DEFAULT_ICON_SIZE});
//    mAddAutoMethod->setToolTip(tr("Click the button to set auto methed!"));
    mAddAutoMethod->setFocusPolicy(Qt::NoFocus);
//    mAddAutoMethod->setStyleSheet("QToolButton{background:transparent}");
//  mAddAutoMethod->setEnabled(false);
    connect(mAddAutoMethod,&QPushButton::clicked,this,[this](){
        emit addJoystickKey(JoystickBaseKey::AUTOMETHOD);
    });

    QLabel* autoLabel = new QLabel(this);
    autoLabel->setText(tr("Skills"));
    autoLabel->setFont(font_button);

    QVBoxLayout* autolayout = new QVBoxLayout();
    autolayout->addWidget(mAddAutoMethod);
    autolayout->addSpacing(3);
    autolayout->addWidget(autoLabel);
    autolayout->setAlignment(mAddAutoMethod,Qt::AlignHCenter);
    autolayout->setAlignment(autoLabel,Qt::AlignHCenter);

    QGridLayout* mGridLayout = new QGridLayout();
    mGridLayout->setMargin(8); //控件到布局周围一圈的距离
    mGridLayout->setHorizontalSpacing(0);
    mGridLayout->setVerticalSpacing(10);
    mGridLayout->addLayout(singleKeylayout, 0, 0, 1, 1);
    mGridLayout->addLayout(comboKeylayout, 0, 1, 1, 1);
    mGridLayout->addLayout(autolayout, 0, 2, 1,1);
    mGridLayout->addLayout(leftJoysticklayout, 1,0, 1, 1);
    mGridLayout->addLayout(rightJoysticklayout, 1, 1, 1, 1);

//    QVBoxLayout* mSetKeyVBoxLayout = new QVBoxLayout();
////    mGridLayout->setMargin(0);
////    mGridLayout->setSpacing(0);   //这个设置会影响他的子布局的该属性设置
//    mSetKeyVBoxLayout->addWidget(mKeySetLabel);
//    mSetKeyVBoxLayout->addLayout(mGridLayout);
//    mSetKeyVBoxLayout->addStretch();

//    mTransparencySlider = new QSlider(this);
//    mTransparencySlider->setMinimum(10);
//    mTransparencySlider->setMaximum(100);
//    mTransparencySlider->setValue(50);
//    mTransparencySlider->setTracking(true);
//    mTransparencySlider->setOrientation(Qt::Horizontal);
//    connect(mTransparencySlider, &QSlider::sliderMoved, this, [this]() {
//        emit updateTransparency(mTransparencySlider->value() / 100.0);
//    });

//    QLabel* mTransparencyLabel = new QLabel(this);
//    mTransparencyLabel->setText(tr("按键透明度"));
//    mTransparencyLabel->setFont(font_button);
//    mTransparencyLabel->setAlignment(Qt::AlignCenter);

//    QLabel* mKeyTipsLabel = new QLabel(this);
//    mKeyTipsLabel->setText(tr("键位提醒"));
//    mKeyTipsLabel->setFont(font_button);

//    QPushButton* mKeySelectBtn = new QPushButton(this);
//    mKeySelectBtn->setCheckable(true);
//    mKeySelectBtn->setFixedSize(21,21);
//    mKeySelectBtn->setFocusPolicy(Qt::NoFocus);
//    mKeySelectBtn->setStyleSheet("QPushButton{background:transparent;background-image:url(:/new/res/icon_unselect.png);}"
//                              "QPushButton:hover{background:transparent;background-image:url(:/new/res/icon_selected.png);}"
//                                 "QPushButton:checked{background:transparent;background-image:url(:/new/res/icon_selected.png);}");

    QPushButton* mSaveBtn = new QPushButton(this);   //保存设置，意味着保存当前的所有按键设置，并且退出设置界面，不销毁资源
    mSaveBtn->setText(tr("Save"));
    mSaveBtn->setFocusPolicy(Qt::NoFocus);
    mSaveBtn->setFixedSize(80,30);
    connect(mSaveBtn, &QPushButton::clicked, this, [this] {
        emit saveGameKeys(true);
    });

//    QPushButton* mHideBtn = new QPushButton(this);   //退出设置,意味着放弃当前的所有按键设置，并且退出设置界面，销毁资源
//    mHideBtn->setText(tr("Hide"));
//    mHideBtn->setFocusPolicy(Qt::NoFocus);
//    connect(mHideBtn, &QPushButton::clicked, this, [this] {
//        emit hideGameKeys();

//    });

    QPushButton* mClearBtn = new QPushButton(this);  //清除所有标识
    mClearBtn->setText(tr("Clear"));
    mClearBtn->setFocusPolicy(Qt::NoFocus);
    mClearBtn->setFixedSize(80,30);
    connect(mClearBtn, &QPushButton::clicked, this, [this] {
        bool result = KylinUI::MessageBox::question(this->parent(),tr("Warning"),
                                     tr("Are you sure to delete all the keys?!"));
        if(result){
            emit clearGameKeys(true);
        }
    });

//    QGridLayout* mGridLayoutOther = new QGridLayout();
//    mGridLayoutOther->setMargin(8);
//    mGridLayoutOther->setHorizontalSpacing(0);
//    mGridLayoutOther->setVerticalSpacing(10);
//    mGridLayoutOther->addWidget(mKeyTipsLabel,0,0,1,4,Qt::AlignLeft);
//    mGridLayoutOther->addWidget(mKeySelectBtn,0,8,1,2);
//    mGridLayoutOther->addWidget(mTransparencyLabel, 1, 0, 1, 4,Qt::AlignLeft);
//    mGridLayoutOther->addWidget(mTransparencySlider,2,0, 1, 10);

    QHBoxLayout* mBtnBoxHBoxLayout = new QHBoxLayout();
    mBtnBoxHBoxLayout->addStretch(1);
    mBtnBoxHBoxLayout->addWidget(mClearBtn);
    mBtnBoxHBoxLayout->addStretch(1);
    mBtnBoxHBoxLayout->addWidget(mSaveBtn);
    mBtnBoxHBoxLayout->addStretch(1);

//    mGridLayoutOther->addWidget(mHideBtn,2, 2, 1, 1);

//    QLabel* mOtherLabel = new QLabel(this);
//    mOtherLabel->setText(tr("辅助设置"));
//    mOtherLabel->setFont(font_label);
//    mOtherLabel->setMargin(5);
//    mOtherLabel->setStyleSheet("QLabel{background-color:rgb(220,220,220);}");

//    QVBoxLayout* mOtherVBoxLayout = new QVBoxLayout();
//    mOtherVBoxLayout->addWidget(mOtherLabel);
//    mOtherVBoxLayout->addSpacing(15);
//    mOtherVBoxLayout->addLayout(mGridLayoutOther);
//    mOtherVBoxLayout->addStretch();
//    mOtherVBoxLayout->addLayout(mBtnBoxHBoxLayout);
//    mOtherVBoxLayout->addStretch();
    //mOtherVBoxLayout->setAlignment(mOtherLabel,Qt::AlignTop);

    mRightVBoxLayout->addSpacing(24);
    mRightVBoxLayout->addLayout(mGridLayout);
    mRightVBoxLayout->addStretch();
    mRightVBoxLayout->addLayout(mBtnBoxHBoxLayout);
    mRightVBoxLayout->addSpacing(25);
    mRightVBoxLayout->setMargin(0);
    mRightVBoxLayout->setSpacing(0);

    this->setLayout(mRightVBoxLayout);
}
JoystickSetWidget::~JoystickSetWidget(){

}
void JoystickSetWidget::enableLeftJoystickBtn(bool enable){
    mAddJoystickLeftBtn->setEnabled(enable);
}
void JoystickSetWidget::enableRightJoystickBtn(bool enable){
    mAddJoystickRightBtn->setEnabled(enable);
}
