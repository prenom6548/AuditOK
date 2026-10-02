/* Copyright (C) 2020, KylinSoft Co., Ltd.
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
#ifndef EDIT_H
#define EDIT_H
#include <QLineEdit>
#include <QKeyEvent>
#include <QDebug>
class Edit : public QLineEdit
{
    Q_OBJECT
public:
    explicit Edit(QWidget *parent = 0);

public:
    QString Text;

private:
    bool m_rightButtonPress = false;

    void focusOutEvent(QFocusEvent *event);
    void keyPressEvent(QKeyEvent *event);

    bool event(QEvent *event);
Q_SIGNALS:
    void renamefinished();
    void showOrigName();
    void dealDoubleClicked(bool isleaveOrNot);
};

#endif // BUTTON_H
