/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 * Zero Liu    liuzenghui1@kylinos.cn
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

//
// Created by sqp on 2021/11/17.
//

#ifndef KYLIN_MOBILE_ASSISTANT_TIPWIDGET_H
#define KYLIN_MOBILE_ASSISTANT_TIPWIDGET_H


#include <QLabel>

class TipWidget : public QLabel{
    Q_OBJECT

public:
    static TipWidget &Instance();
    void SetMesseage(const QString &strMessage, const QPoint *pPoint=nullptr);

private:
    TipWidget();
    ~TipWidget();

    void OnTimer();
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);

private:
    QWidget *mpParent;
    QTimer  *mpTimer;
    bool     mbEnter;
    int      mnTransparent;

};


#endif //KYLIN_MOBILE_ASSISTANT_TIPWIDGET_H
