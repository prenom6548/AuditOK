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
#include "edit.h"

Edit::Edit(QWidget *parent) : QLineEdit(parent)
{
    this->setObjectName("Kylin-photo-viewer_QLineEdit_rename");
    this->setAccessibleName("Kylin-photo-viewer_QLineEdit_rename");
    this->setAccessibleDescription("rename edit");
}

void Edit::focusOutEvent(QFocusEvent *event)
{
    if(m_rightButtonPress){
        return;
    }
    Text = this->text();
    clearFocus();
    close();
    Q_EMIT renamefinished();
    QLineEdit::focusOutEvent(event);
}
void Edit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        Text = this->text();
        close();
//        Q_EMIT renamefinished();
    }
    QLineEdit::keyPressEvent(event);
}

bool Edit::event(QEvent *event)
{
    if (event->type() == QEvent::HoverLeave) {
        Q_EMIT dealDoubleClicked(false);
    } else if (event->type() == QEvent::HoverEnter) {
        Q_EMIT dealDoubleClicked(true);
    }
    if(event->type() == QEvent::MouseButtonPress){
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::RightButton) {
            m_rightButtonPress = true;
        }
    } else if (event->type() == QEvent::ChildRemoved){
        m_rightButtonPress = false;
    }
    return QLineEdit::event(event);
}
