/*
 * Copyright (C) 2023 KylinSoftCo., Ltd.
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
#include "numberswidget.h"
#include "kbbutton.h"
#include "commondef.h"

#include <QFrame>
#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QVariant>
#include <QEvent>
#include <QDebug>

NumbersWidget::NumbersWidget(QWidget *parent)
    : QWidget(parent)
{
    this->setAttribute(Qt::WA_TranslucentBackground);//背景透明
    initUI();
}

NumbersWidget::~NumbersWidget()
{

}

void NumbersWidget::initUI()
{
    // all chars
    m_layoutBtnList = new QVBoxLayout();
    m_layoutBtnList->setContentsMargins(8,0,0,0);
    m_layoutBtnList->setSpacing(1);
    listFrame = new QFrame();
    listFrame->setObjectName("listFrame");
    listFrame->setLayout(m_layoutBtnList);
    listFrame->setStyleSheet("QFrame{border-radius: 8px}");
    listFrame->setFrameStyle(QFrame::Plain);
    m_scrollFrame = new QScrollArea(this);
    m_scrollFrame->setObjectName("scrollFrame");
    m_scrollFrame->setFocusPolicy(Qt::NoFocus);
    m_scrollFrame->setContentsMargins(0, 0, 0, 0);
    m_scrollFrame->setStyleSheet("QScrollArea {background-color: #C0CED3D9; border-radius:8px;}");
    m_scrollFrame->viewport()->setStyleSheet("background-color:transparent;");
    m_scrollFrame->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    m_scrollFrame->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollFrame->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    m_scrollFrame->setWidgetResizable(true);
    m_scrollFrame->setWidget(listFrame);
    m_scrollFrame->setGeometry(KEYBOARD_FIXED_DEFAULT_NUMBER_L1, KEYBOARD_FIXED_DEFAULT_TMARGIN,
                               KEYBOARD_FIXED_DEFAULT_NUMBER_WIDTH, KEYBOARD_FIXED_DEFAULT_NUMBER_CHARS_HEIGHT);
    listFrame->setGeometry(KEYBOARD_FIXED_DEFAULT_NUMBER_L1, KEYBOARD_FIXED_DEFAULT_TMARGIN,
                           KEYBOARD_FIXED_DEFAULT_NUMBER_WIDTH, KEYBOARD_FIXED_DEFAULT_NUMBER_CHARS_HEIGHT);
    m_mapBtnGeometrys[m_scrollFrame] = m_scrollFrame->geometry();
    //m_mapBtnGeometrys[listFrame] = listFrame->geometry();
    QChar chChars[] = {',', '.', '?', '!', '\'', ':', '~', '@', ';', '"',
                       '/', '(', ')', '_', '+', '=', '`', '^', '#', '*',
                       '%', '&', '\\', '[', ']', '<', '>', '{', '}', '|',
                       '$', '-'};
    for (int n = 0; n < sizeof(chChars)/sizeof(QChar); n++) {
        KBButton *charBtn = new KBButton(listFrame);
        charBtn->setCharId(chChars[n]);
        charBtn->updateStyleSheet(KEYBOARD_OTHER_COLOR_NORMAL,KEYBOARD_OTHER_COLOR_PRESSED,
                                       KEYBOARD_OTHER_COLOR_PRESSED,KEYBOARD_OTHER_COLOR_NORMAL,
                                       KEYBOARD_OTHER_COLOR_PRESSED,KEYBOARD_OTHER_FONT_COLOR_PRESS,
                                  KBButton::BORDER_RADIUS_NONE);
        charBtn->setObjectName(QString("btn_%1").arg(QString(chChars[n])));
        connect(charBtn, &KBButton::clicked, this, &NumbersWidget::onBtnClicked);
        charBtn->setFixedHeight(KEYBOARD_FIXED_DEFAULT_NUMBER_HEIGHT);
        m_layoutBtnList->addWidget(charBtn);
    }
    // line 1
    QChar chLine1[] = {'1','2','3'};
    for (int n = 0; n < sizeof(chLine1)/sizeof(QChar); n++) {
        KBButton *numberBtn = new KBButton(this);
        numberBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_NUMBER_L1+(KEYBOARD_FIXED_DEFAULT_NUMBER_WIDTH+KEYBOAED_FIXED_DEFAULT_HSPACING)*(n+1),
                               KEYBOARD_FIXED_DEFAULT_TMARGIN,
                               KEYBOARD_FIXED_DEFAULT_NUMBER_WIDTH,
                               KEYBOARD_FIXED_DEFAULT_NUMBER_HEIGHT);
        numberBtn->setCharId(chLine1[n]);
        numberBtn->updateStyleSheet(KEYBOARD_LETTER_COLOR_NORMAL,KEYBOARD_LETTER_COLOR_PRESSED,KEYBOARD_LETTER_COLOR_PRESSED,
                                    KEYBOARD_LETTER_COLOR_BORDER_NORMAL,KEYBOARD_LETTER_COLOR_BORDER_PRESSED,KEYBOARD_FONT_COLOR_PRESS);
        connect(numberBtn, &KBButton::clicked, this, &NumbersWidget::onBtnClicked);
        m_mapBtnGeometrys[numberBtn] = numberBtn->geometry();
    }
    // backspace
    KBButton *backspaceBtn = new KBButton(this);
    backspaceBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_NUMBER_L1+(KEYBOARD_FIXED_DEFAULT_NUMBER_WIDTH+KEYBOAED_FIXED_DEFAULT_HSPACING)*4,
                           KEYBOARD_FIXED_DEFAULT_TMARGIN,
                           KEYBOARD_FIXED_DEFAULT_NUMBER_WIDTH,
                           KEYBOARD_FIXED_DEFAULT_NUMBER_HEIGHT);
    backspaceBtn->setObjectName("btn_backspace");
    backspaceBtn->updateStyleSheet(KEYBOARD_OTHER_COLOR_NORMAL,KEYBOARD_OTHER_COLOR_PRESSED,
                                   KEYBOARD_OTHER_COLOR_PRESSED,KEYBOARD_OTHER_COLOR_BORDER_NORMAL,
                                   KEYBOARD_OTHER_COLOR_BORDER_PRESSED,KEYBOARD_OTHER_FONT_COLOR_PRESS);
    backspaceBtn->setIcon(QIcon(":/images/images/delet.svg"));
    connect(backspaceBtn, &KBButton::clicked, this, &NumbersWidget::onBtnClicked);
    m_mapBtnGeometrys[backspaceBtn] = backspaceBtn->geometry();

    // line 2
    QChar chLine2[] = {'4','5','6'};
    for (int n = 0; n < sizeof(chLine2)/sizeof(QChar); n++) {
        KBButton *numberBtn = new KBButton(this);
        numberBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_NUMBER_L2+(KEYBOARD_FIXED_DEFAULT_NUMBER_WIDTH+KEYBOAED_FIXED_DEFAULT_HSPACING)*(n+1),
                               KEYBOARD_FIXED_DEFAULT_TMARGIN+(KEYBOARD_FIXED_DEFAULT_NUMBER_HEIGHT+KEYBOAED_FIXED_DEFAULT_VSPACING)*1,
                               KEYBOARD_FIXED_DEFAULT_NUMBER_WIDTH,
                               KEYBOARD_FIXED_DEFAULT_NUMBER_HEIGHT);
        numberBtn->setCharId(chLine2[n]);
        numberBtn->updateStyleSheet(KEYBOARD_LETTER_COLOR_NORMAL,KEYBOARD_LETTER_COLOR_PRESSED,KEYBOARD_LETTER_COLOR_PRESSED,
                                    KEYBOARD_LETTER_COLOR_BORDER_NORMAL,KEYBOARD_LETTER_COLOR_BORDER_PRESSED,KEYBOARD_FONT_COLOR_PRESS);
        connect(numberBtn, &KBButton::clicked, this, &NumbersWidget::onBtnClicked);
        m_mapBtnGeometrys[numberBtn] = numberBtn->geometry();
    }
    // @
    KBButton *atBtn = new KBButton(this);
    atBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_NUMBER_L2+(KEYBOARD_FIXED_DEFAULT_NUMBER_WIDTH+KEYBOAED_FIXED_DEFAULT_HSPACING)*4,
                           KEYBOARD_FIXED_DEFAULT_TMARGIN+(KEYBOARD_FIXED_DEFAULT_NUMBER_HEIGHT+KEYBOAED_FIXED_DEFAULT_VSPACING)*1,
                           KEYBOARD_FIXED_DEFAULT_NUMBER_WIDTH,
                           KEYBOARD_FIXED_DEFAULT_NUMBER_HEIGHT);
    atBtn->setCharId('@');
    atBtn->updateStyleSheet(KEYBOARD_OTHER_COLOR_NORMAL,KEYBOARD_OTHER_COLOR_PRESSED,
                                   KEYBOARD_OTHER_COLOR_PRESSED,KEYBOARD_OTHER_COLOR_BORDER_NORMAL,
                                   KEYBOARD_OTHER_COLOR_BORDER_PRESSED,KEYBOARD_OTHER_FONT_COLOR_PRESS);
    connect(atBtn, &KBButton::clicked, this, &NumbersWidget::onBtnClicked);
    m_mapBtnGeometrys[atBtn] = atBtn->geometry();
    // line 3
    QChar chLine3[] = {'7','8','9'};
    for (int n = 0; n < sizeof(chLine3)/sizeof(QChar); n++) {
        KBButton *numberBtn = new KBButton(this);
        numberBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_NUMBER_L3+(KEYBOARD_FIXED_DEFAULT_NUMBER_WIDTH+KEYBOAED_FIXED_DEFAULT_HSPACING)*(n+1),
                               KEYBOARD_FIXED_DEFAULT_TMARGIN+(KEYBOARD_FIXED_DEFAULT_NUMBER_HEIGHT+KEYBOAED_FIXED_DEFAULT_VSPACING)*2,
                               KEYBOARD_FIXED_DEFAULT_NUMBER_WIDTH,
                               KEYBOARD_FIXED_DEFAULT_NUMBER_HEIGHT);
        numberBtn->setCharId(chLine3[n]);
        numberBtn->updateStyleSheet(KEYBOARD_LETTER_COLOR_NORMAL,KEYBOARD_LETTER_COLOR_PRESSED,KEYBOARD_LETTER_COLOR_PRESSED,
                                    KEYBOARD_LETTER_COLOR_BORDER_NORMAL,KEYBOARD_LETTER_COLOR_BORDER_PRESSED,KEYBOARD_FONT_COLOR_PRESS);
        connect(numberBtn, &KBButton::clicked, this, &NumbersWidget::onBtnClicked);
        m_mapBtnGeometrys[numberBtn] = numberBtn->geometry();
    }
    // symbol
    KBButton *symbolBtn = new KBButton(this);
    symbolBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_NUMBER_L3+(KEYBOARD_FIXED_DEFAULT_NUMBER_WIDTH+KEYBOAED_FIXED_DEFAULT_HSPACING)*4,
                           KEYBOARD_FIXED_DEFAULT_TMARGIN+(KEYBOARD_FIXED_DEFAULT_NUMBER_HEIGHT+KEYBOAED_FIXED_DEFAULT_VSPACING)*2,
                           KEYBOARD_FIXED_DEFAULT_NUMBER_WIDTH,
                           KEYBOARD_FIXED_DEFAULT_NUMBER_HEIGHT);
    symbolBtn->setObjectName("btn_symbol");
    symbolBtn->setText(tr("&&?!"));
    symbolBtn->updateStyleSheet(KEYBOARD_OTHER_COLOR_NORMAL,KEYBOARD_OTHER_COLOR_PRESSED,
                                   KEYBOARD_OTHER_COLOR_PRESSED,KEYBOARD_OTHER_COLOR_BORDER_NORMAL,
                                   KEYBOARD_OTHER_COLOR_BORDER_PRESSED,KEYBOARD_OTHER_FONT_COLOR_PRESS);
    connect(symbolBtn, &KBButton::clicked, this, &NumbersWidget::onBtnClicked);
    m_mapBtnGeometrys[symbolBtn] = symbolBtn->geometry();

    // line 4
    KBButton *returnBtn = new KBButton(this);
    returnBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_NUMBER_L4,
                           KEYBOARD_FIXED_DEFAULT_TMARGIN+(KEYBOARD_FIXED_DEFAULT_NUMBER_HEIGHT+KEYBOAED_FIXED_DEFAULT_VSPACING)*3,
                           KEYBOARD_FIXED_DEFAULT_NUMBER_WIDTH,
                           KEYBOARD_FIXED_DEFAULT_NUMBER_HEIGHT);
    returnBtn->setObjectName("btn_return");
    returnBtn->setText(tr("Return"));
    returnBtn->updateStyleSheet(KEYBOARD_OTHER_COLOR_NORMAL,KEYBOARD_OTHER_COLOR_PRESSED,
                                   KEYBOARD_OTHER_COLOR_PRESSED,KEYBOARD_OTHER_COLOR_BORDER_NORMAL,
                                   KEYBOARD_OTHER_COLOR_BORDER_PRESSED,KEYBOARD_OTHER_FONT_COLOR_PRESS);
    connect(returnBtn, &KBButton::clicked, this, &NumbersWidget::onBtnClicked);
    m_mapBtnGeometrys[returnBtn] = returnBtn->geometry();

    // space
    KBButton *spaceBtn = new KBButton(this);
    spaceBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_NUMBER_L4+(KEYBOARD_FIXED_DEFAULT_NUMBER_WIDTH+KEYBOAED_FIXED_DEFAULT_HSPACING)*1,
                           KEYBOARD_FIXED_DEFAULT_TMARGIN+(KEYBOARD_FIXED_DEFAULT_NUMBER_HEIGHT+KEYBOAED_FIXED_DEFAULT_VSPACING)*3,
                           KEYBOARD_FIXED_DEFAULT_NUMBER_WIDTH,
                           KEYBOARD_FIXED_DEFAULT_NUMBER_HEIGHT);
    spaceBtn->setObjectName("btn_space");
    spaceBtn->updateStyleSheet(KEYBOARD_LETTER_COLOR_NORMAL,KEYBOARD_LETTER_COLOR_PRESSED,
                                   KEYBOARD_LETTER_COLOR_PRESSED,KEYBOARD_LETTER_COLOR_BORDER_NORMAL,
                                   KEYBOARD_LETTER_COLOR_BORDER_PRESSED,KEYBOARD_OTHER_FONT_COLOR_PRESS);
    spaceBtn->setIcon(QIcon(":/images/images/space.svg"));
    connect(spaceBtn, &KBButton::clicked, this, &NumbersWidget::onBtnClicked);
    m_mapBtnGeometrys[spaceBtn] = spaceBtn->geometry();

    // .
    KBButton *dianBtn = new KBButton(this);
    dianBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_NUMBER_L4+(KEYBOARD_FIXED_DEFAULT_NUMBER_WIDTH+KEYBOAED_FIXED_DEFAULT_HSPACING)*3,
                           KEYBOARD_FIXED_DEFAULT_TMARGIN+(KEYBOARD_FIXED_DEFAULT_NUMBER_HEIGHT+KEYBOAED_FIXED_DEFAULT_VSPACING)*3,
                           KEYBOARD_FIXED_DEFAULT_NUMBER_WIDTH,
                           KEYBOARD_FIXED_DEFAULT_NUMBER_HEIGHT);
    dianBtn->setCharId('.');
    dianBtn->updateStyleSheet(KEYBOARD_LETTER_COLOR_NORMAL,KEYBOARD_LETTER_COLOR_PRESSED,
                              KEYBOARD_LETTER_COLOR_PRESSED,KEYBOARD_LETTER_COLOR_BORDER_NORMAL,
                              KEYBOARD_LETTER_COLOR_BORDER_PRESSED,KEYBOARD_FONT_COLOR_PRESS);
    connect(dianBtn, &KBButton::clicked, this, &NumbersWidget::onBtnClicked);
    m_mapBtnGeometrys[dianBtn] = dianBtn->geometry();

    // 9
    KBButton *num0Btn = new KBButton(this);
    num0Btn->setGeometry(KEYBOARD_FIXED_DEFAULT_NUMBER_L4+(KEYBOARD_FIXED_DEFAULT_NUMBER_WIDTH+KEYBOAED_FIXED_DEFAULT_HSPACING)*2,
                           KEYBOARD_FIXED_DEFAULT_TMARGIN+(KEYBOARD_FIXED_DEFAULT_NUMBER_HEIGHT+KEYBOAED_FIXED_DEFAULT_VSPACING)*3,
                           KEYBOARD_FIXED_DEFAULT_NUMBER_WIDTH,
                           KEYBOARD_FIXED_DEFAULT_NUMBER_HEIGHT);
    num0Btn->setCharId('0');
    num0Btn->updateStyleSheet(KEYBOARD_LETTER_COLOR_NORMAL,KEYBOARD_LETTER_COLOR_PRESSED,
                              KEYBOARD_LETTER_COLOR_PRESSED,KEYBOARD_LETTER_COLOR_BORDER_NORMAL,
                              KEYBOARD_LETTER_COLOR_BORDER_PRESSED,KEYBOARD_FONT_COLOR_PRESS);
    connect(num0Btn, &KBButton::clicked, this, &NumbersWidget::onBtnClicked);
    m_mapBtnGeometrys[num0Btn] = num0Btn->geometry();

    // enter
    KBButton *enterBtn = new KBButton(this);
    enterBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_NUMBER_L3+(KEYBOARD_FIXED_DEFAULT_NUMBER_WIDTH+KEYBOAED_FIXED_DEFAULT_HSPACING)*4,
                           KEYBOARD_FIXED_DEFAULT_TMARGIN+(KEYBOARD_FIXED_DEFAULT_NUMBER_HEIGHT+KEYBOAED_FIXED_DEFAULT_VSPACING)*3,
                           KEYBOARD_FIXED_DEFAULT_NUMBER_WIDTH,
                           KEYBOARD_FIXED_DEFAULT_NUMBER_HEIGHT);
    enterBtn->setObjectName("btn_enter");
    enterBtn->updateStyleSheet(KEYBOARD_OTHER_COLOR_NORMAL,KEYBOARD_OTHER_COLOR_PRESSED,
                                   KEYBOARD_OTHER_COLOR_PRESSED,KEYBOARD_OTHER_COLOR_BORDER_NORMAL,
                                   KEYBOARD_OTHER_COLOR_BORDER_PRESSED,KEYBOARD_OTHER_FONT_COLOR_PRESS);
    enterBtn->setIcon(QIcon(":/images/images/enter.svg"));
    connect(enterBtn, &KBButton::clicked, this, &NumbersWidget::onBtnClicked);
    m_mapBtnGeometrys[enterBtn] = enterBtn->geometry();
}

void NumbersWidget::adjustGeometry(double lfWidthScale, double lfHeightScale, bool isVertical/* = false*/, bool floatStatus)
{
    QMap<QWidget*, QRect>::iterator itGeometry = m_mapBtnGeometrys.begin();
    for (; itGeometry != m_mapBtnGeometrys.end(); itGeometry ++) {
        QWidget *widget = itGeometry.key();
        if (widget) {
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
            widget->setGeometry(newGeometry);
        }
    }

    QChar chChars[] = {',', '.', '?', '!', '\'', ':', '~', '@', ';', '"',
                       '/', '(', ')', '_', '+', '=', '`', '^', '#', '*',
                       '%', '&', '\\', '[', ']', '<', '>', '{', '}', '|',
                       '$', '-'};
    for (int n = 0; n < sizeof(chChars)/sizeof(QChar); n++) { //单独更新符号btn的高度
        QString objName = QString("btn_%1").arg(QString(chChars[n]));
        KBButton *btn = findChild<KBButton*>(objName);
        btn->setFixedHeight(KEYBOARD_FIXED_DEFAULT_NUMBER_HEIGHT *lfHeightScale);
    }
    //更新listfarame的高度
    listFrame->setFixedHeight((KEYBOARD_FIXED_DEFAULT_NUMBER_HEIGHT *lfHeightScale +1) * (m_layoutBtnList->count()));
}

void NumbersWidget::onBtnClicked(QChar charId)
{
    QObject *obj = sender();
    KBButton *btn = static_cast<KBButton*>(obj);
    QString objName = btn->objectName();
    int lastUnderline = objName.lastIndexOf('_');
    int start = strlen("btn_");
    int keyLength = lastUnderline - start;
    QString keyName = objName.mid(start, keyLength);
    qDebug() << "keyName: " << keyName;
    if (keyName == BTN_RETURN) {
        Q_EMIT specialBtnClicked(PAGE_LETTER);
    } else if (keyName == "symbol") {
        Q_EMIT specialBtnClicked(PAGE_CHAR);
    } else if (charId != QChar::Null) {
        Q_EMIT narmalBtnClicked(charId);
    } else if (keyName == BTN_BACK) {
        Q_EMIT specialBtnClicked(BTN_BACK);
    } else {
        Q_EMIT specialBtnClicked(keyName);
    }
}

void NumbersWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange){
        refreshTranslate();
    }
}

void NumbersWidget::refreshTranslate()
{
    QMap<QWidget*, QRect>::iterator itGeometry = m_mapBtnGeometrys.begin();
    for (; itGeometry != m_mapBtnGeometrys.end(); itGeometry ++) {
        KBButton *button = qobject_cast<KBButton *>(itGeometry.key());
        if (button) {
            if (button->objectName() == "btn_return") {
                button->setText(tr("Return"));
            }
        }
    }
}
