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
#ifndef MULTITIFFTOOLWID_H
#define MULTITIFFTOOLWID_H

#include <QObject>
#include <QWidget>
#include <QPainter>
#include <QStyleOption>
class MultiTiffToolWid : public QWidget
{
    Q_OBJECT
public:
    explicit MultiTiffToolWid(QWidget *parent = nullptr);
    void themeStyle(QString theme);

private:
    void paintEvent(QPaintEvent *event);
    QColor m_backgroundColor;
    QString m_theme = "";
};

#endif // MULTITIFFTOOLWID_H
