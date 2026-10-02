/*
 * Copyright (C) 2018 KylinSoftCo., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/
#include "charswidget.h"

#include "commondef.h"
#include <QDebug>

CharsWidget::CharsWidget(QWidget *parent/* = nullptr*/)
    : QWidget(parent)
{
    this->setAttribute(Qt::WA_TranslucentBackground);//背景透明
    initUI();
}

CharsWidget::~CharsWidget()
{

}

void CharsWidget::initUI()
{
    // line 1
    QChar chLine1[] = {'1','2','3','4','5','6','7','8','9','0'};
    for (int n = 0; n < sizeof(chLine1)/sizeof(QChar); n++) {
        KBButton *charBtn = new KBButton(this);
        charBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_LETTER_L1+(KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH+KEYBOAED_FIXED_DEFAULT_HSPACING)*n,
                               KEYBOARD_FIXED_DEFAULT_TMARGIN,
                               KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH,
                               KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT);
        charBtn->setCharId(chLine1[n]);
        charBtn->updateStyleSheet(KEYBOARD_LETTER_COLOR_NORMAL,KEYBOARD_LETTER_COLOR_PRESSED,KEYBOARD_LETTER_COLOR_PRESSED,
                                    KEYBOARD_LETTER_COLOR_BORDER_NORMAL,KEYBOARD_LETTER_COLOR_BORDER_PRESSED,KEYBOARD_FONT_COLOR_PRESS);
        connect(charBtn, &KBButton::clicked, this, &CharsWidget::onBtnClicked);
        m_mapBtnGeometrys[charBtn] = charBtn->geometry();
    }
    // backspace
    KBButton *backspaceBtn = new KBButton(this);
    backspaceBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_LETTER_L1+(KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH+KEYBOAED_FIXED_DEFAULT_HSPACING)*10,
                           KEYBOARD_FIXED_DEFAULT_TMARGIN,
                           KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH,
                           KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT);
    backspaceBtn->setObjectName("btn_backspace");
    backspaceBtn->updateStyleSheet(KEYBOARD_OTHER_COLOR_NORMAL,KEYBOARD_OTHER_COLOR_PRESSED,
                                   KEYBOARD_OTHER_COLOR_PRESSED,KEYBOARD_OTHER_COLOR_BORDER_NORMAL,
                                   KEYBOARD_OTHER_COLOR_BORDER_PRESSED,KEYBOARD_OTHER_FONT_COLOR_PRESS);
    connect(backspaceBtn, &KBButton::clicked, this, &CharsWidget::onBtnClicked);
    backspaceBtn->setIcon(QIcon(":/images/images/delet.svg"));
    m_mapBtnGeometrys[backspaceBtn] = backspaceBtn->geometry();

    // line 2
    QChar chLine2[] = {'~','/',':',';','(',')','@','"','\''};
    for (int n = 0; n < sizeof(chLine2)/sizeof(QChar); n++) {
        KBButton *charBtn = new KBButton(this);
        charBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_LETTER_L2+(KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH+KEYBOAED_FIXED_DEFAULT_HSPACING)*n,
                               KEYBOARD_FIXED_DEFAULT_TMARGIN+(KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT+KEYBOAED_FIXED_DEFAULT_VSPACING),
                               KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH,
                               KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT);
        charBtn->setCharId(chLine2[n]);
        charBtn->updateStyleSheet(KEYBOARD_LETTER_COLOR_NORMAL,KEYBOARD_LETTER_COLOR_PRESSED,KEYBOARD_LETTER_COLOR_PRESSED,
                                    KEYBOARD_LETTER_COLOR_BORDER_NORMAL,KEYBOARD_LETTER_COLOR_BORDER_PRESSED,KEYBOARD_FONT_COLOR_PRESS);
        connect(charBtn, &KBButton::clicked, this, &CharsWidget::onBtnClicked);
        m_mapBtnGeometrys[charBtn] = charBtn->geometry();
    }
    // enter
    KBButton *enterBtn = new KBButton(this);
    enterBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_LETTER_L2+(KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH+KEYBOAED_FIXED_DEFAULT_HSPACING)*9,
                           KEYBOARD_FIXED_DEFAULT_TMARGIN+(KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT+KEYBOAED_FIXED_DEFAULT_VSPACING),
                           KEYBOARD_FIXED_DEFAULT_ENTERBTN_WIDTH,
                           KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT);
    enterBtn->setObjectName("btn_enter");
    enterBtn->updateStyleSheet(KEYBOARD_OTHER_COLOR_NORMAL,KEYBOARD_OTHER_COLOR_PRESSED,
                                   KEYBOARD_OTHER_COLOR_PRESSED,KEYBOARD_OTHER_COLOR_BORDER_NORMAL,
                                   KEYBOARD_OTHER_COLOR_BORDER_PRESSED,KEYBOARD_OTHER_FONT_COLOR_PRESS);
    connect(enterBtn, &KBButton::clicked, this, &CharsWidget::onBtnClicked);
    enterBtn->setIcon(QIcon(":/images/images/enter.svg"));
    m_mapBtnGeometrys[enterBtn] = enterBtn->geometry();

    // line 3
    QChar chLine3[] = {'-','_','#','%','$','+','^',',','.','!'};
    for (int n = 0; n < sizeof(chLine3)/sizeof(QChar); n++) {
        KBButton *charBtn = new KBButton(this);
        charBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_LETTER_L3+(KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH+KEYBOAED_FIXED_DEFAULT_HSPACING)*(n+1),
                               KEYBOARD_FIXED_DEFAULT_TMARGIN+(KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT+KEYBOAED_FIXED_DEFAULT_VSPACING)*2,
                               KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH,
                               KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT);
        charBtn->setCharId(chLine3[n]);
        charBtn->updateStyleSheet(KEYBOARD_LETTER_COLOR_NORMAL,KEYBOARD_LETTER_COLOR_PRESSED,KEYBOARD_LETTER_COLOR_PRESSED,
                                    KEYBOARD_LETTER_COLOR_BORDER_NORMAL,KEYBOARD_LETTER_COLOR_BORDER_PRESSED,KEYBOARD_FONT_COLOR_PRESS);
        connect(charBtn, &KBButton::clicked, this, &CharsWidget::onBtnClicked);
        m_mapBtnGeometrys[charBtn] = charBtn->geometry();
    }
    // more
    KBButton *moreBtn = new KBButton(this);
    moreBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_LETTER_L3,
                           KEYBOARD_FIXED_DEFAULT_TMARGIN+(KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT+KEYBOAED_FIXED_DEFAULT_VSPACING)*2,
                           KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH,
                           KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT);
    moreBtn->setObjectName("btn_more");
    moreBtn->setText(tr("More"));
    moreBtn->updateStyleSheet(KEYBOARD_OTHER_COLOR_NORMAL,KEYBOARD_OTHER_COLOR_PRESSED,
                                   KEYBOARD_OTHER_COLOR_PRESSED,KEYBOARD_OTHER_COLOR_BORDER_NORMAL,
                                   KEYBOARD_OTHER_COLOR_BORDER_PRESSED,KEYBOARD_OTHER_FONT_COLOR_PRESS);
    connect(moreBtn, &KBButton::clicked, this, &CharsWidget::onBtnClicked);
    m_mapBtnGeometrys[moreBtn] = moreBtn->geometry();

    // line 4
    KBButton *returnBtn = new KBButton(this);
    returnBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_LETTER_L4,
                           KEYBOARD_FIXED_DEFAULT_TMARGIN+(KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT+KEYBOAED_FIXED_DEFAULT_VSPACING)*3,
                           KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH,
                           KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT);
    returnBtn->setObjectName("btn_return");
    returnBtn->setText(tr("ABC"));
    returnBtn->updateStyleSheet(KEYBOARD_OTHER_COLOR_NORMAL,KEYBOARD_OTHER_COLOR_PRESSED,
                                   KEYBOARD_OTHER_COLOR_PRESSED,KEYBOARD_OTHER_COLOR_BORDER_NORMAL,
                                   KEYBOARD_OTHER_COLOR_BORDER_PRESSED,KEYBOARD_OTHER_FONT_COLOR_PRESS);
    connect(returnBtn, &KBButton::clicked, this, &CharsWidget::onBtnClicked);
    m_mapBtnGeometrys[returnBtn] = returnBtn->geometry();

    KBButton *numBtn = new KBButton(this);
    numBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_LETTER_L4+(KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH+KEYBOAED_FIXED_DEFAULT_HSPACING),
                           KEYBOARD_FIXED_DEFAULT_TMARGIN+(KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT+KEYBOAED_FIXED_DEFAULT_VSPACING)*3,
                           KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH,
                           KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT);
    numBtn->setObjectName("btn_number");
    numBtn->setText(tr("123"));
    numBtn->updateStyleSheet(KEYBOARD_OTHER_COLOR_NORMAL,KEYBOARD_OTHER_COLOR_PRESSED,
                                   KEYBOARD_OTHER_COLOR_PRESSED,KEYBOARD_OTHER_COLOR_BORDER_NORMAL,
                                   KEYBOARD_OTHER_COLOR_BORDER_PRESSED,KEYBOARD_OTHER_FONT_COLOR_PRESS);
    connect(numBtn, &KBButton::clicked, this, &CharsWidget::onBtnClicked);
    m_mapBtnGeometrys[numBtn] = numBtn->geometry();

    KBButton *threedBtn = new KBButton(this);
    threedBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_LETTER_L4+(KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH+KEYBOAED_FIXED_DEFAULT_HSPACING)*2,
                           KEYBOARD_FIXED_DEFAULT_TMARGIN+(KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT+KEYBOAED_FIXED_DEFAULT_VSPACING)*3,
                           KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH,
                           KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT);
    threedBtn->setObjectName("btn_threed");
    threedBtn->setCharId(('&'));
    threedBtn->updateStyleSheet(KEYBOARD_LETTER_COLOR_NORMAL,KEYBOARD_LETTER_COLOR_PRESSED,KEYBOARD_LETTER_COLOR_PRESSED,
                                KEYBOARD_LETTER_COLOR_BORDER_NORMAL,KEYBOARD_LETTER_COLOR_BORDER_PRESSED,KEYBOARD_FONT_COLOR_PRESS);
    connect(threedBtn, &KBButton::clicked, this, &CharsWidget::onBtnClicked);
    m_mapBtnGeometrys[threedBtn] = threedBtn->geometry();

    KBButton *spaceBtn = new KBButton(this);
    spaceBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_LETTER_L4+(KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH+KEYBOAED_FIXED_DEFAULT_HSPACING)*3,
                           KEYBOARD_FIXED_DEFAULT_TMARGIN+(KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT+KEYBOAED_FIXED_DEFAULT_VSPACING)*3,
                           KEYBOARD_FIXED_DEFAULT_SPACEBTN_WIDTH,
                           KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT);
    spaceBtn->setObjectName("btn_space");
    spaceBtn->updateStyleSheet(KEYBOARD_LETTER_COLOR_NORMAL,KEYBOARD_LETTER_COLOR_PRESSED,
                                   KEYBOARD_LETTER_COLOR_PRESSED,KEYBOARD_LETTER_COLOR_BORDER_NORMAL,
                                   KEYBOARD_LETTER_COLOR_BORDER_PRESSED,KEYBOARD_FONT_COLOR_PRESS);
    spaceBtn->setIcon(QIcon(":/images/images/space.svg"));
    connect(spaceBtn, &KBButton::clicked, this, &CharsWidget::onBtnClicked);
    m_mapBtnGeometrys[spaceBtn] = spaceBtn->geometry();

    KBButton *whBtn = new KBButton(this);
    whBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_LETTER_L4+(KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH+KEYBOAED_FIXED_DEFAULT_HSPACING)*8,
                           KEYBOARD_FIXED_DEFAULT_TMARGIN+(KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT+KEYBOAED_FIXED_DEFAULT_VSPACING)*3,
                           KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH,
                           KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT);
    whBtn->setCharId('?');
    whBtn->updateStyleSheet(KEYBOARD_LETTER_COLOR_NORMAL,KEYBOARD_LETTER_COLOR_PRESSED,KEYBOARD_LETTER_COLOR_PRESSED,
                            KEYBOARD_LETTER_COLOR_BORDER_NORMAL,KEYBOARD_LETTER_COLOR_BORDER_PRESSED,KEYBOARD_FONT_COLOR_PRESS);
    connect(whBtn, &KBButton::clicked, this, &CharsWidget::onBtnClicked);
    m_mapBtnGeometrys[whBtn] = whBtn->geometry();

    KBButton *leftBtn = new KBButton(this);
    leftBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_LETTER_L4+(KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH+KEYBOAED_FIXED_DEFAULT_HSPACING)*9,
                           KEYBOARD_FIXED_DEFAULT_TMARGIN+(KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT+KEYBOAED_FIXED_DEFAULT_VSPACING)*3,
                           KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH,
                           KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT);
    leftBtn->setObjectName("btn_left");
    leftBtn->updateStyleSheet(KEYBOARD_OTHER_COLOR_NORMAL,KEYBOARD_OTHER_COLOR_PRESSED,
                                   KEYBOARD_OTHER_COLOR_PRESSED,KEYBOARD_OTHER_COLOR_BORDER_NORMAL,
                                   KEYBOARD_OTHER_COLOR_BORDER_PRESSED,KEYBOARD_OTHER_FONT_COLOR_PRESS);
    leftBtn->setIcon(QIcon(":/images/images/left.svg"));
    connect(leftBtn, &KBButton::clicked, this, &CharsWidget::onBtnClicked);
    m_mapBtnGeometrys[leftBtn] = leftBtn->geometry();

    KBButton *rightBtn = new KBButton(this);
    rightBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_LETTER_L4+(KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH+KEYBOAED_FIXED_DEFAULT_HSPACING)*10,
                           KEYBOARD_FIXED_DEFAULT_TMARGIN+(KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT+KEYBOAED_FIXED_DEFAULT_VSPACING)*3,
                           KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH,
                           KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT);
    rightBtn->setObjectName("btn_right");
    rightBtn->updateStyleSheet(KEYBOARD_OTHER_COLOR_NORMAL,KEYBOARD_OTHER_COLOR_PRESSED,
                                   KEYBOARD_OTHER_COLOR_PRESSED,KEYBOARD_OTHER_COLOR_BORDER_NORMAL,
                                   KEYBOARD_OTHER_COLOR_BORDER_PRESSED,KEYBOARD_OTHER_FONT_COLOR_PRESS);
    rightBtn->setIcon(QIcon(":/images/images/right.svg"));
    connect(rightBtn, &KBButton::clicked, this, &CharsWidget::onBtnClicked);
    m_mapBtnGeometrys[rightBtn] = rightBtn->geometry();
}

void CharsWidget::adjustGeometry(double lfWidthScale, double lfHeightScale, bool isVertical/* = false*/, bool floatStatus)
{
    QMap<KBButton*, QRect>::iterator itGeometry = m_mapBtnGeometrys.begin();
    for (; itGeometry != m_mapBtnGeometrys.end(); itGeometry ++) {
        KBButton *button = itGeometry.key();
        if (button) {
            QRect oldGeometry = itGeometry.value();
            QRect newGeometry = oldGeometry;
            if (floatStatus) {
                newGeometry.setX(oldGeometry.x()*lfWidthScale*KEYBOARD_FLOAT_PERCENTAGE);
                newGeometry.setY(oldGeometry.y()*lfHeightScale);
                newGeometry.setWidth(oldGeometry.width()*lfWidthScale*KEYBOARD_FLOAT_PERCENTAGE);
                newGeometry.setHeight(oldGeometry.height()*lfHeightScale);
            } else {
                newGeometry.setX(oldGeometry.x()*lfWidthScale);
                newGeometry.setY(oldGeometry.y()*lfHeightScale);
                newGeometry.setWidth(oldGeometry.width()*lfWidthScale);
                newGeometry.setHeight(oldGeometry.height()*lfHeightScale);
            }
            button->setGeometry(newGeometry);
        }
    }
}

void CharsWidget::onBtnClicked(QChar charId)
{
    QObject *obj = sender();
    KBButton *btn = static_cast<KBButton*>(obj);
    QString objName = btn->objectName();
    int lastUnderline = objName.lastIndexOf('_');
    int start = strnlen("btn_", 1024);
    int keyLength = lastUnderline - start;
    QString keyName = objName.mid(start, keyLength);
    if (keyName == BTN_RETURN) {
        Q_EMIT specialBtnClicked(PAGE_LETTER);
    } else if (keyName == "more") {
        Q_EMIT specialBtnClicked(PAGE_CHARSMORE);
    } else if (keyName == "number") {
        Q_EMIT specialBtnClicked(PAGE_NUMBER);
    } else if (charId != QChar::Null) {
        Q_EMIT normalBtnClicked(charId);
    } else {
        Q_EMIT specialBtnClicked(keyName);
    }
}
