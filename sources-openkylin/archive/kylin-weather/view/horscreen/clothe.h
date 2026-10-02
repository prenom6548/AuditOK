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
#ifndef CLOTHE_H
#define CLOTHE_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include "global/corevar.h"
#include <QGSettings/QGSettings>
#include "global/viewvar.h"
class clothe : public QWidget
{
    Q_OBJECT
public:
    explicit clothe(QWidget *parent = nullptr);
    void setItemContent(QString text);
    void isFullScreen(int);
    int m_judgeFullScreen;

private:
    QLabel *m_clotheIcon;
    QLabel *m_str;
    QLabel *m_degree;
    QVBoxLayout *m_clotheLayout;
    QString clothepath = ":/res/contorl_icons/index_clothe.png";

    void initLayout();
    void initQss(double);
    void lable2SetFontSizeSlot(int size);
    QGSettings *m_fontSize = nullptr;

signals:
    void setFontSizeSignal(int);
};

#endif // CLOTHE_H
