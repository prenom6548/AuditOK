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
/**
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
**/
#include "virtualkeyboardwidget.h"

#include <QPainterPath>
#include <QPainter>
#include <QMouseEvent>
#include "commondef.h"
#include <QDebug>
#include <QVBoxLayout>
#include "dragwidget.h"
#include "kbtitle.h"
#include "letterswidget.h"
#include "numberswidget.h"
#include "charswidget.h"
#include "charsmorewidget.h"

VirtualKeyboardWidget::VirtualKeyboardWidget(QWidget *parent)
    : QWidget(parent)
    , m_lfWidthScale(1.0)
    , m_lfHeightScale(1.0)
    , m_isVertical(false)
{
    Q_INIT_RESOURCE(keyboard);
    setAttribute(Qt::WA_TranslucentBackground);//背景透明
    //setAutoFillBackground(true);
    setWindowFlags(Qt::FramelessWindowHint |
                   Qt::WindowStaysOnTopHint |
                   Qt::WindowDoesNotAcceptFocus);

    vKeyboard = new X11Keyboard(this);
    connect(this, SIGNAL(keyPressed(QChar)),
            vKeyboard, SLOT(onKeyPressed(QChar)));
    connect(this, SIGNAL(keyPressed(FuncKey::FUNCKEY)),
            vKeyboard, SLOT(onKeyPressed(FuncKey::FUNCKEY)));
    initUI();
    initConnections();
}

VirtualKeyboardWidget::~VirtualKeyboardWidget()
{

}

void VirtualKeyboardWidget::initUI()
{
    QVBoxLayout *layoutMain = new QVBoxLayout(this);
    layoutMain->setContentsMargins(0,0,0,0);
    layoutMain->setSpacing(0);

    m_dragWidget = new DragWidget();
    layoutMain->addWidget(m_dragWidget);
    m_dragWidget->installEventFilter(this);

    m_kbTitle = new KBTitle();
    layoutMain->addWidget(m_kbTitle);

    m_stackedWidget = new QStackedWidget();

    m_lettersWidget = new LettersWidget();
    m_stackedWidget->addWidget(m_lettersWidget);

    m_numbersWidget = new NumbersWidget(this);
    m_stackedWidget->addWidget(m_numbersWidget);

    m_charsWidget = new CharsWidget();
    m_stackedWidget->addWidget(m_charsWidget);

    m_charsMoreWidget = new CharsMoreWidget();
    m_stackedWidget->addWidget(m_charsMoreWidget);

    m_stackedWidget->setCurrentIndex(VKB_PAGE_LETTERS);
    m_nCurPage = VKB_PAGE_CHARSMORE;
    layoutMain->addWidget(m_stackedWidget);
}

void VirtualKeyboardWidget::initConnections()
{
    connect(m_kbTitle, &KBTitle::btnClicked, this, &VirtualKeyboardWidget::onSpecialBtnClicked);
    connect(m_lettersWidget, &LettersWidget::specialBtnClicked, this, &VirtualKeyboardWidget::onSpecialBtnClicked);
    connect(m_lettersWidget, &LettersWidget::normalBtnClicked, this, &VirtualKeyboardWidget::onNormalBtnClicked);
    connect(m_numbersWidget, &NumbersWidget::specialBtnClicked, this, &VirtualKeyboardWidget::onSpecialBtnClicked);
    connect(m_numbersWidget, &NumbersWidget::narmalBtnClicked, this, &VirtualKeyboardWidget::onNormalBtnClicked);
    connect(m_charsWidget, &CharsWidget::specialBtnClicked, this, &VirtualKeyboardWidget::onSpecialBtnClicked);
    connect(m_charsWidget, &CharsWidget::normalBtnClicked, this, &VirtualKeyboardWidget::onNormalBtnClicked);
    connect(m_charsMoreWidget, &CharsMoreWidget::specialBtnClicked, this, &VirtualKeyboardWidget::onSpecialBtnClicked);
    connect(m_charsMoreWidget, &CharsMoreWidget::normalBtnClicked, this, &VirtualKeyboardWidget::onNormalBtnClicked);
}

void VirtualKeyboardWidget::adjustGeometry()
{
    QWidget *parentWidget = qobject_cast<QWidget*>(parent());
    if (parentWidget) {
        //qDebug()<< "parent: " << parentWidget <<"Parent gemotry:"<<parentWidget->geometry();
        double lfWidth = parentWidget->geometry().width();
        double lfHeight = parentWidget->geometry().height();
        m_isVertical = lfHeight > lfWidth;
        m_lfWidthScale = lfWidth/KEYBOARD_PARENT_DEFAULT_WIDTH;
        if (m_isVertical)
            m_lfHeightScale = lfHeight / KEYBOARD_PARENT_DEFAULT_WIDTH;
        else
            m_lfHeightScale = lfHeight / KEYBOARD_PARENT_DEFAULT_HEIGHT;
        if (m_isdragState) {
            lfWidth = m_lfWidthScale * KEYBOARD_DRAGSHOW_FIXED_DEFAULT_WIDTH;
            lfHeight = m_lfHeightScale * KEYBOARD_DRAGSHOW_FIXED_DEFAULT_HEIGHT;
            setGeometry(QRect(m_lfWidthScale * (KEYBOARD_PARENT_DEFAULT_WIDTH - KEYBOARD_DRAGSHOW_FIXED_DEFAULT_WIDTH) / 2,
                              parentWidget->geometry().height() - lfHeight,
                              lfWidth, lfHeight));
            m_dragWidget->show();
        } else {
            lfWidth = m_lfWidthScale * KEYBOARD_FIXED_DEFAULT_WIDTH;
            lfHeight = m_lfHeightScale * KEYBOARD_DRAGHIDE_FIXED_DEFAULT_HEIGHT;
            m_dragWidget->hide();
            setGeometry(QRect(0, parentWidget->geometry().height()-lfHeight, lfWidth, lfHeight));
        }

        //qDebug()<<"Widget geometry:"<<geometry();
    } else {
        if (m_isdragState) {
            setGeometry(QRect(0, 0,
                              KEYBOARD_DRAGSHOW_FIXED_DEFAULT_WIDTH, KEYBOARD_DRAGSHOW_FIXED_DEFAULT_HEIGHT));
            m_dragWidget->show();
        } else {
            setGeometry(QRect(0, 0,
                              KEYBOARD_FIXED_DEFAULT_WIDTH, KEYBOARD_DRAGHIDE_FIXED_DEFAULT_HEIGHT));
            m_dragWidget->hide();
        }
        m_lfWidthScale = 1.0;
        m_lfHeightScale = 1.0;
        m_isVertical = false;
    }
    if (m_dragWidget) {
        m_dragWidget->adjustGeometry(m_lfWidthScale, m_lfHeightScale, m_isVertical, m_isdragState);
    }
    if (m_kbTitle) {
        m_kbTitle->adjustGeometry(m_lfWidthScale, m_lfHeightScale, m_isVertical, m_isdragState);
    }
    if (m_lettersWidget) {
        m_lettersWidget->adjustGeometry(m_lfWidthScale, m_lfHeightScale, m_isVertical, m_isdragState);
    }
    if (m_numbersWidget) {
        m_numbersWidget->adjustGeometry(m_lfWidthScale, m_lfHeightScale, m_isVertical, m_isdragState);
    }
    if (m_charsWidget) {
        m_charsWidget->adjustGeometry(m_lfWidthScale, m_lfHeightScale, m_isVertical, m_isdragState);
    }
    if (m_charsMoreWidget) {
        m_charsMoreWidget->adjustGeometry(m_lfWidthScale, m_lfHeightScale, m_isVertical, m_isdragState);
    }
}

bool VirtualKeyboardWidget::eventFilter(QObject *watched, QEvent *event)
{
    if(watched != m_dragWidget && !isMove) return QWidget::eventFilter(watched, event);
    switch(event->type())
    {
    case QEvent::MouseButtonPress:
        onMouseEvents(1);
        return true;
    case QEvent::MouseMove:
        onMouseEvents(2);
        return true;
    case QEvent::MouseButtonRelease:
        onMouseEvents(3);
        return true;
    default:
        break;
    }
    return QWidget::eventFilter(watched, event);
}

void VirtualKeyboardWidget::resizeEvent(QResizeEvent *event)
{
    adjustGeometry();
}

void VirtualKeyboardWidget::paintEvent(QPaintEvent *event)
{
    QPainterPath path;
    QPainter painter(this);
    painter.setOpacity(1.0);
    painter.setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter.setClipping(true);
    painter.setPen(Qt::transparent);
    if (m_isdragState) {
        path.addRoundedRect(this->rect(), 16, 16);
    } else {
        path.addRoundedRect(this->rect(), 0, 0);
    }
    path.setFillRule(Qt::WindingFill);
    painter.setBrush(QColor("#EBEDEF"));
    painter.setPen(Qt::transparent);
    painter.drawPath(path);
    QWidget::paintEvent(event);
}

void VirtualKeyboardWidget::onSpecialBtnClicked(QString keyName)
{
    Modifier::MOD mod = Modifier::getModifier(keyName);
    FuncKey::FUNCKEY funcKey = FuncKey::getKey(keyName);
    if (mod != Modifier::UNKNOWN) {
        if(vKeyboard->hasModifier(mod)) {
            vKeyboard->removeModifier(mod);
            m_lettersWidget->changeFuncKeyStyle(keyName, false);
        } else {
            vKeyboard->addModifier(mod);
            m_lettersWidget->changeFuncKeyStyle(keyName, true);
        }
        if (keyName == BTN_CAPSLOCK) {
            Q_EMIT keyPressed(FuncKey::UNKNOWN);
            clearModifier();
        }
    } else if(funcKey != FuncKey::UNKNOWN) {
        Q_EMIT keyPressed(funcKey);
    } else if (keyName == PAGE_CHARSMORE) {
        m_stackedWidget->setCurrentIndex(VKB_PAGE_CHARSMORE);
        update();
    } else if (keyName == PAGE_CHAR) {
        m_stackedWidget->setCurrentIndex(VKB_PAGE_CHARS);
        update();
    } else if (keyName == PAGE_NUMBER) {
        m_stackedWidget->setCurrentIndex(VKB_PAGE_NUMBERS);
        update();
    } else if (keyName == PAGE_LETTER) {
        m_stackedWidget->setCurrentIndex(VKB_PAGE_LETTERS);
        update();
    } else if (keyName == BTN_FLOAT) {
        m_isdragState = !m_isdragState;
        adjustGeometry();
        Q_EMIT aboutToFloat();
    } else if (keyName == BTN_CLOSE) {
        Q_EMIT aboutToClose();
    }
}

void VirtualKeyboardWidget::onNormalBtnClicked(QChar c)
{
    Q_EMIT keyPressed(c);
    clearModifier();
}

void VirtualKeyboardWidget::clearModifier()
{
    for(auto mod : vKeyboard->getAllModifier()) {
        QString modName = Modifier::getModifierName(mod);
        m_lettersWidget->changeFuncKeyStyle(modName, false);
    }
    vKeyboard->clearModifier();
}

void VirtualKeyboardWidget::onMouseEvents(int type)
{
    switch (type) {
    case 1:
    {
        isMove = true;
        lastPoint = QCursor::pos();
        break;
    }
    case 2:
    {
        if(isMove)
        {
            QPoint cPoint = QCursor::pos();
            QPoint p = pos();
            p.setX(p.x() - lastPoint.x() + cPoint.x());
            p.setY(p.y() - lastPoint.y() + cPoint.y());
            lastPoint = cPoint;
            move(p);
        }
        break;
    }
    case 3:
    {
        isMove = false;
        break;
    }
    default:
        break;
    }
}


bool VirtualKeyboardWidget::getFloatStatus()
{
    return m_isdragState;
}
