/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Alan Xie    xiehuijun@kylinos.cn
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

#include "singlekey.h"
#include "kmrewindow.h"
#include "utils.h"
#include "preferences.h"
#include "gamekeymanager.h"
#include "widgets/messagebox.h"
#include "displaymanager.h"
#include "sessionsettings.h"

#include <sys/syslog.h>
#include <QPainter>
#include <QEvent>
#include <QKeyEvent>

#define DEFAULT_SINGLE_KEY_SHORT_WIDTH 50
#define DEFAULT_SINGLE_KEY_LONG_WIDTH  100
#define DEFAULT_SINGLE_KEY_HEIGHT 50
#define DEFAULT_LINE_EDIT_HEIGHT 18

SingleKey::SingleKey(int idx, KmreWindow* window) 
    : BaseKey(window)
    , m_index(idx)
    , m_keyString("")
    , m_storedKeyString("")
    , m_keyModifier(Qt::NoModifier)
    , m_isLongWidthCurrently(false)
    , m_isWidthSwitched(false)
{
    int displayWidth, displayHeight, initialWidth, initialHeight;
    mMainWindow->getDisplayManager()->getMainWidgetSize(displayWidth, displayHeight, initialWidth, initialHeight);
    //syslog(LOG_DEBUG, "[SingleKey][%s] displayWidth = %d, displayHeight = %d, initialWidth = %d, initialHeight = %d", 
    //        __func__, displayWidth, displayHeight, initialWidth, initialHeight);

    m_short_width_scale_ratio = DEFAULT_SINGLE_KEY_SHORT_WIDTH / static_cast<double>(initialWidth);
    m_long_width_scale_ratio = DEFAULT_SINGLE_KEY_LONG_WIDTH / static_cast<double>(initialWidth);
    m_height_scale_ratio = DEFAULT_SINGLE_KEY_HEIGHT / static_cast<double>(initialHeight);
    this->resize(m_short_width_scale_ratio * displayWidth, m_height_scale_ratio * displayHeight);

    m_bgBtn = new QPushButton(this);
    m_bgBtn->resize(this->width(), this->height());
    m_bgBtn->setStyleSheet("QPushButton{background:transparent; border-image: url(:/res/key_hover.png);}"
                           "QPushButton:hover{background:transparent; border-image:url(:/res/key_hover.png);}"
                           "QPushButton:focus{outline: none;}");
    m_bgBtn->setMouseTracking(true);
    m_bgBtn->installEventFilter(this);

    m_inputEdit = new QLineEdit(this);
    m_inputEdit->setFocusPolicy(Qt::ClickFocus);
    int inputEditWidth = this->width() - 8;
    m_inputEdit->setGeometry((this->width() - inputEditWidth) / 2, 
                             (this->height() - DEFAULT_LINE_EDIT_HEIGHT) / 2 - 2, 
                             inputEditWidth, 
                             DEFAULT_LINE_EDIT_HEIGHT);
    m_inputEdit->setAlignment(Qt::AlignHCenter);
    m_inputEdit->setStyleSheet("QLineEdit{background:transparent;border:none;color:rgba(255,255,255,1);}");

    m_closeBtn = new QPushButton(this);
    m_closeBtn->setFixedSize(20, 20);
    m_closeBtn->setStyleSheet("QPushButton{background:transparent;background-image:url(:/res/key_close_normal.png);}"
                              "QPushButton:hover{background:transparent;background-image:url(:/res/key_close_hover.png);}");
    m_closeBtn->move(this->width() - m_closeBtn->width(), 0);
    connect(m_closeBtn, &QPushButton::clicked, this, [this] {
        emit deleteGameKey(m_index);
    });

    m_overlayBtn = new OverlayButton(mMainWindow, this);
    m_overlayBtn->resize(this->width(), this->height());
    m_overlayBtn->setVisible(false);

    this->installEventFilter(this);
    m_inputEdit->installEventFilter(this);
}

SingleKey::~SingleKey()
{
    delete m_bgBtn;
    delete m_overlayBtn;
    delete m_inputEdit;
    delete m_closeBtn;
}

QPoint SingleKey::getCoordnate(QPoint pos)
{
    return QPoint(pos.x() - width() / 2, pos.y() - height()/ 2);
}

double SingleKey::getWidthScaleRatio()
{
    return m_isLongWidthCurrently ? m_long_width_scale_ratio : m_short_width_scale_ratio;
}

void SingleKey::updateSize(bool update_panel)
{
    Q_UNUSED(update_panel);
    QSize displaySize = mMainWindow->getDisplayManager()->getMainWidgetDisplaySize();
    this->resize(getWidthScaleRatio() * displaySize.width(), m_height_scale_ratio * displaySize.height());

    m_bgBtn->resize(this->width(), this->height());
    m_overlayBtn->resize(this->width(), this->height());
    int inputEditWidth = this->width() - 8;
    m_inputEdit->setGeometry((this->width() - inputEditWidth) / 2, 
                             (this->height() - DEFAULT_LINE_EDIT_HEIGHT) / 2 - 2, 
                             inputEditWidth, 
                             DEFAULT_LINE_EDIT_HEIGHT);
    m_closeBtn->move(this->width() - m_closeBtn->width(), 0);
}

void SingleKey::updatePos()
{
    QSize displaySize = mMainWindow->getDisplayManager()->getMainWidgetDisplaySize();
    int xpos = displaySize.width() * event_coordinate_x;
    int ypos = displaySize.height() * event_coordinate_y;
    QPoint pos = mMainWindow->getGameKeyManager()->getGlobalPos(QPoint(xpos, ypos));
    QPoint coordnate = getCoordnate(pos);
    move(coordnate);
    checkPosOutsideScreen(coordnate);
}

void SingleKey::tempUpdate()
{
    updateSize();
    if (m_isWidthSwitched) {
        m_isWidthSwitched = false;
        QPoint pos = this->pos();
        if (m_isLongWidthCurrently) {
            move(pos.x() - (DEFAULT_SINGLE_KEY_LONG_WIDTH - DEFAULT_SINGLE_KEY_SHORT_WIDTH) / 2, pos.y());
        }
        else {
            move(pos.x() + (DEFAULT_SINGLE_KEY_LONG_WIDTH - DEFAULT_SINGLE_KEY_SHORT_WIDTH) / 2, pos.y());
        }
    }
}

void SingleKey::setEventXY(double x, double y)
{
    event_coordinate_x = x;
    event_coordinate_y = y;
}

void SingleKey::getEventXY(double &x, double &y)
{
    x = event_coordinate_x;
    y = event_coordinate_y;
}

void SingleKey::enableEdit(bool enable)
{
    m_bgBtn->setStyleSheet(QString("QPushButton{background:transparent; border-image: url(:/res/%1);}"
                           "QPushButton:hover{background:transparent; border-image:url(:/res/%1);}"
                           "QPushButton:focus{outline: none;}")
                           .arg(enable ? "key_hover.png" : "key_normal.png"));
    m_inputEdit->setEnabled(enable);
    m_inputEdit->setSelection(m_keyString.length(), m_keyString.length());// 取消文本选中状态
    m_closeBtn->setVisible(enable);
    m_overlayBtn->setVisible(!enable);// 覆盖 m_inputEdit，防止按键被鼠标拖动
    m_enableEdit = enable;
}

void SingleKey::setKeyString(QString character)
{
    m_keyString = character;
    bool oldWidthFlg = m_isLongWidthCurrently;
    m_isLongWidthCurrently = (m_keyString.length() > 3);
    m_isWidthSwitched = (oldWidthFlg != m_isLongWidthCurrently);
    m_inputEdit->clear();
    m_inputEdit->setText(character);
}

QString SingleKey::composeKeyString(Qt::KeyboardModifiers modifier, QString key)
{
    QString strModifier = "";
    switch (modifier) {
        case Qt::ControlModifier:
            strModifier = "CTRL";
            break;
        case Qt::ShiftModifier:
            strModifier = "SHIFT";
            break;
        case Qt::AltModifier:
            strModifier = "ALT";
            break;
        default:
            break;
    }

    if ((!strModifier.isEmpty()) && (!key.isEmpty())) {
        if (strModifier == key) {
            return key;
        }
        return strModifier + "+" + key;
    }
    return strModifier + key;
}

QString SingleKey::genKeyString(QKeyEvent *k)
{
    QString convertStr = convertNativeScanCodeToString(k->nativeScanCode());
    QString strKey = composeKeyString(k->modifiers(), convertStr);
    //syslog(LOG_DEBUG, "[SingleKey] genKeyString, modifiers: %d, scan code: %d, strKey: %s", 
    //                    k->modifiers(), k->nativeScanCode(), strKey.toStdString().c_str());
    return strKey;
}

bool SingleKey::eventFilter(QObject *obj, QEvent *event)
{
    //syslog(LOG_DEBUG, "[SingleKey] eventFilter, event type: %d", event->type());
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        m_lastPos = mouseEvent->pos();
        m_mousePressed = true;
    }
    else if (event->type() == QEvent::MouseMove && m_enableEdit && m_mousePressed) {
        QRect validRect = mMainWindow->getGameKeyManager()->getGameKeyValidRect();
        //计算新的移动后的位置
        QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
        QPoint movePoint;
        if (SessionSettings::getInstance().windowUsePlatformWayland()) {
            movePoint = mouseEvent->pos() - m_lastPos + QPoint(this->x(), this->y());
        } else {
            movePoint = mouseEvent->globalPos() - m_lastPos;
        }
        //设置可移动的X和Y的范围
        bool moveX = (movePoint.x() > validRect.x()) && (movePoint.x() < validRect.right() - this->width());
        bool moveY = (movePoint.y() > validRect.y()) && (movePoint.y() < validRect.bottom() - this->height());
        
        if (moveX && moveY) {//在X和Y的允许范围内移动
            this->move(movePoint);
        }
        else if (moveX) {//在X的允许范围内移动
            this->move(movePoint.x(), this->pos().y());
        }
        else if (moveY) {//在Y的允许范围内移动
            this->move(this->pos().x(), movePoint.y());
        }
    }
    else if (event->type() == QEvent::MouseButtonRelease) {
        m_mousePressed = false;
    }
    else if (event->type() == QEvent::KeyPress && obj == m_inputEdit) {
        QString strKey = genKeyString(static_cast<QKeyEvent *>(event));
        // 判断该按键是否已经设置过
        if ((m_storedKeyString != strKey) && (!strKey.isEmpty())) {
            //若界面(尚未保存到配置文件中)或配置文件中已经存在准备输入的按键，则无法再次输入
            if (mMainWindow->getGameKeyManager()->isGameKeyExist(strKey)) {
                KylinUI::MessageBox::warning(this->parentWidget(), tr("Warning"), tr("The Key is existing, please input another key!"));
                return true;
            }
        }
        //如果已经有walkwidget对象的图标了，说明AWSD四个按键已经设置了，此时不再单独设置这四个按键
        if (strKey == "A" || strKey == "W" || strKey == "S" || strKey == "D") {
            if (mMainWindow->getGameKeyManager()->hasSteelingWheel()) {
                KylinUI::MessageBox::warning(this->parentWidget(), tr("Warning"), tr("The Key is existing, please input another key!"));
                return true;
            }
        }

        setKeyString(strKey);
        tempUpdate();

        return true;
    }

    return QWidget::eventFilter(obj, event);
}

QString SingleKey::convertNativeScanCodeToString(quint32 keyCode)
{
    switch (keyCode) {
    case 65:
        return "SP";
        break;
    case 24:
        return "Q";
        break;
    case 25:
        return "W";
        break;
    case 26:
        return "E";
        break;
    case 27:
        return "R";
        break;
    case 28:
        return "T";
        break;
    case 29:
        return "Y";
        break;
    case 30:
        return "U";
        break;
    case 31:
        return "I";
        break;
    case 32:
        return "O";
        break;
    case 33:
        return "P";
        break;
    case 38:
        return "A";
        break;
    case 39:
        return "S";
        break;
    case 40:
        return "D";
        break;
    case 41:
        return "F";
        break;
    case 42:
        return "G";
        break;
    case 43:
        return "H";
        break;
    case 44:
        return "J";
        break;
    case 45:
        return "K";
        break;
    case 46:
        return "L";
        break;
    case 52:
        return "Z";
        break;
    case 53:
        return "X";
        break;
    case 54:
        return "C";
        break;
    case 55:
        return "V";
        break;
    case 56:
        return "B";
        break;
    case 57:
        return "N";
        break;
    case 58:
        return "M";
        break;
    case 10:
        return "1";
        break;
    case 11:
        return "2";
        break;
    case 12:
        return "3";
        break;
    case 13:
        return "4";
        break;
    case 14:
        return "5";
        break;
    case 15:
        return "6";
        break;
    case 16:
        return "7";
        break;
    case 17:
        return "8";
        break;
    case 18:
        return "9";
        break;
    case 19:
        return "0";
        break;
    case 87:
        return "N1";
        break;
    case 88:
        return "N2";
        break;
    case 89:
        return "N3";
        break;
    case 83:
        return "N4";
        break;
    case 84:
        return "N5";
        break;
    case 85:
        return "N6";
        break;
    case 79:
        return "N7";
        break;
    case 80:
        return "N8";
        break;
    case 81:
        return "N9";
        break;
    case 90:
        return "N0";
        break;
    case 111:
        return "↑";
        break;
    case 113:
        return "←";
        break;
    case 114:
        return "→";
        break;
    case 116:
        return "↓";
        break;
    case 50:
        return "SHIFT";
        break;
    case 37:
        return "CTRL";
        break;
    case 64:
        return "ALT";
        break;
    default:
        break;
    }
    return "";
}
