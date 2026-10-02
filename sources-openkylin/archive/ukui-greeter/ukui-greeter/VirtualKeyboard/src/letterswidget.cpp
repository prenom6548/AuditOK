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
#include "letterswidget.h"
#include "commondef.h"
#include "xeventmonitor.h"
#include <QDebug>
#include <QX11Info>

LettersWidget::LettersWidget(QWidget *parent/* = nullptr*/)
    : QWidget(parent)
{
    this->setAttribute(Qt::WA_TranslucentBackground);//背景透明
    initUI();
    onCapsChanged();
    if(QX11Info::isPlatformX11()){
    connect(XEventMonitor::instance(), SIGNAL(keyRelease(const QString &)),
            this, SLOT(onGlobalKeyRelease(const QString &)));
    }
}

LettersWidget::~LettersWidget()
{

}

void LettersWidget::initUI()
{
    // line 1
    QChar chLine1[] = {'q','w','e','r','t','y','u','i','o','p'};
    for (int n = 0; n < sizeof(chLine1)/sizeof(QChar); n++) {
        KBButton *letterBtn = new KBButton(this);
        QString objName = "btn_" + QString::number(chLine1[n].toLatin1());
        letterBtn->setObjectName(objName);
        letterBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_LETTER_L1+(KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH+KEYBOAED_FIXED_DEFAULT_HSPACING)*n,
                               KEYBOARD_FIXED_DEFAULT_TMARGIN,
                               KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH,
                               KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT);
        letterBtn->setCharId(chLine1[n]);
        letterBtn->updateStyleSheet(KEYBOARD_LETTER_COLOR_NORMAL,KEYBOARD_LETTER_COLOR_PRESSED,KEYBOARD_LETTER_COLOR_PRESSED,
                                    KEYBOARD_LETTER_COLOR_BORDER_NORMAL,KEYBOARD_LETTER_COLOR_BORDER_PRESSED,KEYBOARD_FONT_COLOR_PRESS);
        connect(letterBtn, &KBButton::clicked, this, &LettersWidget::onBtnClicked);
        m_mapBtnGeometrys[letterBtn] = letterBtn->geometry();
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
    backspaceBtn->setIcon(QIcon(":/images/images/delet.svg"));
    connect(backspaceBtn, &KBButton::clicked, this, &LettersWidget::onBtnClicked);
    m_mapBtnGeometrys[backspaceBtn] = backspaceBtn->geometry();
    qDebug()<<"backspaceBtn geometry:"<<backspaceBtn->geometry();

    // line 2
    QChar chLine2[] = {'a','s','d','f','g','h','j','k','l'};
    for (int n = 0; n < sizeof(chLine2)/sizeof(QChar); n++) {
        KBButton *letterBtn = new KBButton(this);
        QString objName = "btn_" + QString::number(chLine2[n].toLatin1());
        letterBtn->setObjectName(objName);
        letterBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_LETTER_L2+(KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH+KEYBOAED_FIXED_DEFAULT_HSPACING)*n,
                               KEYBOARD_FIXED_DEFAULT_TMARGIN+(KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT+KEYBOAED_FIXED_DEFAULT_VSPACING),
                               KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH,
                               KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT);
        letterBtn->setCharId(chLine2[n]);
        letterBtn->updateStyleSheet(KEYBOARD_LETTER_COLOR_NORMAL,KEYBOARD_LETTER_COLOR_PRESSED,KEYBOARD_LETTER_COLOR_PRESSED,
                                    KEYBOARD_LETTER_COLOR_BORDER_NORMAL,KEYBOARD_LETTER_COLOR_BORDER_PRESSED,KEYBOARD_FONT_COLOR_PRESS);
        connect(letterBtn, &KBButton::clicked, this, &LettersWidget::onBtnClicked);
        m_mapBtnGeometrys[letterBtn] = letterBtn->geometry();
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
    enterBtn->setIcon(QIcon(":/images/images/enter.svg"));
    connect(enterBtn, &KBButton::clicked, this, &LettersWidget::onBtnClicked);
    m_mapBtnGeometrys[enterBtn] = enterBtn->geometry();

    // line 3
    QChar chLine3[] = {'z','x','c','v','b','n','m',',','.'};
    for (int n = 0; n < sizeof(chLine3)/sizeof(QChar); n++) {
        KBButton *letterBtn = new KBButton(this);
        QString objName = "btn_" + QString::number(chLine3[n].toLatin1());
        letterBtn->setObjectName(objName);
        letterBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_LETTER_L3+(KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH+KEYBOAED_FIXED_DEFAULT_HSPACING)*(n+1),
                               KEYBOARD_FIXED_DEFAULT_TMARGIN+(KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT+KEYBOAED_FIXED_DEFAULT_VSPACING)*2,
                               KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH,
                               KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT);
        letterBtn->setCharId(chLine3[n]);
        letterBtn->updateStyleSheet(KEYBOARD_LETTER_COLOR_NORMAL,KEYBOARD_LETTER_COLOR_PRESSED,KEYBOARD_LETTER_COLOR_PRESSED,
                                    KEYBOARD_LETTER_COLOR_BORDER_NORMAL,KEYBOARD_LETTER_COLOR_BORDER_PRESSED,
                                    KEYBOARD_FONT_COLOR_PRESS);
        connect(letterBtn, &KBButton::clicked, this, &LettersWidget::onBtnClicked);
        m_mapBtnGeometrys[letterBtn] = letterBtn->geometry();
    }
    // shift l
    KBButton *shiftLBtn = new KBButton(this);
    shiftLBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_LETTER_L3,
                           KEYBOARD_FIXED_DEFAULT_TMARGIN+(KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT+KEYBOAED_FIXED_DEFAULT_VSPACING)*2,
                           KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH,
                           KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT);
    shiftLBtn->setObjectName("btn_shift_l");
    shiftLBtn->updateStyleSheet(KEYBOARD_OTHER_COLOR_NORMAL,KEYBOARD_OTHER_COLOR_PRESSED,
                                   KEYBOARD_OTHER_COLOR_PRESSED,KEYBOARD_OTHER_COLOR_BORDER_NORMAL,
                                   KEYBOARD_OTHER_COLOR_BORDER_PRESSED,
                                KEYBOARD_OTHER_FONT_COLOR_PRESS);
    shiftLBtn->setIcon(QIcon(":/images/images/shift.svg"));
    connect(shiftLBtn, &KBButton::clicked, this, &LettersWidget::onBtnClicked);
    m_mapBtnGeometrys[shiftLBtn] = shiftLBtn->geometry();
    // shift r
    KBButton *shiftRBtn = new KBButton(this);
    shiftRBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_LETTER_L3+(KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH+KEYBOAED_FIXED_DEFAULT_HSPACING)*10,
                           KEYBOARD_FIXED_DEFAULT_TMARGIN+(KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT+KEYBOAED_FIXED_DEFAULT_VSPACING)*2,
                           KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH,
                           KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT);
    shiftRBtn->setObjectName("btn_shift_r");
    shiftRBtn->updateStyleSheet(KEYBOARD_OTHER_COLOR_NORMAL,KEYBOARD_OTHER_COLOR_PRESSED,
                                   KEYBOARD_OTHER_COLOR_PRESSED,KEYBOARD_OTHER_COLOR_BORDER_NORMAL,
                                   KEYBOARD_OTHER_COLOR_BORDER_PRESSED,
                                KEYBOARD_OTHER_FONT_COLOR_PRESS);
    shiftRBtn->setIcon(QIcon(":/images/images/shift.svg"));
    connect(shiftRBtn, &KBButton::clicked, this, &LettersWidget::onBtnClicked);
    m_mapBtnGeometrys[shiftRBtn] = shiftRBtn->geometry();

    // line 4
    KBButton *symbolBtn = new KBButton(this);
    symbolBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_LETTER_L4,
                           KEYBOARD_FIXED_DEFAULT_TMARGIN+(KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT+KEYBOAED_FIXED_DEFAULT_VSPACING)*3,
                           KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH,
                           KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT);
    symbolBtn->setObjectName("btn_symbol");
    symbolBtn->setText(tr("&&?!"));
    symbolBtn->updateStyleSheet(KEYBOARD_OTHER_COLOR_NORMAL,KEYBOARD_OTHER_COLOR_PRESSED,
                                   KEYBOARD_OTHER_COLOR_PRESSED,KEYBOARD_OTHER_COLOR_BORDER_NORMAL,
                                   KEYBOARD_OTHER_COLOR_BORDER_PRESSED,
                                KEYBOARD_OTHER_FONT_COLOR_PRESS);
    connect(symbolBtn, &KBButton::clicked, this, &LettersWidget::onBtnClicked);
    m_mapBtnGeometrys[symbolBtn] = symbolBtn->geometry();

    KBButton *numBtn = new KBButton(this);
    numBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_LETTER_L4+(KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH+KEYBOAED_FIXED_DEFAULT_HSPACING),
                           KEYBOARD_FIXED_DEFAULT_TMARGIN+(KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT+KEYBOAED_FIXED_DEFAULT_VSPACING)*3,
                           KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH,
                           KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT);
    numBtn->setObjectName("btn_num");
    numBtn->setText(tr("123"));
    numBtn->updateStyleSheet(KEYBOARD_OTHER_COLOR_NORMAL,KEYBOARD_OTHER_COLOR_PRESSED,
                                   KEYBOARD_OTHER_COLOR_PRESSED,KEYBOARD_OTHER_COLOR_BORDER_NORMAL,
                                   KEYBOARD_OTHER_COLOR_BORDER_PRESSED,
                             KEYBOARD_OTHER_FONT_COLOR_PRESS);
    connect(numBtn, &KBButton::clicked, this, &LettersWidget::onBtnClicked);
    m_mapBtnGeometrys[numBtn] = numBtn->geometry();

    KBButton *ctrlBtn = new KBButton(this);
    ctrlBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_LETTER_L4+(KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH+KEYBOAED_FIXED_DEFAULT_HSPACING)*2,
                           KEYBOARD_FIXED_DEFAULT_TMARGIN+(KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT+KEYBOAED_FIXED_DEFAULT_VSPACING)*3,
                           KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH,
                           KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT);
    ctrlBtn->setObjectName("btn_ctrl");
    ctrlBtn->setText("Ctrl");
    ctrlBtn->updateStyleSheet(KEYBOARD_OTHER_COLOR_NORMAL,KEYBOARD_OTHER_COLOR_PRESSED,
                                   KEYBOARD_OTHER_COLOR_PRESSED,KEYBOARD_OTHER_COLOR_BORDER_NORMAL,
                                   KEYBOARD_OTHER_COLOR_BORDER_PRESSED,
                              KEYBOARD_OTHER_FONT_COLOR_PRESS);
    connect(ctrlBtn, &KBButton::clicked, this, &LettersWidget::onBtnClicked);
    m_mapBtnGeometrys[ctrlBtn] = ctrlBtn->geometry();

    KBButton *spaceBtn = new KBButton(this);
    spaceBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_LETTER_L4+(KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH+KEYBOAED_FIXED_DEFAULT_HSPACING)*3,
                           KEYBOARD_FIXED_DEFAULT_TMARGIN+(KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT+KEYBOAED_FIXED_DEFAULT_VSPACING)*3,
                           KEYBOARD_FIXED_DEFAULT_SPACEBTN_WIDTH,
                           KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT);
    spaceBtn->setObjectName("btn_space");
    spaceBtn->updateStyleSheet(KEYBOARD_LETTER_COLOR_NORMAL,KEYBOARD_LETTER_COLOR_PRESSED,
                                   KEYBOARD_LETTER_COLOR_PRESSED,KEYBOARD_LETTER_COLOR_BORDER_NORMAL,
                                   KEYBOARD_LETTER_COLOR_BORDER_PRESSED,
                               KEYBOARD_OTHER_FONT_COLOR_PRESS);
    spaceBtn->setIcon(QIcon(":/images/images/space.svg"));
    connect(spaceBtn, &KBButton::clicked, this, &LettersWidget::onBtnClicked);
    m_mapBtnGeometrys[spaceBtn] = spaceBtn->geometry();

    KBButton *altBtn = new KBButton(this);
    altBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_LETTER_L4+(KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH+KEYBOAED_FIXED_DEFAULT_HSPACING)*8,
                           KEYBOARD_FIXED_DEFAULT_TMARGIN+(KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT+KEYBOAED_FIXED_DEFAULT_VSPACING)*3,
                           KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH,
                           KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT);
    altBtn->setObjectName("btn_alt");
    altBtn->setText("Alt");
    altBtn->updateStyleSheet(KEYBOARD_OTHER_COLOR_NORMAL,KEYBOARD_OTHER_COLOR_PRESSED,
                                   KEYBOARD_OTHER_COLOR_PRESSED,KEYBOARD_OTHER_COLOR_BORDER_NORMAL,
                                   KEYBOARD_OTHER_COLOR_BORDER_PRESSED,
                             KEYBOARD_OTHER_FONT_COLOR_PRESS);
    connect(altBtn, &KBButton::clicked, this, &LettersWidget::onBtnClicked);
    m_mapBtnGeometrys[altBtn] = altBtn->geometry();

    KBButton *leftBtn = new KBButton(this);
    leftBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_LETTER_L4+(KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH+KEYBOAED_FIXED_DEFAULT_HSPACING)*9,
                           KEYBOARD_FIXED_DEFAULT_TMARGIN+(KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT+KEYBOAED_FIXED_DEFAULT_VSPACING)*3,
                           KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH,
                           KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT);
    leftBtn->setObjectName("btn_left");
    leftBtn->updateStyleSheet(KEYBOARD_OTHER_COLOR_NORMAL,KEYBOARD_OTHER_COLOR_PRESSED,
                                   KEYBOARD_OTHER_COLOR_PRESSED,KEYBOARD_OTHER_COLOR_BORDER_NORMAL,
                                   KEYBOARD_OTHER_COLOR_BORDER_PRESSED,
                              KEYBOARD_OTHER_FONT_COLOR_PRESS);
    leftBtn->setIcon(QIcon(":/images/images/left.svg"));
    connect(leftBtn, &KBButton::clicked, this, &LettersWidget::onBtnClicked);
    m_mapBtnGeometrys[leftBtn] = leftBtn->geometry();

    KBButton *rightBtn = new KBButton(this);
    rightBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_LETTER_L4+(KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH+KEYBOAED_FIXED_DEFAULT_HSPACING)*10,
                           KEYBOARD_FIXED_DEFAULT_TMARGIN+(KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT+KEYBOAED_FIXED_DEFAULT_VSPACING)*3,
                           KEYBOARD_FIXED_DEFAULT_LETTER_WIDTH,
                           KEYBOARD_FIXED_DEFAULT_LETTER_HEIGHT);
    rightBtn->setObjectName("btn_right");
    rightBtn->updateStyleSheet(KEYBOARD_OTHER_COLOR_NORMAL,KEYBOARD_OTHER_COLOR_PRESSED,
                                   KEYBOARD_OTHER_COLOR_PRESSED,KEYBOARD_OTHER_COLOR_BORDER_NORMAL,
                                   KEYBOARD_OTHER_COLOR_BORDER_PRESSED,
                               KEYBOARD_OTHER_FONT_COLOR_PRESS);
    rightBtn->setIcon(QIcon(":/images/images/right.svg"));
    connect(rightBtn, &KBButton::clicked, this, &LettersWidget::onBtnClicked);
    m_mapBtnGeometrys[rightBtn] = rightBtn->geometry();
}

void LettersWidget::adjustGeometry(double lfWidthScale, double lfHeightScale, bool isVertical/* = false*/, bool floatStatus)
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

void LettersWidget::onBtnClicked(QChar charId)
{
    QObject *obj = sender();
    KBButton *btn = static_cast<KBButton*>(obj);
    QString objName = btn->objectName();
    int lastUnderline = objName.lastIndexOf('_');
    int start = strnlen("btn_", 1024);
    int keyLength = lastUnderline - start;
    QString keyName = objName.mid(start, keyLength);

    if (charId != QChar::Null && isShift && !capsState) { //第一次按shift，按后取消shift状态
        isShift = false;
        changeFuncKeyStyle("shift_l", false);
        changeFuncKeyStyle("shift_r", false);
        toggleCase();
    }

    if (keyName == "num") {                 //数字页面
        Q_EMIT specialBtnClicked(PAGE_NUMBER);
    } else if (keyName == "symbol") {           //符号页面
        Q_EMIT specialBtnClicked(PAGE_CHAR);
    } else if (keyName == BTN_SHIFT) {          //shift
        if (capsState) {
            capsState = false;
            isShift = false;
            changeFuncKeyStyle("shift_l", false);
            changeFuncKeyStyle("shift_r", false);
            Q_EMIT specialBtnClicked(BTN_CAPSLOCK);
        } else if (isShift) {               //shift键锁定
            capsState = true;
            isShift = true;
            changeFuncKeyStyle("shift_l", true);
            changeFuncKeyStyle("shift_r", true);
            Q_EMIT specialBtnClicked(BTN_CAPSLOCK);
        } else {
            isShift = true;
            changeFuncKeyStyle("shift_l", true);
            changeFuncKeyStyle("shift_r", true);
        }
        toggleCase();
    } else if (charId != QChar::Null) {       //字符
        Q_EMIT normalBtnClicked(charId);
    } else {                    //ctrl或者alt
        Q_EMIT specialBtnClicked(keyName);

    }
}


void LettersWidget::onGlobalKeyRelease(const QString &key)
{
    if(key == "Caps_Lock")
    {
        onCapsChanged();
    }
}

void LettersWidget::onCapsChanged()
{
    capsState = checkCapsState();
    isShift = capsState;
    for (int i = 97; i < 123; i++) {            //大小写切换
        QString objName = QString("btn_%1").arg(QString::number(i));
        KBButton *btn = findChild<KBButton*>(objName);
        btn->setCapsStatus(capsState);
        changeFuncKeyStyle("shift_l", capsState);
        changeFuncKeyStyle("shift_r", capsState);
    }
}

void LettersWidget::toggleCase()
{
    for (int i = 97; i < 123; i++) {            //大小写切换
        QString objName = QString("btn_%1").arg(QString::number(i));
        KBButton *btn = findChild<KBButton*>(objName);
        btn->setShiftState(isShift);
    }
}

void LettersWidget::changeFuncKeyStyle(QString obj, bool isLock)
{
    if(obj == BTN_CAPSLOCK)
        return;
    QString objName = QString("btn_%1").arg(obj);
    KBButton *btn = findChild<KBButton*>(objName);
    if (isLock) {
        btn->updateStyleSheet(KEYBOARD_OTHER_COLOR_PRESSED,KEYBOARD_OTHER_COLOR_PRESSED,
                              KEYBOARD_OTHER_COLOR_PRESSED,KEYBOARD_OTHER_COLOR_BORDER_NORMAL,
                              KEYBOARD_OTHER_COLOR_BORDER_PRESSED,KEYBOARD_OTHER_FONT_COLOR_PRESS,
                              KBButton::BORDER_RADIUS_ALL,true);
        if (capsState && obj.contains(BTN_SHIFT)) {
            btn->setIcon(QIcon(":/images/images/shift_lock.svg"));
        } else if (obj.contains(BTN_SHIFT)) {
            btn->setIcon(QIcon(":/images/images/rectangle.svg"));
        }
    } else {
        btn->updateStyleSheet(KEYBOARD_OTHER_COLOR_NORMAL,KEYBOARD_OTHER_COLOR_PRESSED,
                              KEYBOARD_OTHER_COLOR_PRESSED,KEYBOARD_OTHER_COLOR_BORDER_NORMAL,
                              KEYBOARD_OTHER_COLOR_BORDER_PRESSED,KEYBOARD_OTHER_FONT_COLOR_PRESS);
        if (obj.contains(BTN_SHIFT)) {
            btn->setIcon(QIcon(":/images/images/shift.svg"));
        }
    }
}
