///*
// * Copyright (C) 2020, KylinSoft Co., Ltd.
// *
// * This program is free software: you can redistribute it and/or modify
// * it under the terms of the GNU General Public License as published by
// * the Free Software Foundation, either version 3 of the License, or
// * (at your option) any later version.
// *
// * This program is distributed in the hope that it will be useful,
// * but WITHOUT ANY WARRANTY; without even the implied warranty of
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// * GNU General Public License for more details.
// *
// * You should have received a copy of the GNU General Public License
// * along with this program.  If not, see <https://www.gnu.org/licenses/>.
// */

//#include "allpupwindow.h"
//#include "widgetstyle.h"
//#include "UI/mainwidget.h"
//#include "xatom-helper.h"
//#include <ukui-log4qt.h>

//#define PT_12 12
//#define PT_11 11

//AllPupWindow::AllPupWindow(QWidget *parent) : QDialog(parent)
//{
//    inpupdialog();
//    dlgcolor();
//}

//void AllPupWindow::closeDialog()
//{
//    this->close();
//    enterLineEdit->clear();
//}

//void AllPupWindow::slotTextChanged(QString text)
//{
//    KyInfo() << "text = " << text;
//    if (!nameIsValid(text) && text != "") {
//        QMessageBox::warning(Widget::mutual,tr("Prompt information"),tr("Could not contain characters: \ / : * ? \" & < > |"), QMessageBox::Ok);

//        enterLineEdit->cursorBackward(true, 1);
//        enterLineEdit->del();
//    }

//    enterLineEdit->setLabelNumber(15 - enterLineEdit->text().length());
//    if (text.length() == 15) {
//        tips->setText(tr("Reached upper character limit"));
//        tips->setStyleSheet("QLabel{color:#F44E50;}");
//    } else {
//        tips->setText("");
//    }
//}

//void AllPupWindow::inpupdialog()
//{
//    this->setWindowTitle(tr("Music Player"));
//    this->setFixedSize(376, 222);
////    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

////    MotifWmHints hints;
////    hints.flags = MWM_HINTS_FUNCTIONS|MWM_HINTS_DECORATIONS;
////    hints.functions = MWM_FUNC_ALL;
////    hints.decorations = MWM_DECOR_BORDER;
////    XAtomHelper::getInstance()->setWindowMotifHint(this->winId(), hints);

//    this->setWindowModality(Qt::ApplicationModal); //弹出自定义对话框时主界面不可操作
////    pupDialog->setModal(true);
//    testLayout = new QVBoxLayout();
//    this->setLayout(testLayout);
//    titleLayout = new QHBoxLayout();
//    titleLab = new QLabel(this);
//    closeBtn = new QToolButton(this);
//    closeBtn->setFixedSize(30,30);
//    closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
//    closeBtn->setProperty("isWindowButton", 0x2);
//    closeBtn->setProperty("useIconHighlightEffect", 0x8);
//    closeBtn->setAutoRaise(true);
//    closeBtn->hide();

////    titleLab->setText("提示信息");
//    titleLab->setText(tr("Prompt information"));
//    titleLab->setFixedHeight(28);

//    titleLayout->addWidget(titleLab,Qt::AlignLeft);
//    titleLayout->addWidget(closeBtn,0,Qt::AlignRight);

//    titleLayout->setMargin(0);
//    titleLayout->setSpacing(0);

//    enterLineEdit = new LabEdit;
//    enterLineEdit->setFixedSize(312, 40);
//    enterLineEdit->setMaxLength(15);

//    tips = new QLabel(this);
//    tips->setFixedHeight(28);


//    confirmBtn = new QPushButton(this);

//    cancelBtn = new QPushButton(this);

//    btnLayout = new QHBoxLayout();

//    stackWid = new QStackedWidget();
//    stackWid->addWidget(enterLineEdit);

//    stackWid->setFixedSize(312,40);
////    confirmBtn->setText("确认");
//    confirmBtn->setText(tr("Confirm"));
//    confirmBtn->setFixedSize(105,36);
//    confirmBtn->setProperty("isImportant", true);
////    cancelBtn->setText("取消");
//    cancelBtn->setText(tr("Cancel"));
//    cancelBtn->setFixedSize(105,36);

//    btnLayout->addWidget(cancelBtn, 0, Qt::AlignRight);
//    btnLayout->addWidget(confirmBtn, Qt::AlignRight);
//    btnLayout->setSpacing(20);
//    btnLayout->setMargin(0);

//    enterLineEdit->setPlaceholderText("请输入歌单标题：");
//    enterLineEdit->setPlaceholderText(tr("Please input playlist name:"));

//    testLayout->setMargin(0);
//    testLayout->setSpacing(0);
//    testLayout->addLayout(titleLayout);
//    testLayout->addSpacing(16);
//    testLayout->addWidget(stackWid);
//    testLayout->addSpacing(2);
//    testLayout->addWidget(tips);
//    testLayout->addSpacing(14);
//    testLayout->addLayout(btnLayout);
//    testLayout->setContentsMargins(32, 16, 32, 24);

//    this->setAutoFillBackground(true);
//    this->setBackgroundRole(QPalette::Base);

////    connect(addSongListBtn, SIGNAL(clicked(bool)), this, SLOT(addSongList()));
//    connect(enterLineEdit,SIGNAL(textChanged(QString)),this,SLOT(slotTextChanged(QString)));
//    connect(closeBtn,SIGNAL(clicked(bool)),this,SLOT(closeDialog()));
//    connect(cancelBtn,SIGNAL(clicked(bool)),this,SLOT(closeDialog()));
//    connect(enterLineEdit, &QLineEdit::textChanged, this, &AllPupWindow::checkInput);

//    //限制应用内字体固定大小
////    QFont sizeFont;
////    sizeFont.setPixelSize(14);
////    confirmBtn->setFont(sizeFont);
////    cancelBtn->setFont(sizeFont);
////    titleLab->setFont(sizeFont);
////    enterLineEdit->setFont(sizeFont);

//}
//void AllPupWindow::checkInput()
//{
//    if (enterLineEdit->text().isEmpty()) {
//        confirmBtn->setDisabled(true);
//    } else {
//        confirmBtn->setDisabled(false);
//    }
//}

//void AllPupWindow::slotLableSetFontSize(int size)
//{
//    //默认大小12px,换算成pt为9
//    double lableBaseFontSize = PT_11;//魔鬼数字，自行处理
//    double nowFontSize = lableBaseFontSize * double(size) / 11;//11为系统默认大小，魔鬼数字，自行处理
//    QFont font;
//    font.setPointSizeF(nowFontSize);
//    tips->setFont(font);
//}

//void AllPupWindow::dlgcolor()
//{
//    if(WidgetStyle::themeColor == 1 )
//    {
////        cancelBtn->setStyleSheet("PushButton{border-radius:4px;border:1px solid #DDDFE7;color:#606265;}"
////                                 "QPushButton::hover{background:#4D4D4D;}");
////        cancelBtn->setStyleSheet("QPushButton{border-radius:4px;border:1px solid #DDDFE7;color:#606265;}"
////                                 "QPushButton::hover{background:#4D4D4D;}");

////        enterLineEdit->setStyleSheet("width:392px;height:32px;border-radius:4px;\
////                                     border: 1px solid #606265;color:#F9F9F9;background:#3D3D41;");

////        pupDialog->setStyleSheet("background-color:#3D3D41;");

////        confirmBtn->setStyleSheet("QPushButton{background:#3790FA;border-radius:4px;color:#FFFFFF;}"
////                                  "QPushButton::hover{background:#40A9FB;}"
////                                  "QPushButton::pressed{background:#296CD9;}"
////                                  );

////        titleLab->setStyleSheet("width:80px;height:14px;\
////                                font-weight: 600;\
////                                border:none;\
////                                color: #F9F9F9;\
////                                line-height:14px;");
//    }
//    else if(WidgetStyle::themeColor == 0)
//    {
////        cancelBtn->setStyleSheet("QPushButton{border-radius:4px;border:1px solid #DDDFE7;color:#8F9399;}"
////                                 "QPushButton::hover{background:#F2F6FD;}");

////        enterLineEdit->setStyleSheet("width:392px;height:32px;border-radius:4px;border:1px solid #DDDFE7;");

////        pupDialog->setStyleSheet("background-color:#FFFFFF;");

////        confirmBtn->setStyleSheet("QPushButton{background:#3790FA;border-radius:4px;color:#FFFFFF;}"
////                                  "QPushButton::hover{background:#40A9FB;}"
////                                  "QPushButton::pressed{background:#296CD9;}"
////                                  );

////        titleLab->setStyleSheet("width:80px;height:14px;\
////                                font-weight: 600;\
////                                border:none;\
////                                color: #1B1B1B;\
////                                line-height:14px;");
//    }
//}

