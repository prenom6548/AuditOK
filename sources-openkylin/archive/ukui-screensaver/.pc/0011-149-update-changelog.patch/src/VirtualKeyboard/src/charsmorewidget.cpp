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
#include "charsmorewidget.h"
#include "commondef.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QVariant>
#include <QDebug>

CharsMoreWidget::CharsMoreWidget(QWidget *parent/* = nullptr*/)
    : QWidget(parent)
{
    this->setAttribute(Qt::WA_TranslucentBackground);//背景透明
    setWindowFlags(Qt::FramelessWindowHint |
                   Qt::WindowStaysOnTopHint |
                   Qt::WindowDoesNotAcceptFocus);
    initUI();
}

CharsMoreWidget::~CharsMoreWidget()
{

}

void CharsMoreWidget::adjustGeometry(double lfWidthScale, double lfHeightScale, bool isVertical/* = false*/, bool floatStatus)
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
        btn->setFixedHeight(KEYBOARD_FIXED_DEFAULT_NORMAL_CHARSMORE_BTN_HEIGHT *lfHeightScale);
    }
    //更新listfarame的高度
    listFrame->setFixedHeight(KEYBOARD_FIXED_DEFAULT_NORMAL_CHARSMORE_BTN_HEIGHT *m_vlayoutBtnList->count() *lfHeightScale);
}

void CharsMoreWidget::onBtnClicked(QChar charId)
{
    QObject *obj = sender();
    KBButton *btn = static_cast<KBButton*>(obj);
    QString objName = btn->objectName();
    int lastUnderline = objName.lastIndexOf('_');
    int start = strlen("btn_");
    int keyLength = lastUnderline - start;
    QString keyName = objName.mid(start, keyLength);
    if (keyName == BTN_RETURN) {
        Q_EMIT specialBtnClicked(PAGE_CHAR);
    } else if (charId != QChar::Null) {
        Q_EMIT normalBtnClicked(charId);
    } else {
        Q_EMIT specialBtnClicked(keyName);
    }
}

void CharsMoreWidget::initUI()
{
    // all chars
    m_vlayoutBtnList = new QVBoxLayout();
    m_vlayoutBtnList->setContentsMargins(8,0,0,0);
    m_vlayoutBtnList->setSpacing(1);
    listFrame = new QFrame();
    listFrame->setLayout(m_vlayoutBtnList);
    listFrame->setStyleSheet("QFrame{border-radius: 8px}");
    m_scrollFrame = new QScrollArea(this);
    m_scrollFrame->setContentsMargins(0, 0, 0, 0);
    m_scrollFrame->setFocusPolicy(Qt::NoFocus);
    m_scrollFrame->setStyleSheet("QScrollArea {background-color: #C0FFFFFF; border-radius:8px;}");
    m_scrollFrame->viewport()->setStyleSheet("background-color:transparent;");
    m_scrollFrame->verticalScrollBar()->setProperty("drawScrollBarGroove", false);
    m_scrollFrame->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollFrame->verticalScrollBar()->setContextMenuPolicy(Qt::NoContextMenu);
    m_scrollFrame->setWidgetResizable(true);
    m_scrollFrame->setWidget(listFrame);
    m_scrollFrame->setGeometry(KEYBOARD_FIXED_DEFAULT_CHARSMORE_L1, 0,
                               KEYBOARD_FIXED_DEFAULT_CHARSMORE_LIST_WIDTH, KEYBOARD_FIXED_DEFAULT_CHARSMORE_LIST_HEIGHT);
    listFrame->setGeometry(KEYBOARD_FIXED_DEFAULT_CHARSMORE_L1, 0,
                           KEYBOARD_FIXED_DEFAULT_CHARSMORE_LIST_WIDTH, KEYBOARD_FIXED_DEFAULT_CHARSMORE_LIST_HEIGHT);
    m_mapBtnGeometrys[m_scrollFrame] = m_scrollFrame->geometry();
    m_mapBtnGeometrys[listFrame] = listFrame->geometry();
    QChar chChars[] = {',', '.', '?', '!', '\'', ':', '~', '@', ';', '"',
                       '/', '(', ')', '_', '+', '=', '`', '^', '#', '*',
                       '%', '&', '\\', '[', ']', '<', '>', '{', '}', '|',
                       '$', '-', ' ', ' ', ' ', ' '};
    QHBoxLayout *lastLayout = nullptr;
    for (int n = 0; n < sizeof(chChars)/sizeof(QChar); n++) {
        KBButton *charBtn = new KBButton(listFrame);
        charBtn->setCharId(chChars[n]);
        charBtn->updateStyleSheet(KEYBOARD_LETTER_COLOR_NORMAL,KEYBOARD_LETTER_COLOR_PRESSED,
                                       KEYBOARD_LETTER_COLOR_PRESSED,KEYBOARD_LETTER_COLOR_NORMAL,
                                       KEYBOARD_LETTER_COLOR_PRESSED,KEYBOARD_FONT_COLOR_PRESS,
                                  KBButton::BORDER_RADIUS_NONE);
        if ((n%KEYBOARD_FIXED_DEFAULT_CHARSMORE_LIST_COLS) == 0) {
            lastLayout = new QHBoxLayout();
            lastLayout->setContentsMargins(0,0,0,0);
            lastLayout->setSpacing(1);
            m_vlayoutBtnList->addLayout(lastLayout);
        }
        if (chChars[n] == ' ') {
            charBtn->setDisabled(true);
        } else {
            charBtn->setObjectName(QString("btn_%1").arg(QString(chChars[n])));
        }
        charBtn->setFixedHeight(KEYBOARD_FIXED_DEFAULT_NORMAL_CHARSMORE_BTN_HEIGHT);
        lastLayout->addWidget(charBtn);
        m_mapSubWidgetListRects[charBtn] = charBtn->geometry();
        connect(charBtn, &KBButton::clicked, this, &CharsMoreWidget::onBtnClicked);
    }
    // backspace
    KBButton *backspaceBtn = new KBButton(this);
    backspaceBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_CHARSMORE_L1+(KEYBOARD_FIXED_DEFAULT_CHARSMORE_LIST_WIDTH+KEYBOARD_FIXED_DEFAULT_CHARSMORE_HSPACING),
                           0,
                           KEYBOARD_FIXED_DEFAULT_CHARSMORE_BTN_WIDTH,
                           KEYBOARD_FIXED_DEFAULT_CHARSMORE_BTN_HEIGHT);
    backspaceBtn->setObjectName("btn_backspace");
    backspaceBtn->updateStyleSheet(KEYBOARD_OTHER_COLOR_NORMAL,KEYBOARD_OTHER_COLOR_PRESSED,
                                   KEYBOARD_OTHER_COLOR_PRESSED,KEYBOARD_OTHER_COLOR_BORDER_NORMAL,
                                   KEYBOARD_OTHER_COLOR_BORDER_PRESSED,KEYBOARD_OTHER_FONT_COLOR_PRESS);
    backspaceBtn->setIcon(QIcon(":/images/images/delet.svg"));
    m_mapBtnGeometrys[backspaceBtn] = backspaceBtn->geometry();
    connect(backspaceBtn, &KBButton::clicked, this, &CharsMoreWidget::onBtnClicked);
    // enter
    KBButton *enterBtn = new KBButton(this);
    enterBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_CHARSMORE_L1+(KEYBOARD_FIXED_DEFAULT_CHARSMORE_LIST_WIDTH+KEYBOARD_FIXED_DEFAULT_CHARSMORE_HSPACING),
                           (KEYBOARD_FIXED_DEFAULT_CHARSMORE_BTN_HEIGHT+KEYBOARD_FIXED_DEFAULT_CHARSMORE_VSPACING)*1,
                           KEYBOARD_FIXED_DEFAULT_CHARSMORE_BTN_WIDTH,
                           KEYBOARD_FIXED_DEFAULT_CHARSMORE_BTN_HEIGHT);
    enterBtn->setObjectName("btn_enter");
    enterBtn->updateStyleSheet(KEYBOARD_OTHER_COLOR_NORMAL,KEYBOARD_OTHER_COLOR_PRESSED,
                                   KEYBOARD_OTHER_COLOR_PRESSED,KEYBOARD_OTHER_COLOR_BORDER_NORMAL,
                                   KEYBOARD_OTHER_COLOR_BORDER_PRESSED,KEYBOARD_OTHER_FONT_COLOR_PRESS);
    enterBtn->setIcon(QIcon(":/images/images/enter.svg"));
    connect(enterBtn, &KBButton::clicked, this, &CharsMoreWidget::onBtnClicked);
    m_mapBtnGeometrys[enterBtn] = enterBtn->geometry();
    // return
    KBButton *returnBtn = new KBButton(this);
    returnBtn->setGeometry(KEYBOARD_FIXED_DEFAULT_CHARSMORE_L1+(KEYBOARD_FIXED_DEFAULT_CHARSMORE_LIST_WIDTH+KEYBOARD_FIXED_DEFAULT_CHARSMORE_HSPACING),
                           (KEYBOARD_FIXED_DEFAULT_CHARSMORE_BTN_HEIGHT+KEYBOARD_FIXED_DEFAULT_CHARSMORE_VSPACING)*2,
                           KEYBOARD_FIXED_DEFAULT_CHARSMORE_BTN_WIDTH,
                           KEYBOARD_FIXED_DEFAULT_CHARSMORE_BTN_HEIGHT);
    returnBtn->setObjectName("btn_return");
    returnBtn->setText(tr("&&?!"));
    returnBtn->updateStyleSheet(KEYBOARD_OTHER_COLOR_NORMAL,KEYBOARD_OTHER_COLOR_PRESSED,
                                   KEYBOARD_OTHER_COLOR_PRESSED,KEYBOARD_OTHER_COLOR_BORDER_NORMAL,
                                   KEYBOARD_OTHER_COLOR_BORDER_PRESSED,KEYBOARD_OTHER_FONT_COLOR_PRESS);
    connect(returnBtn, &KBButton::clicked, this, &CharsMoreWidget::onBtnClicked);
    m_mapBtnGeometrys[returnBtn] = returnBtn->geometry();
}
