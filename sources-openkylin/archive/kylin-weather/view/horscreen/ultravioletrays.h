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
#ifndef ULTRAVIOLETRAYS_H
#define ULTRAVIOLETRAYS_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include "global/corevar.h"
#include <QGSettings/QGSettings>
class ultravioletrays : public QWidget
{
    Q_OBJECT
public:
    explicit ultravioletrays(QWidget *parent = nullptr);
    void setItemContent(QString text);
    void isFullScreen(int);
    int m_judgeFullScreen;

private:
    QVBoxLayout *m_uvLayout;
    QLabel *m_uvIcon;
    QLabel *m_str;
    QLabel *m_degree;
    QString uvpath = ":/res/contorl_icons/index_uv.png";

    void initLayout();
    void initQss(double);
    void lable2SetFontSizeSlot(int size);
    QGSettings *m_fontSize = nullptr;

signals:
};

#endif // ULTRAVIOLETRAYS_H
