/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#include "mylistwidget.h"
#include <QKeyEvent>
#include <QDebug>
#include <powerlistwidget.h>

MyListWidget::MyListWidget(QWidget *parent) : QListWidget(parent)
{
    QLocale local;
    QString systemLang = local.name();
    if (systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ") {
        setLayoutDirection(Qt::RightToLeft);
    } else {
        setLayoutDirection(Qt::LeftToRight);
    }
    installEventFilter(this);
    setFocusPolicy(Qt::TabFocus);
}

void MyListWidget::setUserListWidget(bool isUserListWidget)
{
    m_isUserListWidget = isUserListWidget;
}

void MyListWidget::setIsLoginOptionWidget(bool isLoginOption)
{
    m_isLoginOption = isLoginOption;
}

bool MyListWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == this && event->type() == QEvent::Show) {
        if (!qobject_cast<PowerListWidget*>(this)) {
            m_lastRow = -1;//从头开始
            m_hasFocus = false;

            // 清除所有选择
            this->clearSelection();
            this->clearFocus();
            this->setCurrentRow(-1);
        } else {
            if (this->count() > 0) {
                m_lastRow = 0;
            }
        }
    }

    if (this->flow() == Flow::TopToBottom) {
        /*在收到快捷键按下后，首先会收到keypress事件，随后会收到一个ShortcutOverride的快捷键响应事件，
         * 收到这个事件后会把listWidget的currentRow往后移一个，随后才会收到keyrelease事件。*/
        if (event->type() == QEvent::KeyPress) { //先记录按键按下时的currentRow
            m_lastRow = this->currentRow();
        }
        if (event->type() == QEvent::KeyRelease) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            switch (keyEvent->key()) {
                case Qt::Key_Up:
                case Qt::Key_PageUp:
                    if (this->currentRow() == 0
                        && m_lastRow
                               == 0) /*收到按键释放信号时，判断当前currentRow与按键按下时记录的currentRow是否一致*/
                        this->setCurrentRow(this->count() - 1);
                    break;
                case Qt::Key_Down:
                case Qt::Key_PageDown:
                    if (this->currentRow() == this->count() - 1 && m_lastRow == this->count() - 1)
                        this->setCurrentRow(0);
                    break;
                case Qt::Key_HomePage:
                    this->setCurrentRow(0);
                    break;
                case Qt::Key_End:
                    this->setCurrentRow(this->count() - 1);

                    break;
                case Qt::Key_Enter:
                case Qt::Key_Return:
                    Q_EMIT itemClicked(currentItem());
                default:
                    break;
            }
        }
    } else {
        /*在收到快捷键按下后，首先会收到keypress事件，随后会收到一个ShortcutOverride的快捷键响应事件，
         * 收到这个事件后会把listWidget的currentRow往后移一个，随后才会收到keyrelease事件。*/
        if (event->type() == QEvent::KeyPress) { // 自定义按键选中item
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            if (keyEvent && (keyEvent->key() == Qt::Key_Left || keyEvent->key() == Qt::Key_Right
                             || keyEvent->key() == Qt::Key_Home || keyEvent->key() == Qt::Key_End)) {
                return true;
            }
        } else if (event->type() == QEvent::KeyRelease) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            switch (keyEvent->key()) {
                case Qt::Key_Left: {
                        if (layoutDirection() == Qt::RightToLeft) {
                            if (m_lastRow == -1) {
                                for (int i = 0; i < count(); i++) {
                                    if (!isRowHidden(i)) {
                                        setCurrentRow(i);
                                        m_lastRow = i;
                                        break;
                                    }
                                }
                            } else {
                                for (int i = 0; i < count() - 1; i ++) {
                                    m_lastRow = m_lastRow + 1;
                                    m_lastRow = (m_lastRow > (count() - 1)) ? 0 : m_lastRow;
                                    if (!isRowHidden(m_lastRow)) {
                                        setCurrentRow(m_lastRow);
                                        break;
                                    }
                                }
                            }
                        } else {
                            if (m_lastRow == -1) {
                                for (int i = count() - 1; i >= 0; i--) {
                                    if (!isRowHidden(i)) {
                                        setCurrentRow(i);
                                        m_lastRow = i;
                                        break;
                                    }
                                }
                            } else {
                                for (int i = 0; i < count() - 1; i ++) {
                                    m_lastRow = m_lastRow - 1;
                                    m_lastRow = (m_lastRow < 0) ? (count() - 1) : m_lastRow;
                                    if (!isRowHidden(m_lastRow)) {
                                        setCurrentRow(m_lastRow);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    return true;
                case Qt::Key_Right:{
                        if (layoutDirection() == Qt::RightToLeft) {
                            if (m_lastRow == -1) {
                                for (int i = count() - 1; i >= 0; i--) {
                                    if (!isRowHidden(i)) {
                                        setCurrentRow(i);
                                        m_lastRow = i;
                                        break;
                                    }
                                }
                            } else {
                                for (int i = 0; i < count() - 1; i ++) {
                                    m_lastRow = m_lastRow - 1;
                                    m_lastRow = (m_lastRow < 0) ? (count() - 1) : m_lastRow;
                                    if (!isRowHidden(m_lastRow)) {
                                        setCurrentRow(m_lastRow);
                                        break;
                                    }
                                }
                            }
                        } else {
                            if (m_lastRow == -1) {
                                // 初始状态：选择第一个可见项（向右的起点）
                                for (int i = 0; i < count(); i++) {
                                    if (!isRowHidden(i)) {
                                        setCurrentRow(i);
                                        m_lastRow = i;
                                        break;
                                    }
                                }
                            } else {
                                for (int i = 0; i < count() - 1; i ++) {
                                    m_lastRow = m_lastRow + 1;
                                    m_lastRow = (m_lastRow > (count() - 1)) ? 0 : m_lastRow;
                                    if (!isRowHidden(m_lastRow)) {
                                        setCurrentRow(m_lastRow);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    return true;
                case Qt::Key_Home: {
                        if (layoutDirection() == Qt::RightToLeft) {
                            for (int i = count() - 1; i >= 0; i --) {
                                if (!isRowHidden(i)) {
                                    setCurrentRow(i);
                                    m_lastRow = i;
                                    break;
                                }
                            }
                        } else {
                            for (int i = 0; i < count() - 1; i ++) {
                                if (!isRowHidden(i)) {
                                    setCurrentRow(i);
                                    m_lastRow = i;
                                    break;
                                }
                            }
                        }
                    }
                    return true;
                case Qt::Key_End: {
                        if (layoutDirection() == Qt::RightToLeft) {
                            for (int i = 0; i < count() - 1; i ++) {
                                if (!isRowHidden(i)) {
                                    setCurrentRow(i);
                                    m_lastRow = i;
                                    break;
                                }
                            }
                        } else {
                            for (int i = count() - 1; i >= 0; i --) {
                                if (!isRowHidden(i)) {
                                    setCurrentRow(i);
                                    m_lastRow = i;
                                    break;
                                }
                            }
                        }
                    }
                    break;
                case Qt::Key_Enter:
                case Qt::Key_Return:
                    Q_EMIT itemClicked(currentItem());
                    break;
                case Qt::Key_Tab:
                case Qt::Key_Backtab:
                    {
                        if (!qobject_cast<PowerListWidget*>(this)) {
                            if (layoutDirection() == Qt::RightToLeft) {
                                for (int i = count() - 1; i >= 0; i--) {
                                    if (!isRowHidden(i)) {
                                        setCurrentRow(i);
                                        m_lastRow = i;
                                        break;
                                    }
                                }
                            } else {
                                for (int i = 0; i <= count() - 1; i++) {
                                    if (!isRowHidden(i)) {
                                        setCurrentRow(i);
                                        m_lastRow = i;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }
    if (event->type() == QEvent::FocusOut) { //焦点丢失，记录当前焦点所在item，清空选中状态
        if (m_lastRow != -1) {
            if (!m_isUserListWidget) {
                for (int i = 0; i < this->count(); i++) {
                    if (!this->isRowHidden(i)) {
                        this->setCurrentRow(i, QItemSelectionModel::SelectionFlag::Clear);
                        //焦点丢失时不会主动发送当前item改变的信号，需要手动发送
                        Q_EMIT currentItemChanged(this->currentItem(), this->item(i));
                        break;
                    }
                }
            } else {
                this->setCurrentRow(this->currentRow(), QItemSelectionModel::SelectionFlag::Clear);
                //焦点丢失时不会主动发送当前item改变的信号，需要手动发送
                Q_EMIT currentItemChanged(this->currentItem(), this->currentItem());
            }
        } else {
            PowerListWidget *powerListWidget = qobject_cast<PowerListWidget*>(this);
            if (powerListWidget) {
                this->setFocus();
            }
        }
    } else if (event->type() == QEvent::MouseButtonRelease) { //鼠标点击设置选中状态

    } else if (event->type() == QEvent::FocusIn) { //焦点进入设置选中状态
        PowerListWidget *powerListWidget = qobject_cast<PowerListWidget*>(this);
        if (powerListWidget) {
            setDefaultFocus();
        } else {
            if (obj == this) {
                if (!m_hasFocus) {
                    m_hasFocus = true;
                    m_lastRow = -1;//从头开始
                }
            }
            return true;
        }
    }
    return QListWidget::eventFilter(obj, event);
}

void MyListWidget::resetSelectionState()
{
    m_lastRow = -1;
    clearSelection();
    setCurrentRow(-1);
}


void MyListWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        refreshTranslate();
    }
}

void MyListWidget::refreshTranslate()
{
    QLocale local;
    QString systemLang = local.name();
    if (systemLang == "ug_CN" || systemLang == "ky_KG" || systemLang == "kk_KZ") {
        setLayoutDirection(Qt::RightToLeft);
    } else {
        setLayoutDirection(Qt::LeftToRight);
    }
}

void MyListWidget::setDefaultFocus()
{
}