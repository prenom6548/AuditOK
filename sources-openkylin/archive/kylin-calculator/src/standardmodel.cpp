/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "standardmodel.h"
#include "data_warehouse.h"

#include <QPalette>

StandardOutput::StandardOutput(QWidget *parent)
{
    // 初始化组件
    this->setWidgetUi();
    // 设置组件样式
    this->setWidgetStyle();
}

// 初始化组件
void StandardOutput::setWidgetUi()
{
    QFont staLabLastFont("SourceHanSansCN-Light", 40, 15);
    this->staLabLast->setAlignment(Qt::AlignRight | Qt::AlignTop);
    this->staLabLast->setFont(staLabLastFont);
    this->staLabLast->setText("");
    this->staLabLast->show();

    QFont staLabPreFont("SourceHanSansCN-Light", 40, 15);
    this->staLabPre->setAlignment(Qt::AlignRight | Qt::AlignTop);
    this->staLabPre->setFont(staLabPreFont);
    this->staLabPre->setText("");
    this->staLabPre->show();

    if (DataWarehouse::getInstance()->platform == QString("intel")) {
        QFont staLabNowFont("OPPOSans", 48, 15);
        staLabNowFont.setPixelSize(48);
        this->staLabNow->setAlignment(Qt::AlignRight | Qt::AlignBottom);
        this->staLabNow->setFont(staLabNowFont);
        this->staLabNow->setText("0");
        this->staLabNow->setMinimumHeight(100);
        this->staLabNow->show();
    } else {
        QFont staLabNowFont("SourceHanSansCN-Normal", 48, 15);
        this->staLabNow->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        this->staLabNow->setFont(staLabNowFont);
        this->staLabNow->setText("0");
        this->staLabNow->show();
    }

    QVBoxLayout *staOutputLayout = new QVBoxLayout();

    /* intel output ui */
    if (DataWarehouse::getInstance()->platform == QString("intel")) {
        this->staLabLast->hide();
        this->staLabPre->hide();

        staOutputLayout->addWidget(this->staLabNow);
        staOutputLayout->setSpacing(0);

        this->setLayout(staOutputLayout);

        return;
    }

    this->staLabLast->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->staLabPre->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->staLabNow->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    staOutputLayout->addWidget(this->staLabLast);
    staOutputLayout->addWidget(this->staLabPre);
    staOutputLayout->addLayout(staLabNowLayout);

    staOutputLayout->setStretchFactor(this->staLabLast, 14);
    staOutputLayout->setStretchFactor(this->staLabPre, 1);
    staOutputLayout->setStretchFactor(this->staLabNowLayout, 1);
    staOutputLayout->setSpacing(0);

    this->setLayout(staOutputLayout);

    return;
}

// 设置组件样式
void StandardOutput::setWidgetStyle(bool resetFontSize)
{
    if (WidgetStyle::themeColor == WidgetStyle::ThemeColor::LIGHT) {
        this->staLabLast->setStyleSheet("color:#8C8C8C;font-size:12px;margin:0 7px 0 7px;");
        this->staLabPre->setStyleSheet("color:#FB9119;font-size:20px;margin:0 7px 0 7px;");

        if (resetFontSize) {
            this->staLabNow->setStyleSheet("color:#272A2D;font-size:48px;font-weight:15px;margin:0 0 5px 7px;");
        } else {
            QString fontSizeStr = QString::number(staLabNow->font().pixelSize());

            this->staLabNow->setStyleSheet(QString("color:#272A2D;font-size:") + fontSizeStr
                                           + QString("px;font-weight:15px;margin:0 0 5px 7px;"));
        }
    } else if (WidgetStyle::themeColor == WidgetStyle::ThemeColor::DARK) {
        this->staLabLast->setStyleSheet("color:#8C8C8C;font-size:12px;margin:0 7px 0 7px;");
        this->staLabPre->setStyleSheet("color:#FB9119;font-size:20px;margin:0 7px 0 7px;");

        if (resetFontSize) {
            this->staLabNow->setStyleSheet("color:#FFFFFF;font-size:48px;font-weight:15px;margin:0 0 5px 7px;");
        } else {
            QString fontSizeStr = QString::number(staLabNow->font().pixelSize());
            this->staLabNow->setStyleSheet(QString("color:#FFFFFF;font-size:") + fontSizeStr
                                           + QString("px;font-weight:15px;margin:0 0 5px 7px;"));
        }
    }
}

StandardModel::StandardModel(QWidget *parent) : QWidget(parent)
{
    if (DataWarehouse::getInstance()->platform == QString("intel")) {
        this->createIntelModel();
        this->createIntelStyle();
    } else {
        // 初始化组件
        this->setWidgetUi();
    }
}

void StandardModel::createIntelModel(void)
{
    qDebug() << "Info : create intel stand mode ui...";

    /* 实例按钮 */
    for (int i = 0; i < 10; i++) {
        btnNum[i] = new BasicButton(this);
        btnNum[i]->setText(QString::number(i));
        btnNum[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        btnNum[i]->setIcon(QIcon(":/image/intelStandLight/" + QString::number(i) + ".svg"));
    }

    btnClear = new BasicButton(this);  /* 清除键 */
    btnClear->setAccessibleName("kylin-calculator_BasicButton_btnClear");
    btnPer = new BasicButton(this);    /* 百分号 */
    btnPer->setAccessibleName("kylin-calculator_BasicButton_btnPer");
    btnDelete = new BasicButton(this); /* 删除键 */
    btnDelete->setAccessibleName("kylin-calculator_BasicButton_btnDelete");
    btnDiv = new BasicButton(this);    /* 除号 */
    btnDiv->setAccessibleName("kylin-calculator_BasicButton_btnDiv");
    btnMulti = new BasicButton(this);  /* 乘号 */
    btnMulti->setAccessibleName("kylin-calculator_BasicButton_btnMulti");
    btnSub = new BasicButton(this);    /* 减号 */
    btnSub->setAccessibleName("kylin-calculator_BasicButton_btnSub");
    btnAdd = new BasicButton(this);    /* 加号 */
    btnAdd->setAccessibleName("kylin-calculator_BasicButton_btnAdd");
    btnEqual = new BasicButton(this);  /* 等号 */
    btnEqual->setAccessibleName("kylin-calculator_BasicButton_btnEqual");
    btnPoint = new BasicButton(this);  /* 小数点 */
    btnPoint->setAccessibleName("kylin-calculator_BasicButton_btnPoint");

    /* 设置按钮的显示文本 */
    QString btnList = "C,÷,×,B,ｰ,+,=,%,.";
    QStringList btnNameList = btnList.split(",");
    int index = 0;
    btnClear->setText(btnNameList[index++]);
    btnDiv->setText(btnNameList[index++]);
    btnMulti->setText(btnNameList[index++]);
    btnDelete->setText(btnNameList[index++]);
    btnSub->setText(btnNameList[index++]);
    btnAdd->setText(btnNameList[index++]);
    btnEqual->setText(btnNameList[index++]);
    btnPer->setText(btnNameList[index++]);
    btnPoint->setText(btnNameList[index++]);

    btnClear->setIcon(QIcon(":/image/intelStandLight/ac.svg"));
    btnDiv->setIcon(QIcon(":/image/intelStandLight/except.svg"));
    btnMulti->setIcon(QIcon(":/image/intelStandLight/ride.svg"));
    btnDelete->setIcon(QIcon(":/image/intelStandLight/delete.svg"));
    btnSub->setIcon(QIcon(":/image/intelStandLight/reduce.svg"));
    btnAdd->setIcon(QIcon(":/image/intelStandLight/add.svg"));
    btnEqual->setIcon(QIcon(":/image/intelStandLight/equal.svg"));
    btnPer->setIcon(QIcon(":/image/intelStandLight/per.svg"));
    btnPoint->setIcon(QIcon(":/image/intelStandLight/spot.svg"));

    /* 设置按钮自适应放缩 */
    btnClear->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnDiv->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnMulti->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnDelete->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnSub->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnAdd->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnEqual->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnPer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnPoint->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    changeBtnIconSize();

    /* 将按钮进行网格布局 */
    QGridLayout *btnLayout = new QGridLayout();
    btnLayout->addWidget(btnClear, 0, 0, 1, 1);
    btnLayout->addWidget(btnPer, 0, 1, 1, 1);
    btnLayout->addWidget(btnDelete, 0, 2, 1, 1);
    btnLayout->addWidget(btnAdd, 0, 3, 1, 1);
    btnLayout->addWidget(btnSub, 1, 3, 1, 1);
    btnLayout->addWidget(btnMulti, 2, 3, 1, 1);
    btnLayout->addWidget(btnDiv, 3, 3, 1, 1);
    btnLayout->addWidget(btnPoint, 4, 2, 1, 1);
    btnLayout->addWidget(btnEqual, 4, 3, 1, 1);

    /* 数字按钮布局 */
    btnLayout->addWidget(btnNum[0], 4, 0, 1, 2);
    for (int i = 1; i < 10; i++) {
        btnLayout->addWidget(btnNum[i], 3 - (i - 1) / 3, (i - 1) % 3, 1, 1);
    }

    /* 设置间距和背景样式 */
    btnLayout->setSpacing(2);
    btnLayout->setMargin(1);
    this->setLayout(btnLayout);
}

void StandardModel::createIntelStyle()
{
    qDebug() << "change intel stand mode ui style...";

    QString btnStyle;
    QString btnOpStyle;
    QString btnEqualStyle;
    QString intelPlugLeft;  /* intel 小插件左下圆角 */
    QString intelPlugRight; /* intel 小插件右下圆角 */

    if (WidgetStyle::themeColor == WidgetStyle::ThemeColor::LIGHT) {
        /* 填充背景色 */
        QPalette pal = this->palette();
        pal.setColor(QPalette::Background, QColor("#F6F6F6"));
        this->setAutoFillBackground(true);
        this->setPalette(pal);

        for (int i = 0; i < 10; i++) {
            btnNum[i]->setIcon(QIcon(":/image/intelStandLight/" + QString::number(i) + ".svg"));
        }

        btnClear->setIcon(QIcon(":/image/intelStandLight/ac.svg"));
        btnDiv->setIcon(QIcon(":/image/intelStandLight/except.svg"));
        btnMulti->setIcon(QIcon(":/image/intelStandLight/ride.svg"));
        btnDelete->setIcon(QIcon(":/image/intelStandLight/delete.svg"));
        btnSub->setIcon(QIcon(":/image/intelStandLight/reduce.svg"));
        btnAdd->setIcon(QIcon(":/image/intelStandLight/add.svg"));
        btnEqual->setIcon(QIcon(":/image/intelStandLight/equal.svg"));
        btnPer->setIcon(QIcon(":/image/intelStandLight/per.svg"));
        btnPoint->setIcon(QIcon(":/image/intelStandLight/spot.svg"));

        btnStyle = "QPushButton{"
                   "background-color:#FFFFFF;border-radius:0px;"
                   "font-size:42px;font-family:HelveticaNeueLTPro-UltLt;color:#FFFFFF;"
                   "}"
                   "QPushButton:pressed{background-color:#F6F6F6;}";

        btnOpStyle = "QPushButton{"
                     "background-color:#FFFFFF;border-radius:0px;"
                     "font-size:35px;font-family:HelveticaNeueLTPro-UltLt;color:#FFFFFF;"
                     "}"
                     "QPushButton:pressed{background-color:#F6F6F6;}";

        btnEqualStyle = "QPushButton{"
                        "background-color:#FB7054;border-radius:0px;"
                        "font-size:48px;font-family:HelveticaNeueLTPro-UltLt;color:#FFFFFF;"
                        "}"
                        "QPushButton:pressed{background-color:#EA5E42;}";

        intelPlugLeft = "QPushButton{"
                        "background-color:#FFFFFF;border-bottom-left-radius:32px;border-top-left-radius:0px;border-top-"
                        "right-radius:0px;border-bottom-right-radius:0px;"
                        "font-size:42px;font-family:HelveticaNeueLTPro-UltLt;color:#FFFFFF;"
                        "}"
                        "QPushButton:pressed{background-color:#F6F6F6;}";

        intelPlugRight = "QPushButton{"
                         "background-color:#FB7054;border-bottom-right-radius:32px;border-top-left-radius:0px;border-"
                         "top-right-radius:0px;border-bottom-left-radius:0px;"
                         "font-size:48px;font-family:HelveticaNeueLTPro-UltLt;color:#FFFFFF;"
                         "}"
                         "QPushButton:pressed{background-color:#EA5E42;}";

    } else if (WidgetStyle::themeColor == WidgetStyle::ThemeColor::DARK) {
        /* 填充背景色 */
        QPalette pal = this->palette();
        pal.setColor(QPalette::Background, Qt::black);
        this->setAutoFillBackground(true);
        this->setPalette(pal);

        for (int i = 0; i < 10; i++) {
            btnNum[i]->setIcon(QIcon(":/image/intelStandDark/" + QString::number(i) + ".svg"));
        }

        btnClear->setIcon(QIcon(":/image/intelStandDark/ac.svg"));
        btnDiv->setIcon(QIcon(":/image/intelStandDark/except.svg"));
        btnMulti->setIcon(QIcon(":/image/intelStandDark/ride.svg"));
        btnDelete->setIcon(QIcon(":/image/intelStandDark/delete.svg"));
        btnSub->setIcon(QIcon(":/image/intelStandDark/reduce.svg"));
        btnAdd->setIcon(QIcon(":/image/intelStandDark/add.svg"));
        btnEqual->setIcon(QIcon(":/image/intelStandDark/equal.svg"));
        btnPer->setIcon(QIcon(":/image/intelStandDark/per.svg"));
        btnPoint->setIcon(QIcon(":/image/intelStandDark/spot.svg"));

        btnStyle = "QPushButton{"
                   "width:100px;height:80px;opacity:0.95;"
                   "background-color:#27292C;border-radius:0px;"
                   "font-size:42px;font-family:HelveticaNeueLTPro-UltLt;color:#FFFFFF;"
                   "}"
                   "QPushButton:hover{background-color:#474747;}";

        btnOpStyle = "QPushButton{"
                     "width:100px;height:80px;opacity:1;"
                     "background-color:#222E36;border-radius:0px;"
                     "font-size:35px;font-family:HelveticaNeueLTPro-UltLt;color:#FFFFFF;}"
                     "QPushButton:hover{background-color:#474747;}";


        btnEqualStyle = "QPushButton{"
                        "width:200px;height:80px;opacity:1;"
                        "background-color:#E64A19;border-radius:0px;"
                        "font-size:48px;font-family:HelveticaNeueLTPro-UltLt;color:#FFFFFF;"
                        "}"
                        "QPushButton:hover{background-color:#E45E4C;}";

        intelPlugLeft = "QPushButton{"
                        "width:100px;height:80px;opacity:0.95;"
                        "background-color:#27292C;border-bottom-left-radius:32px;border-top-left-radius:0px;border-top-"
                        "right-radius:0px;border-bottom-right-radius:0px;"
                        "font-size:42px;font-family:HelveticaNeueLTPro-UltLt;color:#FFFFFF;"
                        "}"
                        "QPushButton:hover{background-color:#474747;}";

        intelPlugRight = "QPushButton{"
                         "width:200px;height:80px;opacity:1;"
                         "background-color:#E64A19;border-bottom-right-radius:32px;border-top-left-radius:0px;border-"
                         "top-right-radius:0px;border-bottom-left-radius:0px;"
                         "font-size:48px;font-family:HelveticaNeueLTPro-UltLt;color:#FFFFFF;"
                         "}"
                         "QPushButton:hover{background-color:#E45E4C;}";
    }

    /* 设置按钮风格 */
    for (int i = 0; i < 10; i++) {
        btnNum[i]->setStyleSheet(btnStyle);
    }

    // 设置按钮样式standardModel
    btnClear->setStyleSheet(btnStyle);
    btnPer->setStyleSheet(btnStyle);
    btnDelete->setStyleSheet(btnStyle);
    btnPoint->setStyleSheet(btnStyle);

    btnDiv->setStyleSheet(btnOpStyle);
    btnMulti->setStyleSheet(btnOpStyle);
    btnSub->setStyleSheet(btnOpStyle);
    btnAdd->setStyleSheet(btnOpStyle);

    btnEqual->setStyleSheet(btnEqualStyle);

    /* intel 小插件圆角处理 */
    if (DataWarehouse::getInstance()->platform == QString("intel")
        && (DataWarehouse::getInstance()->intelPlug == DataWarehouse::StartMode::MODE_PLUG
            || DataWarehouse::getInstance()->intelPlug == DataWarehouse::StartMode::MODE_PLUG_AGAIN)) {
        btnNum[0]->setStyleSheet(intelPlugLeft);
        btnEqual->setStyleSheet(intelPlugRight);
        this->setStyleSheet("border-radius:32px;");
    }
    return;
}

void StandardModel::changeBtnIconSize()
{
    int iconWidth = 100;
    int iconHeight = 80;

    for (int i = 0; i < 10; i++) {
        if (i == 0) {
            btnNum[i]->setIconSize(QSize(iconWidth * 2, iconHeight));
        } else {
            btnNum[i]->setIconSize(QSize(iconWidth, iconHeight));
        }
    }
    btnClear->setIconSize(QSize(iconWidth, iconHeight));
    btnDiv->setIconSize(QSize(iconWidth, iconHeight));
    btnMulti->setIconSize(QSize(iconWidth, iconHeight));
    btnDelete->setIconSize(QSize(iconWidth, iconHeight));
    btnSub->setIconSize(QSize(iconWidth, iconHeight));
    btnAdd->setIconSize(QSize(iconWidth, iconHeight));
    btnEqual->setIconSize(QSize(iconWidth, iconHeight));
    btnPer->setIconSize(QSize(iconWidth, iconHeight));
    btnPoint->setIconSize(QSize(iconWidth, iconHeight));
}

// 初始化组件
void StandardModel::setWidgetUi()
{
    // 计算器按钮
    QString str_list = "C,÷,×,B,7,8,9,ｰ,4,5,6,+,1,2,3,,%,0,.,=";
    QStringList btn_name_list = str_list.split(",");

    for (int i = 0; i < 10; i++) {
        btnNum[i] = new BasicButton(this);
        btnNum[i]->setText(QString::number(i));
        btnNum[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        btnNum[i]->setIcon(QIcon(":/image/newIcon/standard/btnNum" + QString::number(i) + ".svg"));
        btnNum[i]->setIconSize(QSize(106, 62));
        btnNum[i]->setProperty("useButtonPalette", true);
        btnNum[i]->setAccessibleName(QString("kylin_calculator_BasicButton_btnNum%1").arg(i));
    }

    // btnDZero = new BasicButton(this);
    // btnDZero->setText("00");
    // btnDZero->setIcon(QIcon(":/image/standard/btnNum00.svg"));
    // btnDZero->setIconSize(QSize(106, 62));

    btnClear = new BasicButton(this);
    btnClear->setAccessibleName("kylin-calculator_BasicButton_btnClear");
    btnPer = new BasicButton(this);
    btnPer->setAccessibleName("kylin-calculator_BasicButton_btnPer");
    btnDelete = new BasicButton(this);
    btnDelete->setAccessibleName("kylin-calculator_BasicButton_btnDelete");

    btnDiv = new BasicButton(this);
    btnDiv->setAccessibleName("kylin-calculator_BasicButton_btnDiv");
    btnMulti = new BasicButton(this);
    btnMulti->setAccessibleName("kylin-calculator_BasicButton_btnMulti");
    btnSub = new BasicButton(this);
    btnSub->setAccessibleName("kylin-calculator_BasicButton_btnSub");
    btnAdd = new BasicButton(this);
    btnAdd->setAccessibleName("kylin-calculator_BasicButton_btnAdd");
    btnEqual = new BasicButton(this);
    btnEqual->setAccessibleName("kylin-calculator_BasicButton_btnEqual");

    btnPoint = new BasicButton(this);
    btnPoint->setAccessibleName("kylin-calculator_BasicButton_btnPoint");

    m_picturetowhite = new PictureToWhite();
    m_picturetowhite->setObjectName("kylin-calculator_PictureToWhite_m_picturetowhite");

    // 设置按钮的显示文本
    QString btnList = "C,÷,×,B,ｰ,+,=,%,.";
    QStringList btnNameList = btnList.split(",");
    int index = 0;
    btnClear->setText(btnNameList[index++]);
    btnDiv->setText(btnNameList[index++]);
    btnMulti->setText(btnNameList[index++]);
    btnDelete->setText(btnNameList[index++]);
    btnSub->setText(btnNameList[index++]);
    btnAdd->setText(btnNameList[index++]);
    btnEqual->setText(btnNameList[index++]);
    btnPer->setText(btnNameList[index++]);
    btnPoint->setText(btnNameList[index++]);

    btnClear->setIcon(QIcon(":/image/newIcon/standard/btnClear.svg"));
    btnDiv->setIcon(QIcon(":/image/newIcon/standard/btnDiv.svg"));
    btnMulti->setIcon(QIcon(":/image/newIcon/standard/btnMulti.svg"));
    btnDelete->setIcon(QIcon(":/image/newIcon/standard/btnDelete.svg"));
    btnSub->setIcon(QIcon(":/image/newIcon/standard/btnSub.svg"));
    btnAdd->setIcon(QIcon(":/image/newIcon/standard/btnAdd.svg"));
    btnEqual->setIcon(QIcon(":/image/newIcon/standard/btnEqual.svg"));
    btnPer->setIcon(QIcon(":/image/newIcon/standard/btnPer.svg"));
    btnPoint->setIcon(QIcon(":/image/newIcon/standard/btnPoint.svg"));
    // btnEqual->setProperty("useIconHighlightEffect", 0x2);
    btnEqual->setProperty("useIconHighlightEffect", 0x4);

    btnClear->setIconSize(QSize(106, 62));
    btnDiv->setIconSize(QSize(106, 62));
    btnMulti->setIconSize(QSize(106, 62));
    btnDelete->setIconSize(QSize(85, 50));
    btnSub->setIconSize(QSize(106, 62));
    btnAdd->setIconSize(QSize(106, 62));
    btnEqual->setIconSize(QSize(106, 62));
    btnPer->setIconSize(QSize(106, 62));
    btnPoint->setIconSize(QSize(106, 62));

    // 设置按钮自适应放缩
    btnClear->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnDiv->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnMulti->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnDelete->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnSub->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnAdd->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnEqual->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnPer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnPoint->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    btnClear->setProperty("useButtonPalette", true);
    btnDiv->setProperty("useButtonPalette", true);
    btnMulti->setProperty("useButtonPalette", true);
    btnDelete->setProperty("useButtonPalette", true);
    btnSub->setProperty("useButtonPalette", true);
    btnAdd->setProperty("useButtonPalette", true);
    btnPer->setProperty("useButtonPalette", true);
    btnPoint->setProperty("useButtonPalette", true);

    btnEqual->setProperty("isImportant", true);

    // 将按钮进行网格布局
    QGridLayout *btnLayout = new QGridLayout();
    //    btnLayout->addWidget(btnClear, 0, 0, 1, 1);
    //    btnLayout->addWidget(btnDiv, 0, 1, 1, 1);
    //    btnLayout->addWidget(btnMulti, 0, 2, 1, 1);
    //    btnLayout->addWidget(btnDelete, 0, 3, 1, 1);
    //    btnLayout->addWidget(btnSub, 1, 3, 1, 1);
    //    btnLayout->addWidget(btnAdd, 2, 3, 1, 1);
    //    btnLayout->addWidget(btnEqual, 3, 3, 2, 1);
    //    btnLayout->addWidget(btnPer, 4, 0, 1, 1);
    //    btnLayout->addWidget(btnPoint, 4, 2, 1, 1);

    btnLayout->addWidget(btnClear, 0, 0, 1, 1);
    btnLayout->addWidget(btnPer, 0, 1, 1, 1);
    btnLayout->addWidget(btnDelete, 0, 2, 1, 1);
    btnLayout->addWidget(btnDiv, 0, 3, 1, 1);
    btnLayout->addWidget(btnMulti, 1, 3, 1, 1);
    btnLayout->addWidget(btnSub, 2, 3, 1, 1);
    btnLayout->addWidget(btnAdd, 3, 3, 1, 1);
    btnLayout->addWidget(btnPoint, 4, 2, 1, 1);
    btnLayout->addWidget(btnEqual, 4, 3, 1, 1);

    // 数字按钮布局
    // btnLayout->addWidget(btnDZero, 4, 0, 1, 1);
    btnLayout->addWidget(btnNum[0], 4, 0, 1, 2);
    for (int i = 1; i < 10; i++) {
        btnLayout->addWidget(btnNum[i], 3 - (i - 1) / 3, (i - 1) % 3, 1, 1);
    }

    // 设置间距和背景样式
    btnLayout->setSpacing(2);
    btnLayout->setMargin(1);
    btnLayout->setContentsMargins(4, 0, 4, 4);
    // this->setFixedHeight(320);
    this->setLayout(btnLayout);
}

void StandardModel::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Period:
        btnPoint->click();
        break;
    case Qt::Key_0:
        btnNum[0]->click();
        break;
    case Qt::Key_1:
        btnNum[1]->click();
        break;
    case Qt::Key_2:
        btnNum[2]->click();
        break;
    case Qt::Key_3:
        btnNum[3]->click();
        break;
    case Qt::Key_4:
        btnNum[4]->click();
        break;
    case Qt::Key_5:
        btnNum[5]->click();
        break;
    case Qt::Key_6:
        btnNum[6]->click();
        break;
    case Qt::Key_7:
        btnNum[7]->click();
        break;
    case Qt::Key_8:
        btnNum[8]->click();
        break;
    case Qt::Key_9:
        btnNum[9]->click();
        break;
    case Qt::Key_Plus:
        btnAdd->click();
        break;
    case Qt::Key_Minus:
        btnSub->click();
        break;
    case Qt::Key_Asterisk:
        btnMulti->click();
        break;
    case Qt::Key_Slash:
        btnDiv->click();
        break;
    case Qt::Key_Escape:
        btnClear->click();
        break;
    case Qt::Key_Percent:
        btnPer->click();
        break;
    case Qt::Key_Backspace:
        btnDelete->click();
        break;
    case Qt::Key_Enter:
        btnEqual->click();
        break;
    case 16777220:
        btnEqual->click();
        break;
    case 0x3d:
        btnEqual->click();
        break;
    default:
        return QWidget::keyPressEvent(event);
    }

    return;
}
