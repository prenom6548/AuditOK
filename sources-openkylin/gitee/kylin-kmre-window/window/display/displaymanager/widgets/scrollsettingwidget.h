/*
 * Copyright (C) 2013 ~ 2020 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
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

#ifndef SCROLLSETTINGWIDGET_H
#define SCROLLSETTINGWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QSlider>

class ScrollSettingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ScrollSettingWidget(QString pkgName, QWidget *parent = 0);

private:
    QLineEdit *m_line = nullptr;
    QSlider *m_slider = nullptr;
};

#endif // SCROLLSETTINGWIDGET_H
