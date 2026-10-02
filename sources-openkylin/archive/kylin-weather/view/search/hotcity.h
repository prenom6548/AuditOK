/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
#ifndef HOTCITY_H
#define HOTCITY_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QDebug>
#include "hotcitybtn.h"
#include "global/viewvar.h"
class HotCity : public QWidget
{
    Q_OBJECT
    friend class TestHotCity;
public:
    explicit HotCity(QWidget *parent = nullptr);

private:
    QWidget *m_hotWid;
    QList<HotcityBtn *> m_list;
    QLabel *m_title;

    void initPostion();
    void initConnect();
    QMap<int, QStringList> m_btnList;

signals:
    void setHotCity(QString code); //
};

#endif // HOTCITY_H
