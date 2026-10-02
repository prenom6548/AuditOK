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
#ifndef MULTITIFFTOOLBTN_H
#define MULTITIFFTOOLBTN_H

#include <QObject>
#include <QWidget>
#include <kpushbutton.h>
#include <QPainter>
#include <QStyleOption>
#include <QMouseEvent>
class MultiTiffToolBtn : public QPushButton
{
    Q_OBJECT
public:
    explicit MultiTiffToolBtn(QString btnType, QWidget *parent = nullptr);
    void themeStyle(QString theme);

private:
    void paintEvent(QPaintEvent *event);
    QString m_theme = "";
    QString m_btnType = "";

Q_SIGNALS:
};

#endif // MULTITIFFTOOLBTN_H
