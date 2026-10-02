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
#ifndef FLU_H
#define FLU_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include "global/corevar.h"
#include <QGSettings/QGSettings>
class flu : public QWidget
{
    Q_OBJECT
public:
    explicit flu(QWidget *parent = nullptr);
    void setItemContent(QString text);
    void isFullScreen(int);
    int m_judgeFullScreen;

private:
    QVBoxLayout *m_fluLayout;
    QLabel *m_fluIcon;
    QLabel *m_str;
    QLabel *m_degree;
    QString flupath = ":/res/contorl_icons/index_flu.png";

    void initLayout();
    void initQss(double);
    void lable2SetFontSizeSlot(int size);
    QGSettings *m_fontSize = nullptr;

signals:
};

#endif // FLU_H
