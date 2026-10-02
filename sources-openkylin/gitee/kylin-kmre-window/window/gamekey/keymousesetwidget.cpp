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

#include "keymousesetwidget.h"
#include "JoystickCommon.h"

#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QGridLayout>
#include <QToolButton>
#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include "joystickbasekey.h"



KeyMouseSetWidget::KeyMouseSetWidget(QWidget *parent) : QWidget(parent)
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

    QPushButton* mAddDirectionBtn = new QPushButton(this);
    mAddDirectionBtn->setFixedSize(DEFAULT_BUTTON_SIZE,DEFAULT_BUTTON_SIZE);
    mAddDirectionBtn->setIcon(QIcon(":/new/res/icon_direction2x.png"));
    mAddDirectionBtn->setIconSize({DEFAULT_ICON_SIZE,DEFAULT_ICON_SIZE});
//    mAddDirectionBtn->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
//    mAddDirectionBtn->setToolTip(tr("Click the button to create a custom game button"));
    mAddDirectionBtn->setFocusPolicy(Qt::NoFocus);
//    mAddSingleKeyBtn->setBackgroundRole(QPalette::);
//    mAddSingleKeyBtn->setAutoFillBackground(true);
//    mAddDirectionBtn->setStyleSheet("QPushButton{background:transparent}");
    connect(mAddDirectionBtn, &QPushButton::clicked, this, [this] {
        emit addKeyMouseKey(JoystickBaseKey::KeyType::DIRECTION);
    });

    QLabel* directionLabel = new QLabel(this);
    directionLabel->setText(tr("Direction"));
    directionLabel->setFont(font_button);

    QVBoxLayout* directionlayout = new QVBoxLayout();
    directionlayout->addWidget(mAddDirectionBtn);
    directionlayout->addSpacing(3);
    directionlayout->addWidget(directionLabel);
    directionlayout->setAlignment(mAddDirectionBtn,Qt::AlignHCenter);
    directionlayout->setAlignment(directionLabel,Qt::AlignHCenter);

    QPushButton* mAddSingleKeyBtn = new QPushButton(this);
    mAddSingleKeyBtn->setFixedSize(DEFAULT_BUTTON_SIZE,DEFAULT_BUTTON_SIZE);
    mAddSingleKeyBtn->setIcon(QIcon(":/new/res/icon_click2x.png"));
    mAddSingleKeyBtn->setIconSize({DEFAULT_ICON_SIZE,DEFAULT_ICON_SIZE});
//    mAddSingleKeyBtn->setToolTip(tr("Click the button to create a custom game button"));
    mAddSingleKeyBtn->setFocusPolicy(Qt::NoFocus);
//    mAddSingleKeyBtn->setStyleSheet("QPushButton{background:transparent}");
    connect(mAddSingleKeyBtn, &QPushButton::clicked, this, [this] {
        emit addKeyMouseKey(JoystickBaseKey::KeyType::SINGLEKEY);
    });

    QLabel* singleLabel = new QLabel(this);
    singleLabel->setText(tr("Click"));
    singleLabel->setFont(font_button);

    QVBoxLayout* singlelayout = new QVBoxLayout();
    singlelayout->addWidget(mAddSingleKeyBtn);
    singlelayout->addSpacing(3);
    singlelayout->addWidget(singleLabel);
    singlelayout->setAlignment(mAddSingleKeyBtn,Qt::AlignHCenter);
    singlelayout->setAlignment(singleLabel,Qt::AlignHCenter);

    QPushButton* mAddViewBtn = new QPushButton(this);
    mAddViewBtn->setFixedSize(DEFAULT_BUTTON_SIZE,DEFAULT_BUTTON_SIZE);
    mAddViewBtn->setIcon(QIcon(":/new/res/icon_view2x.png"));
    mAddViewBtn->setIconSize({DEFAULT_ICON_SIZE,DEFAULT_ICON_SIZE});
//    mAddViewBtn->setToolTip(tr("Click the button to create a custom game button"));
    mAddViewBtn->setFocusPolicy(Qt::NoFocus);
//    mAddViewBtn->setStyleSheet("QPushButton{background:transparent}");
    connect(mAddViewBtn, &QPushButton::clicked, this, [this] {
        emit addKeyMouseKey(JoystickBaseKey::VIEW);
    });

    QLabel* viewLabel = new QLabel(this);
    viewLabel->setText(tr("View"));
    viewLabel->setFont(font_button);

    QVBoxLayout* viewlayout = new QVBoxLayout();
    viewlayout->addWidget(mAddViewBtn);
    viewlayout->addSpacing(3);
    viewlayout->addWidget(viewLabel);
    viewlayout->setAlignment(mAddViewBtn,Qt::AlignHCenter);
    viewlayout->setAlignment(viewLabel,Qt::AlignHCenter);

    QPushButton* mAddShotBtn = new QPushButton(this);
    mAddShotBtn->setFixedSize(DEFAULT_BUTTON_SIZE,DEFAULT_BUTTON_SIZE);
    mAddShotBtn->setIcon(QIcon(":/new/res/icon_shot2x.png"));
    mAddShotBtn->setIconSize({DEFAULT_ICON_SIZE,DEFAULT_ICON_SIZE});
//    mAddShotBtn->setToolTip(tr("Click the button to set auto methed!"));
    mAddShotBtn->setFocusPolicy(Qt::NoFocus);
//    mAddShotBtn->setStyleSheet("QPushButton{background:transparent}");
    connect(mAddShotBtn, &QPushButton::clicked, this, [this] {
        emit addKeyMouseKey(JoystickBaseKey::CROSSHAIR);
    });

    QLabel* mAddShotLabel = new QLabel(this);
    mAddShotLabel->setText(tr("Crosshair"));
    mAddShotLabel->setFont(font_button);

    QVBoxLayout* mAddShotlayout = new QVBoxLayout();
    mAddShotlayout->addWidget(mAddShotBtn);
    mAddShotlayout->addSpacing(3);
    mAddShotlayout->addWidget(mAddShotLabel);
    mAddShotlayout->setAlignment(mAddShotBtn,Qt::AlignHCenter);
    mAddShotlayout->setAlignment(mAddShotLabel,Qt::AlignHCenter);

    QPushButton* mAddFireBtn = new QPushButton(this);
    mAddFireBtn->setFixedSize(DEFAULT_BUTTON_SIZE,DEFAULT_BUTTON_SIZE);
    mAddFireBtn->setIcon(QIcon(":/new/res/icon_fire2x.png"));
    mAddFireBtn->setIconSize({DEFAULT_ICON_SIZE,DEFAULT_ICON_SIZE});
//    mAddFireBtn->setToolTip(tr("Click the button to set auto methed!"));
    mAddFireBtn->setFocusPolicy(Qt::NoFocus);
//    mAddFireBtn->setStyleSheet("QPushButton{background:transparent}");
    connect(mAddFireBtn, &QPushButton::clicked, this, [this] {
        emit addKeyMouseKey(JoystickBaseKey::FIRE);
    });

    QLabel* mAddFireLabel = new QLabel(this);
    mAddFireLabel->setText(tr("Fire"));
    mAddFireLabel->setFont(font_button);

    QVBoxLayout* mAddFirelayout = new QVBoxLayout();
    mAddFirelayout->addWidget(mAddFireBtn);
    mAddFirelayout->addSpacing(3);
    mAddFirelayout->addWidget(mAddFireLabel);
    mAddFirelayout->setAlignment(mAddFireBtn,Qt::AlignHCenter);
    mAddFirelayout->setAlignment(mAddFireLabel,Qt::AlignHCenter);

    QPushButton* mAddComboBtn = new QPushButton(this);
    mAddComboBtn->setFixedSize(DEFAULT_BUTTON_SIZE,DEFAULT_BUTTON_SIZE);
    mAddComboBtn->setIcon(QIcon(":/new/res/icon_combo2x.png"));
    mAddComboBtn->setIconSize({DEFAULT_ICON_SIZE,DEFAULT_ICON_SIZE});
//    mAddComboBtn->setToolTip(tr("Click the button to set auto methed!"));
    mAddComboBtn->setFocusPolicy(Qt::NoFocus);
//    mAddComboBtn->setStyleSheet("QPushButton{background:transparent}");
    connect(mAddComboBtn, &QPushButton::clicked, this, [this] {
        emit addKeyMouseKey(JoystickBaseKey::KeyType::COMBOKEY);
    });

    QLabel* mAddComboLabel = new QLabel(this);
    mAddComboLabel->setText(tr("Combo"));
    mAddComboLabel->setFont(font_button);

    QVBoxLayout* mAddCombolayout = new QVBoxLayout();
    mAddCombolayout->addWidget(mAddComboBtn);
    mAddCombolayout->addSpacing(3);
    mAddCombolayout->addWidget(mAddComboLabel);
    mAddCombolayout->setAlignment(mAddComboBtn,Qt::AlignHCenter);
    mAddCombolayout->setAlignment(mAddComboLabel,Qt::AlignHCenter);

    QPushButton* mAddAcceleBtn = new QPushButton(this);
    mAddAcceleBtn->setFixedSize(DEFAULT_BUTTON_SIZE,DEFAULT_BUTTON_SIZE);
    mAddAcceleBtn->setIcon(QIcon(":/new/res/icon_accelerometer2x.png"));
    mAddAcceleBtn->setIconSize({DEFAULT_ICON_SIZE,DEFAULT_ICON_SIZE});
//    mAddAcceleBtn->setToolTip(tr("Click the button to set auto methed!"));
    mAddAcceleBtn->setFocusPolicy(Qt::NoFocus);
//    mAddAcceleBtn->setStyleSheet("QPushButton{background:transparent}");
    connect(mAddAcceleBtn, &QPushButton::clicked, this, [this] {
        emit addKeyMouseKey(JoystickBaseKey::ACCELATE);
    });

    QLabel* mAddAcceleLabel = new QLabel(this);
    mAddAcceleLabel->setText(tr("Accele"));
    mAddAcceleLabel->setFont(font_button);

    QVBoxLayout* mAddAccelelayout = new QVBoxLayout();
    mAddAccelelayout->addWidget(mAddAcceleBtn);
    mAddAccelelayout->addSpacing(3);
    mAddAccelelayout->addWidget(mAddAcceleLabel);
    mAddAccelelayout->setAlignment(mAddAcceleBtn,Qt::AlignHCenter);
    mAddAccelelayout->setAlignment(mAddAcceleLabel,Qt::AlignHCenter);

    QPushButton* mAddSmartAttackBtn = new QPushButton(this);
    mAddSmartAttackBtn->setFixedSize(DEFAULT_BUTTON_SIZE,DEFAULT_BUTTON_SIZE);
    mAddSmartAttackBtn->setIcon(QIcon(":/new/res/icon_smart_attack2x.png"));
    mAddSmartAttackBtn->setIconSize({DEFAULT_ICON_SIZE,DEFAULT_ICON_SIZE});
//    mAddSmartAttackBtn->setToolTip(tr("Click the button to set auto methed!"));
    mAddSmartAttackBtn->setFocusPolicy(Qt::NoFocus);
//    mAddSmartAttackBtn->setStyleSheet("QPushButton{background:transparent}");
    connect(mAddSmartAttackBtn, &QPushButton::clicked, this, [this] {
        emit addKeyMouseKey(JoystickBaseKey::AUTOMETHOD);
    });

    QLabel* mAddSmartAttackLabel = new QLabel(this);
    mAddSmartAttackLabel->setText(tr("Skills"));
    mAddSmartAttackLabel->setFont(font_button);

    QVBoxLayout* mAddSmartAttacklayout = new QVBoxLayout();
    mAddSmartAttacklayout->addWidget(mAddSmartAttackBtn);
    mAddSmartAttacklayout->addSpacing(3);
    mAddSmartAttacklayout->addWidget(mAddSmartAttackLabel);
    mAddSmartAttacklayout->setAlignment(mAddSmartAttackBtn,Qt::AlignHCenter);
    mAddSmartAttacklayout->setAlignment(mAddSmartAttackLabel,Qt::AlignHCenter);

    QGridLayout* mGridLayout = new QGridLayout();
    mGridLayout->setMargin(8); //控件到布局周围一圈的距离
    mGridLayout->setHorizontalSpacing(0);
    mGridLayout->setVerticalSpacing(10);
    mGridLayout->addLayout(directionlayout, 0, 0, 1, 1);
    mGridLayout->addLayout(singlelayout, 0,1, 1, 1);
    mGridLayout->addLayout(viewlayout, 0, 2, 1, 1);
    mGridLayout->addLayout(mAddShotlayout, 1, 0, 1, 1);
    mGridLayout->addLayout(mAddFirelayout, 1,1, 1, 1);
    mGridLayout->addLayout(mAddCombolayout, 1, 2, 1, 1);
    mGridLayout->addLayout(mAddAccelelayout, 2, 0, 1, 1);
    mGridLayout->addLayout(mAddSmartAttacklayout,2, 1, 1, 1);


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
////    connect(mTransparencySlider, &QSlider::sliderMoved, this, [this]() {
////        emit updateTransparency(mTransparencySlider->value() / 100.0);
////    });

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
        emit saveGameKeys(false);
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
        emit clearGameKeys(false);
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

////    mGridLayoutOther->addWidget(mHideBtn,2, 2, 1, 1);

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
KeyMouseSetWidget::~KeyMouseSetWidget(){

}
