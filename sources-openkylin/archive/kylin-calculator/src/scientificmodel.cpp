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

#include "scientificmodel.h"

ScientificOutput::ScientificOutput(QWidget *parent) : OutputBase(parent)
{
    // 初始化组件
    setWidgetUi();

    // 设置组件样式
    setWidgetStyle();
}

// 初始化组件
void ScientificOutput::setWidgetUi()
{
    QFont labLastFont("SourceHanSansCN-Light", 40, 15);

    this->staLabLast->setAlignment(Qt::AlignRight | Qt::AlignTop);
    this->staLabLast->setFont(labLastFont);
    this->staLabLast->setText("");

    QFont staLabPreFont("SourceHanSansCN-Light", 40, 15);
    this->staLabPre->setAlignment(Qt::AlignRight | Qt::AlignTop);
    this->staLabPre->setFont(staLabPreFont);
    this->staLabPre->setText("0");
    // this->staLabPre->setFixedHeight(35);

    QFont staLabNowFont("SourceHanSansCN-Normal", 48, 15);
    this->staLabNow->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    this->staLabNow->setFont(staLabNowFont);
    this->staLabNow->setText("0");
    // this->staLabNow->setFixedHeight(65);
    this->staLabNow->setContentsMargins(0, 0, 0, 0);

    this->staLabLast->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->staLabPre->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->staLabNow->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *rightLayout = new QVBoxLayout(this);
    rightLayout->addWidget(this->staLabLast);
    rightLayout->addWidget(this->staLabPre);
    rightLayout->addLayout(staLabNowLayout);
    rightLayout->setStretchFactor(this->staLabLast, 14);
    rightLayout->setStretchFactor(this->staLabPre, 1);
    rightLayout->setStretchFactor(this->staLabNowLayout, 1);
    rightLayout->setSpacing(0);
    rightLayout->setMargin(0);

    QWidget *outputRWid = new QWidget(this);
    outputRWid->setObjectName("kylin-calculator_QWidget_outputRWid");
    outputRWid->setLayout(rightLayout);

    sciOutputLayout = new QHBoxLayout(this);

    // sciOutputLayout->addWidget(staLabLastL);
    sciOutputLayout->addWidget(outputRWid);

    this->setLayout(sciOutputLayout);
    // this->setFixedHeight(270);
    this->setContentsMargins(0, 0, 0, 0);

    this->staLabLast->show();
    this->staLabPre->show();
    this->staLabNow->show();
}

// 设置组件样式
void ScientificOutput::setWidgetStyle(bool resetFontSize)
{
    if (WidgetStyle::themeColor == WidgetStyle::ThemeColor::LIGHT) {
        this->staLabLast->setStyleSheet("color:#8C8C8C;font-size:12px;margin:0 7px 0 7px;");
        this->staLabPre->setStyleSheet("color:#FB9119;font-size:20px;margin:0 7px 0 7px;");

        if (resetFontSize) {
            this->staLabNow->setStyleSheet("color:#272A2D;font-size:48px;font-weight:15px;margin:0 0 0 7px;");
        } else {
            QString fontSizeStr = QString::number(staLabNow->font().pixelSize());
            this->staLabNow->setStyleSheet("color:#272A2D;font-size:" + fontSizeStr
                                           + "px;font-weight:15px;margin:0 0 0 7px;");
        }
    } else if (WidgetStyle::themeColor == WidgetStyle::ThemeColor::DARK) {
        this->staLabLast->setStyleSheet("color:#8C8C8C;font-size:12px;margin:0 7px 0 7px;");
        this->staLabPre->setStyleSheet("color:#FB9119;font-size:20px;margin:0 7px 0 7px;");

        if (resetFontSize) {
            this->staLabNow->setStyleSheet("color:#FFFFFF;font-size:48px;font-weight:15px;margin:0 0 0 7px;");
        } else {
            QString fontSizeStr = QString::number(staLabNow->font().pixelSize());
            this->staLabNow->setStyleSheet("color:#FFFFFF;font-size:" + fontSizeStr
                                           + "px;font-weight:15px;margin:0 0 0 7px;");
        }
    }
}


ScientificModel::ScientificModel(QWidget *parent) : QWidget(parent)
{
    if (DataWarehouse::getInstance()->platform == QString("intel")) {
        this->createIntelModel();
        this->createIntelStyle();
    } else {
        // 初始化组件
        this->setWidgetUi();
    }
}

// 初始化组件
void ScientificModel::setWidgetUi()
{
    // 计算器按钮
    // 布局按钮
    for (int i = 0; i < 10; i++) {
        btnNum[i] = new BasicButton(this);
        btnNum[i]->setText(QString::number(i));
        btnNum[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        btnNum[i]->setIcon(QIcon(":/image/newIcon/scientific/btnNum" + QString::number(i) + ".svg"));
        btnNum[i]->setIconSize(QSize(106, 62));
        btnNum[i]->setProperty("useButtonPalette", true);
        btnNum[i]->setAccessibleName(QString("kylin_calculator_BasicButton_btnNum%1").arg(i));
    }

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

    btnInd = new BasicButton(this);
    btnInd->setAccessibleName("kylin-calculator_BasicButton_btnInd");
    btnUndo = new BasicButton(this);
    btnUndo->setAccessibleName("kylin-calculator_BasicButton_btnUndo");
    btnBracketLeft = new BasicButton(this);
    btnBracketLeft->setAccessibleName("kylin-calculator_BasicButton_btnBracketLeft");
    btnBracketRight = new BasicButton(this);
    btnBracketRight->setAccessibleName("kylin-calculator_BasicButton_btnBracketRight");

    btnReci = new BasicButton(this);
    btnReci->setAccessibleName("kylin-calculator_BasicButton_btnReci");
    btnXPower2 = new BasicButton(this);
    btnXPower2->setAccessibleName("kylin-calculator_BasicButton_btnXPower2");
    btnXPower3 = new BasicButton(this);
    btnXPower3->setAccessibleName("kylin-calculator_BasicButton_btnXPower3");
    btnYPowerX = new BasicButton(this);
    btnYPowerX->setAccessibleName("kylin-calculator_BasicButton_btnYPowerX");

    btnFac = new BasicButton(this);
    btnFac->setAccessibleName("kylin-calculator_BasicButton_btnFac");
    btnXSquare2 = new BasicButton(this);
    btnXSquare2->setAccessibleName("kylin-calculator_BasicButton_btnXSquare2");
    btnXSquare3 = new BasicButton(this);
    btnXSquare3->setAccessibleName("kylin-calculator_BasicButton_btnXSquare3");
    btnYSquareX = new BasicButton(this);
    btnYSquareX->setAccessibleName("kylin-calculator_BasicButton_btnYSquareX");

    btnSin = new BasicButton(this);
    btnSin->setAccessibleName("kylin-calculator_BasicButton_btnSin");
    btnCos = new BasicButton(this);
    btnCos->setAccessibleName("kylin-calculator_BasicButton_btnCos");
    btnTan = new BasicButton(this);
    btnTan->setAccessibleName("kylin-calculator_BasicButton_btnTan");
    btnLog = new BasicButton(this);
    btnLog->setAccessibleName("kylin-calculator_BasicButton_btnLog");

    btnRad = new BasicButton(this);
    btnRad->setAccessibleName("kylin-calculator_BasicButton_btnRad");
    btnPi = new BasicButton(this);
    btnPi->setAccessibleName("kylin-calculator_BasicButton_btnPi");
    btnExp = new BasicButton(this);
    btnExp->setAccessibleName("kylin-calculator_BasicButton_btnExp");
    btnLn = new BasicButton(this);
    btnLn->setAccessibleName("kylin-calculator_BasicButton_btnLn");

    m_picturetowhite = new PictureToWhite();
    m_picturetowhite->setObjectName("kylin-calculator_PictureToWhite_m_picturetowhite");

    // 设置按钮的显示文本
    // ！！！！！！减号不是ascII码 减号不是ascII码 减号不是ascII码 减号不是ascII码 减号不是ascII码
    QString btnList = "C,÷,×,B,ｰ,+,=,%,.";
    // ！！！！！！减号不是ascII码 减号不是ascII码 减号不是ascII码 减号不是ascII码 减号不是ascII码
    QStringList btnNameList = btnList.split(",");
    int index = 0;

    btnClear->setToolTip(tr("Clear"));
    btnClear->setText(btnNameList[index++]);
    btnDiv->setText(btnNameList[index++]);
    btnMulti->setText(btnNameList[index++]);
    btnDelete->setText(btnNameList[index++]);
    btnSub->setText(btnNameList[index++]);
    btnAdd->setText(btnNameList[index++]);
    btnEqual->setText(btnNameList[index++]);
    btnPer->setText(btnNameList[index++]);
    btnPoint->setText(btnNameList[index++]);

    btnClear->setIcon(QIcon(":/image/newIcon/scientific/btnClear.svg"));
    btnDiv->setIcon(QIcon(":/image/newIcon/scientific/btnDiv.svg"));
    btnMulti->setIcon(QIcon(":/image/newIcon/scientific/btnMulti.svg"));
    btnDelete->setIcon(QIcon(":/image/newIcon/scientific/btnDelete.svg"));
    btnSub->setIcon(QIcon(":/image/newIcon/scientific/btnSub.svg"));
    btnAdd->setIcon(QIcon(":/image/newIcon/scientific/btnAdd.svg"));
    btnEqual->setIcon(QIcon(":/image/newIcon/scientific/btnEqual.svg"));
    btnPer->setIcon(QIcon(":/image/newIcon/scientific/btnPer.svg"));
    btnPoint->setIcon(QIcon(":/image/newIcon/scientific/btnPoint.svg"));

    btnClear->setIconSize(QSize(106, 62));
    btnDiv->setIconSize(QSize(106, 62));
    btnMulti->setIconSize(QSize(106, 62));
    // 删除键按钮大小调整
    // btnDelete->setIconSize(QSize(85,  50));
    btnDelete->setIconSize(QSize(106, 62));
    btnSub->setIconSize(QSize(106, 62));
    btnAdd->setIconSize(QSize(106, 62));
    btnEqual->setIconSize(QSize(106, 62));
    btnPer->setIconSize(QSize(106, 62));
    btnPoint->setIconSize(QSize(106, 62));

    btnClear->setProperty("useButtonPalette", true);
    btnDiv->setProperty("useButtonPalette", true);
    btnMulti->setProperty("useButtonPalette", true);
    btnDelete->setProperty("useButtonPalette", true);
    btnSub->setProperty("useButtonPalette", true);
    btnAdd->setProperty("useButtonPalette", true);
    btnPer->setProperty("useButtonPalette", true);
    btnPoint->setProperty("useButtonPalette", true);


    btnList = "2nd,ANS,(,),1/x,x²,x³,y^x,x!,√,³√,x√y,sin,cos,tan,log,rad,PI,E,ln";
    btnNameList = btnList.split(",");
    index = 0;

    btnInd->setText(btnNameList[index++]);
    btnUndo->setText(btnNameList[index++]);
    btnBracketLeft->setText(btnNameList[index++]);
    btnBracketRight->setText(btnNameList[index++]);

    btnReci->setText(btnNameList[index++]);
    btnXPower2->setText(btnNameList[index++]);
    btnXPower3->setText(btnNameList[index++]);
    btnYPowerX->setText(btnNameList[index++]);

    btnFac->setText(btnNameList[index++]);
    btnXSquare2->setText(btnNameList[index++]);
    btnXSquare3->setText(btnNameList[index++]);
    btnYSquareX->setText(btnNameList[index++]);

    btnSin->setText(btnNameList[index++]);
    btnCos->setText(btnNameList[index++]);
    btnTan->setText(btnNameList[index++]);
    btnLog->setText(btnNameList[index++]);

    btnRad->setText(btnNameList[index++]);
    btnPi->setText(btnNameList[index++]);
    btnExp->setText(btnNameList[index++]);
    btnLn->setText(btnNameList[index++]);

    btnInd->setToolTip(tr("Change some keys to interleaving functions") );
    btnUndo->setToolTip(tr("/") );

    btnReci->setToolTip(tr("Calculate the reciprocal of the displayed value") );
    btnXPower2->setToolTip(tr("Square the displayed value") );
    btnXPower3->setToolTip(tr("Calculate the cubic value of the displayed value") );
    btnYPowerX->setToolTip(tr("Calculate the displayed value power of the next input value") );

    btnFac->setToolTip(tr("Calculate the factorial of the displayed value") );
    btnXSquare2->setToolTip(tr("Square root of the displayed square root") );
    btnXSquare3->setToolTip(tr("Cubic representation of the displayed value") );
    btnYSquareX->setToolTip(tr(" Calculate the displayed value to the y-th power") );

    btnSin->setToolTip(tr("Calculate the sine value of the displayed value") );
    btnCos->setToolTip(tr("Calculate the cosine value of the displayed value") );
    btnTan->setToolTip(tr("Calculate the tangent value of the displayed value") );
    btnLog->setToolTip(tr("Calculate the index value based on the displayed value") );

    btnRad->setToolTip(tr("Switch between degrees and arcs (click to switch)") );
    btnPi->setToolTip(tr("Input pi (3.141596...)") );
    btnExp->setToolTip(tr("Enter e (the value of e is e ≈ 2.71828 18284 59...)") );
    btnLn->setToolTip(tr("Calculate the natural logarithm of the displayed value") );

    QString btnIconStr = "btn2nd,btnAns,btnLbra,btnRbra,btnReci,btnXPower2,btnXPower3,btnYPowerX,btnFac,btnXSquare2,"
                         "btnXSquare3,btnYSquareX,btnSin,btnCos,btnTan,btnLog,btnRad,btnPi,btnE,btnLn";
    QStringList btnIconList = btnIconStr.split(",");
    int iconIdx = 0;

    btnInd->setIcon(QIcon(":/image/newIcon/scientific/" + btnIconList[iconIdx++] + ".svg"));
    btnUndo->setIcon(QIcon(":/image/newIcon/scientific/" + btnIconList[iconIdx++] + ".svg"));
    btnBracketLeft->setIcon(QIcon(":/image/newIcon/scientific/" + btnIconList[iconIdx++] + ".svg"));
    btnBracketRight->setIcon(QIcon(":/image/newIcon/scientific/" + btnIconList[iconIdx++] + ".svg"));

    btnReci->setIcon(QIcon(":/image/newIcon/scientific/" + btnIconList[iconIdx++] + ".svg"));
    btnXPower2->setIcon(QIcon(":/image/newIcon/scientific/" + btnIconList[iconIdx++] + ".svg"));
    btnXPower3->setIcon(QIcon(":/image/newIcon/scientific/" + btnIconList[iconIdx++] + ".svg"));
    btnYPowerX->setIcon(QIcon(":/image/newIcon/scientific/" + btnIconList[iconIdx++] + ".svg"));

    btnFac->setIcon(QIcon(":/image/newIcon/scientific/" + btnIconList[iconIdx++] + ".svg"));
    btnXSquare2->setIcon(QIcon(":/image/newIcon/scientific/" + btnIconList[iconIdx++] + ".svg"));
    btnXSquare3->setIcon(QIcon(":/image/newIcon/scientific/" + btnIconList[iconIdx++] + ".svg"));
    btnYSquareX->setIcon(QIcon(":/image/newIcon/scientific/" + btnIconList[iconIdx++] + ".svg"));

    btnSin->setIcon(QIcon(":/image/newIcon/scientific/" + btnIconList[iconIdx++] + ".svg"));
    btnCos->setIcon(QIcon(":/image/newIcon/scientific/" + btnIconList[iconIdx++] + ".svg"));
    btnTan->setIcon(QIcon(":/image/newIcon/scientific/" + btnIconList[iconIdx++] + ".svg"));
    btnLog->setIcon(QIcon(":/image/newIcon/scientific/" + btnIconList[iconIdx++] + ".svg"));

    btnRad->setIcon(QIcon(":/image/newIcon/scientific/" + btnIconList[iconIdx++] + ".svg"));
    btnPi->setIcon(QIcon(":/image/newIcon/scientific/" + btnIconList[iconIdx++] + ".svg"));
    btnExp->setIcon(QIcon(":/image/newIcon/scientific/" + btnIconList[iconIdx++] + ".svg"));
    btnLn->setIcon(QIcon(":/image/newIcon/scientific/" + btnIconList[iconIdx++] + ".svg"));

    btnInd->setIconSize(QSize(106, 62));
    btnUndo->setIconSize(QSize(106, 62));
    btnBracketLeft->setIconSize(QSize(106, 62));
    btnBracketRight->setIconSize(QSize(106, 62));

    btnReci->setIconSize(QSize(106, 62));
    btnXPower2->setIconSize(QSize(106, 62));
    btnXPower3->setIconSize(QSize(106, 62));
    btnYPowerX->setIconSize(QSize(106, 62));

    btnFac->setIconSize(QSize(106, 62));
    btnXSquare2->setIconSize(QSize(106, 62));
    btnXSquare3->setIconSize(QSize(106, 62));
    btnYSquareX->setIconSize(QSize(106, 62));

    btnSin->setIconSize(QSize(106, 62));
    btnCos->setIconSize(QSize(106, 62));
    btnTan->setIconSize(QSize(106, 62));
    btnLog->setIconSize(QSize(106, 62));

    btnRad->setIconSize(QSize(106, 62));
    btnPi->setIconSize(QSize(106, 62));
    btnExp->setIconSize(QSize(106, 62));
    btnLn->setIconSize(QSize(106, 62));
    // btnRad->setMinimumSize(106, 62);

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

    btnInd->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnUndo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnBracketLeft->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnBracketRight->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    btnReci->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnXPower2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnXPower3->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnYPowerX->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    btnFac->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnXSquare2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnXSquare3->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnYSquareX->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    btnSin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnCos->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnTan->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnLog->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    btnRad->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnPi->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnExp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnLn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    btnInd->setProperty("useButtonPalette", true);
    btnUndo->setProperty("useButtonPalette", true);
    btnBracketLeft->setProperty("useButtonPalette", true);
    btnBracketRight->setProperty("useButtonPalette", true);
    btnReci->setProperty("useButtonPalette", true);
    btnXPower2->setProperty("useButtonPalette", true);
    btnXPower3->setProperty("useButtonPalette", true);
    btnYPowerX->setProperty("useButtonPalette", true);
    btnFac->setProperty("useButtonPalette", true);
    btnXSquare2->setProperty("useButtonPalette", true);
    btnXSquare3->setProperty("useButtonPalette", true);
    btnYSquareX->setProperty("useButtonPalette", true);
    btnSin->setProperty("useButtonPalette", true);
    btnCos->setProperty("useButtonPalette", true);
    btnTan->setProperty("useButtonPalette", true);
    btnLog->setProperty("useButtonPalette", true);
    btnRad->setProperty("useButtonPalette", true);
    btnPi->setProperty("useButtonPalette", true);
    btnExp->setProperty("useButtonPalette", true);
    btnLn->setProperty("useButtonPalette", true);

    btnEqual->setProperty("isImportant", true);

    // 将科学计算器按钮进行网格布局
    QGridLayout *btnSciLayout = new QGridLayout(this);
    btnSciLayout->addWidget(btnInd, 0, 0, 1, 1);
    btnSciLayout->addWidget(btnUndo, 0, 1, 1, 1);
    btnSciLayout->addWidget(btnBracketLeft, 0, 2, 1, 1);
    btnSciLayout->addWidget(btnBracketRight, 0, 3, 1, 1);

    btnSciLayout->addWidget(btnReci, 1, 0, 1, 1);
    btnSciLayout->addWidget(btnXPower2, 1, 1, 1, 1);
    btnSciLayout->addWidget(btnXPower3, 1, 2, 1, 1);
    btnSciLayout->addWidget(btnYPowerX, 1, 3, 1, 1);

    btnSciLayout->addWidget(btnFac, 2, 0, 1, 1);
    btnSciLayout->addWidget(btnXSquare2, 2, 1, 1, 1);
    btnSciLayout->addWidget(btnXSquare3, 2, 2, 1, 1);
    btnSciLayout->addWidget(btnYSquareX, 2, 3, 1, 1);

    btnSciLayout->addWidget(btnSin, 3, 0, 1, 1);
    btnSciLayout->addWidget(btnCos, 3, 1, 1, 1);
    btnSciLayout->addWidget(btnTan, 3, 2, 1, 1);
    btnSciLayout->addWidget(btnLog, 3, 3, 1, 1);

    btnSciLayout->addWidget(btnRad, 4, 0, 1, 1);
    btnSciLayout->addWidget(btnPi, 4, 1, 1, 1);
    btnSciLayout->addWidget(btnExp, 4, 2, 1, 1);
    btnSciLayout->addWidget(btnLn, 4, 3, 1, 1);

    // 设置间距和背景样式
    btnSciLayout->setSpacing(2);
    btnSciLayout->setMargin(1);

    // 将按钮进行网格布局
    QGridLayout *btnLayout = new QGridLayout(this);
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

    QHBoxLayout *btnAllLayout = new QHBoxLayout(this);

    QWidget *btnLWid = new QWidget(this);
    btnLWid->setObjectName("kylin-calculator_QWidget_btnLWid");
    QWidget *btnRWid = new QWidget(this);
    btnRWid->setObjectName("kylin-calculator_QWidget_btnRWid");

    btnLWid->setLayout(btnSciLayout);
    btnRWid->setLayout(btnLayout);

    btnAllLayout->addWidget(btnLWid);
    btnAllLayout->addWidget(btnRWid);

    btnAllLayout->setSpacing(0);
    btnAllLayout->setMargin(0);
    btnAllLayout->setContentsMargins(4, 0, 4, 4);

    // this->setFixedHeight(320);
    this->setLayout(btnAllLayout);

    connect(btnInd, &BasicButton::clicked, this, &ScientificModel::changeBtnSinDisplay);
    connect(btnRad, &BasicButton::clicked, this, &ScientificModel::changeBtnRadDisplay);
}

void ScientificModel::createIntelModel()
{
    qDebug() << "Info : create intel scientific mode ui...";
    // 计算器按钮
    // 布局按钮
    for (int i = 0; i < 10; i++) {
        btnNum[i] = new BasicButton(this);
        btnNum[i]->setText(QString::number(i));
        btnNum[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        btnNum[i]->setIcon(QIcon(":/image/intelScientific/" + QString::number(i) + ".svg"));
        btnNum[i]->setAccessibleName(QString("kylin_calculator_BasicButton_btnNum%1").arg(i));
    }

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

    btnInd = new BasicButton(this);
    btnInd->setAccessibleName("kylin-calculator_BasicButton_btnInd");
    btnUndo = new BasicButton(this);
    btnUndo->setAccessibleName("kylin-calculator_BasicButton_btnUndo");
    btnBracketLeft = new BasicButton(this);
    btnBracketLeft->setAccessibleName("kylin-calculator_BasicButton_btnBracketLeft");
    btnBracketRight = new BasicButton(this);
    btnBracketRight->setAccessibleName("kylin-calculator_BasicButton_btnBracketRight");

    btnReci = new BasicButton(this);
    btnReci->setAccessibleName("kylin-calculator_BasicButton_btnReci");
    btnXPower2 = new BasicButton(this);
    btnXPower2->setAccessibleName("kylin-calculator_BasicButton_btnXPower2");
    btnXPower3 = new BasicButton(this);
    btnXPower3->setAccessibleName("kylin-calculator_BasicButton_btnXPower3");
    btnYPowerX = new BasicButton(this);
    btnYPowerX->setAccessibleName("kylin-calculator_BasicButton_btnYPowerX");

    btnFac = new BasicButton(this);
    btnFac->setAccessibleName("kylin-calculator_BasicButton_btnFac");
    btnXSquare2 = new BasicButton(this);
    btnXSquare2->setAccessibleName("kylin-calculator_BasicButton_btnXSquare2");
    btnXSquare3 = new BasicButton(this);
    btnXSquare3->setAccessibleName("kylin-calculator_BasicButton_btnXSquare3");
    btnYSquareX = new BasicButton(this);
    btnYSquareX->setAccessibleName("kylin-calculator_BasicButton_btnYSquareX");

    btnSin = new BasicButton(this);
    btnSin->setAccessibleName("kylin-calculator_BasicButton_btnSin");
    btnCos = new BasicButton(this);
    btnCos->setAccessibleName("kylin-calculator_BasicButton_btnCos");
    btnTan = new BasicButton(this);
    btnTan->setAccessibleName("kylin-calculator_BasicButton_btnTan");
    btnLog = new BasicButton(this);
    btnLog->setAccessibleName("kylin-calculator_BasicButton_btnLog");

    btnRad = new BasicButton(this);
    btnRad->setAccessibleName("kylin-calculator_BasicButton_btnRad");
    btnPi = new BasicButton(this);
    btnPi->setAccessibleName("kylin-calculator_BasicButton_btnPi");
    btnExp = new BasicButton(this);
    btnExp->setAccessibleName("kylin-calculator_BasicButton_btnExp");
    btnLn = new BasicButton(this);
    btnLn->setAccessibleName("kylin-calculator_BasicButton_btnLn");

    // 设置按钮的显示文本
    // ！！！！！！减号不是ascII码 减号不是ascII码 减号不是ascII码 减号不是ascII码 减号不是ascII码
    QString btnList = "C,÷,×,B,ｰ,+,=,%,.";
    // ！！！！！！减号不是ascII码 减号不是ascII码 减号不是ascII码 减号不是ascII码 减号不是ascII码
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

    btnClear->setIcon(QIcon(":/image/intelScientific/btnClear.svg"));
    btnDiv->setIcon(QIcon(":/image/intelScientific/btnDiv.svg"));
    btnMulti->setIcon(QIcon(":/image/intelScientific/btnMulti.svg"));
    btnDelete->setIcon(QIcon(":/image/intelScientific/btnDelete.svg"));
    btnSub->setIcon(QIcon(":/image/intelScientific/btnSub.svg"));
    btnAdd->setIcon(QIcon(":/image/intelScientific/btnAdd.svg"));
    btnEqual->setIcon(QIcon(":/image/intelScientific/btnEqual.svg"));
    btnPer->setIcon(QIcon(":/image/intelScientific/btnPer.svg"));
    btnPoint->setIcon(QIcon(":/image/intelScientific/btnPoint.svg"));

    btnList = "2nd,ANS,(,),1/x,x²,x³,y^x,x!,√,³√,x√y,sin,cos,tan,log,rad,PI,E,ln";
    btnNameList = btnList.split(",");
    index = 0;

    btnInd->setText(btnNameList[index++]);
    btnUndo->setText(btnNameList[index++]);
    btnBracketLeft->setText(btnNameList[index++]);
    btnBracketRight->setText(btnNameList[index++]);

    btnReci->setText(btnNameList[index++]);
    btnXPower2->setText(btnNameList[index++]);
    btnXPower3->setText(btnNameList[index++]);
    btnYPowerX->setText(btnNameList[index++]);

    btnFac->setText(btnNameList[index++]);
    btnXSquare2->setText(btnNameList[index++]);
    btnXSquare3->setText(btnNameList[index++]);
    btnYSquareX->setText(btnNameList[index++]);

    btnSin->setText(btnNameList[index++]);
    btnCos->setText(btnNameList[index++]);
    btnTan->setText(btnNameList[index++]);
    btnLog->setText(btnNameList[index++]);

    btnRad->setText(btnNameList[index++]);
    btnPi->setText(btnNameList[index++]);
    btnExp->setText(btnNameList[index++]);
    btnLn->setText(btnNameList[index++]);

    QString btnIconStr = "btn2nd,btnAns,btnLbra,btnRbra,btnReci,btnXPower2,btnXPower3,btnYPowerX,btnFac,btnXSquare2,"
                         "btnXSquare3,btnYSquareX,btnSin,btnCos,btnTan,btnLog,btnRad,btnPi,btnE,btnLn";
    QStringList btnIconList = btnIconStr.split(",");
    int iconIdx = 0;

    btnInd->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));
    btnUndo->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));
    btnBracketLeft->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));
    btnBracketRight->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));

    btnReci->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));
    btnXPower2->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));
    btnXPower3->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));
    btnYPowerX->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));

    btnFac->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));
    btnXSquare2->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));
    btnXSquare3->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));
    btnYSquareX->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));

    btnSin->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));
    btnCos->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));
    btnTan->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));
    btnLog->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));

    btnRad->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));
    btnPi->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));
    btnExp->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));
    btnLn->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));

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

    btnInd->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnUndo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnBracketLeft->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnBracketRight->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    btnReci->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnXPower2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnXPower3->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnYPowerX->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    btnFac->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnXSquare2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnXSquare3->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnYSquareX->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    btnSin->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnCos->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnTan->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnLog->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    btnRad->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnPi->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnExp->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnLn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    changeBtnIconSize();

    // 将科学计算器按钮进行网格布局
    QGridLayout *btnSciLayout = new QGridLayout(this);
    btnSciLayout->addWidget(btnInd, 0, 0, 1, 1);
    btnSciLayout->addWidget(btnUndo, 0, 1, 1, 1);
    btnSciLayout->addWidget(btnBracketLeft, 0, 2, 1, 1);
    btnSciLayout->addWidget(btnBracketRight, 0, 3, 1, 1);

    btnSciLayout->addWidget(btnReci, 1, 0, 1, 1);
    btnSciLayout->addWidget(btnXPower2, 1, 1, 1, 1);
    btnSciLayout->addWidget(btnXPower3, 1, 2, 1, 1);
    btnSciLayout->addWidget(btnYPowerX, 1, 3, 1, 1);

    btnSciLayout->addWidget(btnFac, 2, 0, 1, 1);
    btnSciLayout->addWidget(btnXSquare2, 2, 1, 1, 1);
    btnSciLayout->addWidget(btnXSquare3, 2, 2, 1, 1);
    btnSciLayout->addWidget(btnYSquareX, 2, 3, 1, 1);

    btnSciLayout->addWidget(btnSin, 3, 0, 1, 1);
    btnSciLayout->addWidget(btnCos, 3, 1, 1, 1);
    btnSciLayout->addWidget(btnTan, 3, 2, 1, 1);
    btnSciLayout->addWidget(btnLog, 3, 3, 1, 1);

    btnSciLayout->addWidget(btnRad, 4, 0, 1, 1);
    btnSciLayout->addWidget(btnPi, 4, 1, 1, 1);
    btnSciLayout->addWidget(btnExp, 4, 2, 1, 1);
    btnSciLayout->addWidget(btnLn, 4, 3, 1, 1);

    // 设置间距和背景样式
    btnSciLayout->setSpacing(2);
    btnSciLayout->setMargin(1);

    // 将按钮进行网格布局
    QGridLayout *btnLayout = new QGridLayout(this);
    btnLayout->addWidget(btnClear, 0, 0, 1, 1);
    btnLayout->addWidget(btnPer, 0, 1, 1, 1);
    btnLayout->addWidget(btnDelete, 0, 2, 1, 1);
    btnLayout->addWidget(btnAdd, 0, 3, 1, 1);
    btnLayout->addWidget(btnSub, 1, 3, 1, 1);
    btnLayout->addWidget(btnMulti, 2, 3, 1, 1);
    btnLayout->addWidget(btnDiv, 3, 3, 1, 1);
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

    QHBoxLayout *btnAllLayout = new QHBoxLayout(this);

    QWidget *btnLWid = new QWidget(this);
    btnLWid->setObjectName("kylin-calculator_QWidget_btnLWid");
    QWidget *btnRWid = new QWidget(this);
    btnRWid->setObjectName("kylin-calculator_QWidget_btnRWid");

    btnLWid->setLayout(btnSciLayout);
    btnRWid->setLayout(btnLayout);

    btnAllLayout->addWidget(btnLWid);
    btnAllLayout->addWidget(btnRWid);

    btnAllLayout->setSpacing(0);
    btnAllLayout->setMargin(0);

    // this->setFixedHeight(320);
    this->setLayout(btnAllLayout);

    connect(btnInd, &BasicButton::clicked, this, &ScientificModel::changeBtnSinDisplay);
    connect(btnRad, &BasicButton::clicked, this, &ScientificModel::changeBtnRadDisplay);
}

void ScientificModel::createIntelStyle()
{
    QString btnStyle;
    QString btnOpStyle;
    QString btnEqualStyle;

    if (WidgetStyle::themeColor == WidgetStyle::ThemeColor::LIGHT) {
        /* 填充背景色 */
        QPalette pal = this->palette();
        pal.setColor(QPalette::Background, QColor("#F6F6F6"));
        this->setAutoFillBackground(true);
        this->setPalette(pal);

        for (int i = 0; i < 10; i++) {
            btnNum[i]->setIcon(QIcon(":/image/intelScientific/" + QString::number(i) + ".svg"));
        }

        btnClear->setIcon(QIcon(":/image/intelScientific/btnClear.svg"));
        btnDiv->setIcon(QIcon(":/image/intelScientific/btnDiv.svg"));
        btnMulti->setIcon(QIcon(":/image/intelScientific/btnMulti.svg"));
        btnDelete->setIcon(QIcon(":/image/intelScientific/btnDelete.svg"));
        btnSub->setIcon(QIcon(":/image/intelScientific/btnSub.svg"));
        btnAdd->setIcon(QIcon(":/image/intelScientific/btnAdd.svg"));
        btnEqual->setIcon(QIcon(":/image/intelScientific/btnEqual.svg"));
        btnPer->setIcon(QIcon(":/image/intelScientific/btnPer.svg"));
        btnPoint->setIcon(QIcon(":/image/intelScientific/btnPoint.svg"));

        QString btnIconStr = "btn2nd,btnAns,btnLbra,btnRbra,btnReci,btnXPower2,btnXPower3,btnYPowerX,btnFac,"
                             "btnXSquare2,btnXSquare3,btnYSquareX,btnSin,btnCos,btnTan,btnLog,btnRad,btnPi,btnE,btnLn";
        QStringList btnIconList = btnIconStr.split(",");
        int iconIdx = 0;

        btnInd->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));
        btnUndo->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));
        btnBracketLeft->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));
        btnBracketRight->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));

        btnReci->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));
        btnXPower2->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));
        btnXPower3->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));
        btnYPowerX->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));

        btnFac->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));
        btnXSquare2->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));
        btnXSquare3->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));
        btnYSquareX->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));

        btnSin->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));
        btnCos->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));
        btnTan->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));
        btnLog->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));

        btnRad->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));
        btnPi->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));
        btnExp->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));
        btnLn->setIcon(QIcon(":/image/intelScientific/" + btnIconList[iconIdx++] + ".svg"));

        /* 先保持与普通按钮颜色一致 , 添加毛玻璃效果后 , 在调整颜色 */
        btnStyle = "QPushButton{"
                   "background-color:#FFFFFF;border-radius:0px;"
                   "font-size:42px;font-family:HelveticaNeueLTPro-UltLt;color:#FFFFFF;"
                   "}"
                   "QPushButton:pressed{background-color:#F6F6F6;}";

        btnOpStyle = "QPushButton{"
                     //"background-color:#F6F6F6;border-radius:0px;"
                     "background-color:#FFFFFF;border-radius:0px;"
                     "font-size:23px;font-family:HelveticaNeueLTPro-UltLt;color:#FFFFFF;"
                     "}"
                     //"QPushButton:pressed{background-color:#FFFFFF;}";
                     "QPushButton:pressed{background-color:#F6F6F6;}";

        btnEqualStyle = "QPushButton{"
                        "background-color:#FB7054;border-radius:0px;"
                        "font-size:48px;font-family:HelveticaNeueLTPro-UltLt;color:#FFFFFF;"
                        "}"
                        "QPushButton:pressed{background-color:#EA5E42;}";



        btnBracketLeft->setIcon(QIcon(":/image/intelScientific/btnLbra.svg"));
        btnBracketRight->setIcon(QIcon(":/image/intelScientific/btnRbra.svg"));
        btnBracketLeft->show();
        btnBracketRight->show();
    } else if (WidgetStyle::themeColor == WidgetStyle::ThemeColor::DARK) {
        /* 填充背景色 */
        QPalette pal = this->palette();
        pal.setColor(QPalette::Background, Qt::black);
        this->setAutoFillBackground(true);
        this->setPalette(pal);

        for (int i = 0; i < 10; i++) {
            btnNum[i]->setIcon(QIcon(":/image/intelScientificDark/" + QString::number(i) + ".svg"));
        }

        btnClear->setIcon(QIcon(":/image/intelScientificDark/btnClear.svg"));
        btnDiv->setIcon(QIcon(":/image/intelScientificDark/btnDiv.svg"));
        btnMulti->setIcon(QIcon(":/image/intelScientificDark/btnMulti.svg"));
        btnDelete->setIcon(QIcon(":/image/intelScientificDark/btnDelete.svg"));
        btnSub->setIcon(QIcon(":/image/intelScientificDark/btnSub.svg"));
        btnAdd->setIcon(QIcon(":/image/intelScientificDark/btnAdd.svg"));
        btnEqual->setIcon(QIcon(":/image/intelScientificDark/btnEqual.svg"));
        btnPer->setIcon(QIcon(":/image/intelScientificDark/btnPer.svg"));
        btnPoint->setIcon(QIcon(":/image/intelScientificDark/btnPoint.svg"));

        QString btnIconStr = "btn2nd,btnAns,btnLbra,btnRbra,btnReci,btnXPower2,btnXPower3,btnYPowerX,btnFac,"
                             "btnXSquare2,btnXSquare3,btnYSquareX,btnSin,btnCos,btnTan,btnLog,btnRad,btnPi,btnE,btnLn";
        QStringList btnIconList = btnIconStr.split(",");
        int iconIdx = 0;


        btnInd->setIcon(QIcon(":/image/intelScientificDark/" + btnIconList[iconIdx++] + ".svg"));
        btnUndo->setIcon(QIcon(":/image/intelScientificDark/" + btnIconList[iconIdx++] + ".svg"));
        btnBracketLeft->setIcon(QIcon(":/image/intelScientificDark/" + btnIconList[iconIdx++] + ".svg"));
        btnBracketRight->setIcon(QIcon(":/image/intelScientificDark/" + btnIconList[iconIdx++] + ".svg"));

        btnReci->setIcon(QIcon(":/image/intelScientificDark/" + btnIconList[iconIdx++] + ".svg"));
        btnXPower2->setIcon(QIcon(":/image/intelScientificDark/" + btnIconList[iconIdx++] + ".svg"));
        btnXPower3->setIcon(QIcon(":/image/intelScientificDark/" + btnIconList[iconIdx++] + ".svg"));
        btnYPowerX->setIcon(QIcon(":/image/intelScientificDark/" + btnIconList[iconIdx++] + ".svg"));

        btnFac->setIcon(QIcon(":/image/intelScientificDark/" + btnIconList[iconIdx++] + ".svg"));
        btnXSquare2->setIcon(QIcon(":/image/intelScientificDark/" + btnIconList[iconIdx++] + ".svg"));
        btnXSquare3->setIcon(QIcon(":/image/intelScientificDark/" + btnIconList[iconIdx++] + ".svg"));
        btnYSquareX->setIcon(QIcon(":/image/intelScientificDark/" + btnIconList[iconIdx++] + ".svg"));

        btnSin->setIcon(QIcon(":/image/intelScientificDark/" + btnIconList[iconIdx++] + ".svg"));
        btnCos->setIcon(QIcon(":/image/intelScientificDark/" + btnIconList[iconIdx++] + ".svg"));
        btnTan->setIcon(QIcon(":/image/intelScientificDark/" + btnIconList[iconIdx++] + ".svg"));
        btnLog->setIcon(QIcon(":/image/intelScientificDark/" + btnIconList[iconIdx++] + ".svg"));

        btnRad->setIcon(QIcon(":/image/intelScientificDark/" + btnIconList[iconIdx++] + ".svg"));
        btnPi->setIcon(QIcon(":/image/intelScientificDark/" + btnIconList[iconIdx++] + ".svg"));
        btnExp->setIcon(QIcon(":/image/intelScientificDark/" + btnIconList[iconIdx++] + ".svg"));
        btnLn->setIcon(QIcon(":/image/intelScientificDark/" + btnIconList[iconIdx++] + ".svg"));

        btnStyle = "QPushButton{"
                   "background-color:#27292C;border-radius:0px;"
                   "font-size:42px;font-family:HelveticaNeueLTPro-UltLt;color:#FFFFFF;"
                   "}"
                   "QPushButton:hover{background-color:#474747;}";

        btnOpStyle = "QPushButton{"
                     "background-color:#222E36;border-radius:0px;"
                     "font-size:35px;font-family:HelveticaNeueLTPro-UltLt;color:#FFFFFF;}"
                     "QPushButton:hover{background-color:#474747;}";

        btnEqualStyle = "QPushButton{"
                        "background-color:#E64A19;border-radius:0px;"
                        "font-size:48px;font-family:HelveticaNeueLTPro-UltLt;color:#FFFFFF;"
                        "}"
                        "QPushButton:hover{background-color:#E45E4C;}";
    }


    for (int i = 0; i < 10; i++) {
        btnNum[i]->setStyleSheet(btnStyle);
    }

    /* 设置按钮样式standardModel */
    btnClear->setStyleSheet(btnStyle);

    btnDiv->setStyleSheet(btnStyle);
    btnMulti->setStyleSheet(btnStyle);

    btnDelete->setStyleSheet(btnStyle);

    btnSub->setStyleSheet(btnStyle);
    btnAdd->setStyleSheet(btnStyle);
    btnEqual->setStyleSheet(btnEqualStyle);
    btnPer->setStyleSheet(btnStyle);
    btnPoint->setStyleSheet(btnStyle);

    btnInd->setStyleSheet(btnOpStyle);
    btnUndo->setStyleSheet(btnOpStyle);
    btnBracketLeft->setStyleSheet(btnOpStyle);
    btnBracketRight->setStyleSheet(btnOpStyle);

    btnReci->setStyleSheet(btnOpStyle);
    btnXPower2->setStyleSheet(btnOpStyle);
    btnXPower3->setStyleSheet(btnOpStyle);
    btnYPowerX->setStyleSheet(btnOpStyle);

    btnFac->setStyleSheet(btnOpStyle);
    btnXSquare2->setStyleSheet(btnOpStyle);
    btnXSquare3->setStyleSheet(btnOpStyle);
    btnYSquareX->setStyleSheet(btnOpStyle);

    btnSin->setStyleSheet(btnOpStyle);
    btnCos->setStyleSheet(btnOpStyle);
    btnTan->setStyleSheet(btnOpStyle);
    btnLog->setStyleSheet(btnOpStyle);

    btnRad->setStyleSheet(btnOpStyle);
    btnPi->setStyleSheet(btnOpStyle);
    btnExp->setStyleSheet(btnOpStyle);
    btnLn->setStyleSheet(btnOpStyle);
}

void ScientificModel::changeBtnIconSize()
{
    int iconWidth = 236 / 2.3;
    int iconHeight = 112 / 2.3;

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
    btnInd->setIconSize(QSize(iconWidth, iconHeight));
    btnUndo->setIconSize(QSize(iconWidth, iconHeight));
    btnBracketLeft->setIconSize(QSize(iconWidth, iconHeight));
    btnBracketRight->setIconSize(QSize(iconWidth, iconHeight));
    btnReci->setIconSize(QSize(iconWidth, iconHeight));
    btnXPower2->setIconSize(QSize(iconWidth, iconHeight));
    btnXPower3->setIconSize(QSize(iconWidth, iconHeight));
    btnYPowerX->setIconSize(QSize(iconWidth, iconHeight));
    btnFac->setIconSize(QSize(iconWidth, iconHeight));
    btnXSquare2->setIconSize(QSize(iconWidth, iconHeight));
    btnXSquare3->setIconSize(QSize(iconWidth, iconHeight));
    btnYSquareX->setIconSize(QSize(iconWidth, iconHeight));
    btnSin->setIconSize(QSize(iconWidth, iconHeight));
    btnCos->setIconSize(QSize(iconWidth, iconHeight));
    btnTan->setIconSize(QSize(iconWidth, iconHeight));
    btnLog->setIconSize(QSize(iconWidth, iconHeight));
    btnRad->setIconSize(QSize(iconWidth, iconHeight));
    btnPi->setIconSize(QSize(iconWidth, iconHeight));
    btnExp->setIconSize(QSize(iconWidth, iconHeight));
    btnLn->setIconSize(QSize(iconWidth, iconHeight));
}

// 格式化输入表达式
QString ScientificModel::sciFormatInput(QString text)
{
    //    btnList = "2nd,Ans,(,),1/x,x²,x³,y^x,x!,√x,³√x,x√y,sin,cos,tan,log,Rad,π,e,ln";
    if (text == "(")
        return "(";
    else if (text == ")")
        return ")";
    else if (text == "1/x")
        return "^(-1)";
    else if (text == "x²")
        return "^2";
    else if (text == "x³")
        return "^3";
    else if (text == "y^x")
        return "^(";
    else if (text == "x!")
        return "!";
    else if (text == "√x")
        return "√(";
    else if (text == "³√x")
        return "^(1÷3)";
    else if (text == "x√y")
        return "^(1÷";
    else if (text == "π")
        return "π";
    else if (text == "e")
        return "e";
    else
        return text + "(";
}

// 切换角度弧度显示
void ScientificModel::changeBtnSinDisplay()
{
    // 切换文字显示
    if (btnSin->text() == "sin") {
        btnSin->setText("asin");
        btnCos->setText("acos");
        btnTan->setText("atan");
        btnSin->setToolTip(tr("Calculate the arcsine value of the displayed value") );
        btnCos->setToolTip(tr("Calculate the arccosine value of the displayed value") );
        btnTan->setToolTip(tr("Calculate the arctangent value of the displayed value") );
    } else {
        btnSin->setText("sin");
        btnCos->setText("cos");
        btnTan->setText("tan");
        btnSin->setToolTip(tr("Calculate the sine value of the displayed value") );
        btnCos->setToolTip(tr("Calculate the cosine value of the displayed value") );
        btnTan->setToolTip(tr("Calculate the tangent value of the displayed value") );
    }

    // 更新三角函数图片
    updateBtnSinDisplay();
}

// 更新三角函数图片
void ScientificModel::updateBtnSinDisplay()
{
    // 切换图片显示
    if (btnSin->text() == "sin") {

        if (WidgetStyle::themeColor == WidgetStyle::ThemeColor::LIGHT) {
            if (DataWarehouse::getInstance()->platform == QString("intel")) {
                btnSin->setIcon(QIcon(":/image/intelScientific/btnSin.svg"));
                btnCos->setIcon(QIcon(":/image/intelScientific/btnCos.svg"));
                btnTan->setIcon(QIcon(":/image/intelScientific/btnTan.svg"));
            } else {
                btnSin->setIcon(QIcon(":/image/newIcon/scientific/btnSin.svg"));
                btnCos->setIcon(QIcon(":/image/newIcon/scientific/btnCos.svg"));
                btnTan->setIcon(QIcon(":/image/newIcon/scientific/btnTan.svg"));
            }

        } else if (WidgetStyle::themeColor == WidgetStyle::ThemeColor::DARK) {
            if (DataWarehouse::getInstance()->platform == QString("intel")) {
                btnSin->setIcon(QIcon(":/image/intelScientificDark/btnSin.svg"));
                btnCos->setIcon(QIcon(":/image/intelScientificDark/btnCos.svg"));
                btnTan->setIcon(QIcon(":/image/intelScientificDark/btnTan.svg"));
            } else {
                btnSin->setIcon(
                    m_picturetowhite->drawSymbolicColoredPixmap(QPixmap(":/image/newIcon/scientific/btnSin.svg")));
                btnCos->setIcon(
                    m_picturetowhite->drawSymbolicColoredPixmap(QPixmap(":/image/newIcon/scientific/btnCos.svg")));
                btnTan->setIcon(
                    m_picturetowhite->drawSymbolicColoredPixmap(QPixmap(":/image/newIcon/scientific/btnTan.svg")));
            }
        }
    } else {

        if (WidgetStyle::themeColor == WidgetStyle::ThemeColor::LIGHT) {
            if (DataWarehouse::getInstance()->platform == QString("intel")) {
                btnSin->setIcon(QIcon(":/image/intelScientific/btnAsin.svg"));
                btnCos->setIcon(QIcon(":/image/intelScientific/btnAcos.svg"));
                btnTan->setIcon(QIcon(":/image/intelScientific/btnAtan.svg"));
            } else {
                btnSin->setIcon(QIcon(":/image/newIcon/scientific/btnAsin.svg"));
                btnCos->setIcon(QIcon(":/image/newIcon/scientific/btnAcos.svg"));
                btnTan->setIcon(QIcon(":/image/newIcon/scientific/btnAtan.svg"));
            }

        } else if (WidgetStyle::themeColor == WidgetStyle::ThemeColor::DARK) {
            if (DataWarehouse::getInstance()->platform == QString("intel")) {
                btnSin->setIcon(QIcon(":/image/intelScientificDark/btnAsin.svg"));
                btnCos->setIcon(QIcon(":/image/intelScientificDark/btnAcos.svg"));
                btnTan->setIcon(QIcon(":/image/intelScientificDark/btnAtan.svg"));
            } else {
                btnSin->setIcon(
                    m_picturetowhite->drawSymbolicColoredPixmap(QPixmap(":/image/newIcon/scientific/btnAsin.svg")));
                btnCos->setIcon(
                    m_picturetowhite->drawSymbolicColoredPixmap(QPixmap(":/image/newIcon/scientific/btnAcos.svg")));
                btnTan->setIcon(
                    m_picturetowhite->drawSymbolicColoredPixmap(QPixmap(":/image/newIcon/scientific/btnAtan.svg")));
            }
        }
    }
}

// 切换角度弧度显示
void ScientificModel::changeBtnRadDisplay()
{
    // 切换文字显示和图片显示
    if (btnRad->text() == "rad") {
        btnRad->setText("deg");
    } else {
        btnRad->setText("rad");
    }

    // 更新角度弧度图片
    updateBtnRadDisplay();
}

// 更新角度弧度图片
void ScientificModel::updateBtnRadDisplay()
{
    // 切换图片显示
    if (btnRad->text() == "rad") {

        if (WidgetStyle::themeColor == WidgetStyle::ThemeColor::LIGHT) {
            if (DataWarehouse::getInstance()->platform == QString("intel")) {
                btnRad->setIcon(QIcon(":/image/intelScientific/btnRad.svg"));
            } else {
                btnRad->setIcon(QIcon(":/image/newIcon/scientific/btnRad.svg"));
            }

        } else if (WidgetStyle::themeColor == WidgetStyle::ThemeColor::DARK) {
            if (DataWarehouse::getInstance()->platform == QString("intel")) {
                btnRad->setIcon(QIcon(":/image/intelScientificDark/btnRad.svg"));
            } else {
                btnRad->setIcon(
                    m_picturetowhite->drawSymbolicColoredPixmap(QPixmap(":/image/newIcon/scientific/btnRad.svg")));
            }
        }
    } else {

        if (WidgetStyle::themeColor == WidgetStyle::ThemeColor::LIGHT) {
            if (DataWarehouse::getInstance()->platform == QString("intel")) {
                btnRad->setIcon(QIcon(":/image/intelScientific/btnDeg.svg"));
            } else {
                btnRad->setIcon(QIcon(":/image/newIcon/scientific/btnDeg.svg"));
            }

        } else if (WidgetStyle::themeColor == WidgetStyle::ThemeColor::DARK) {
            if (DataWarehouse::getInstance()->platform == QString("intel")) {
                btnRad->setIcon(QIcon(":/image/intelScientificDark/btnDeg.svg"));
            } else {
                btnRad->setIcon(
                    m_picturetowhite->drawSymbolicColoredPixmap(QPixmap(":/image/newIcon/scientific/btnDeg.svg")));
            }
        }
    }
}

void ScientificModel::keyPressEvent(QKeyEvent *event)
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
    case Qt::Key_ParenLeft:
        btnBracketLeft->click();
        break;
    case Qt::Key_ParenRight:
        btnBracketRight->click();
        break;
    case Qt::Key_AsciiCircum:
        btnYPowerX->click();
        break;
    case Qt::Key_Exclam:
        btnFac->click();
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
