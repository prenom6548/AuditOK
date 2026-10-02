/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#ifndef STOPWATCH_ITEM_H
#define STOPWATCH_ITEM_H

#include <QWidget>
#include <QLabel>
#include <QGSettings>
#include "constant_class.h"
#include "gsettingsubject.h"


namespace Ui {
class stopwatch_item;
}

class stopwatch_item : public QWidget
{
    Q_OBJECT

public:
    explicit stopwatch_item(QWidget *parent = nullptr);
    ~stopwatch_item();
    void setupUi(QWidget *stopwatch_item);
    void retranslateUi(QWidget *stopwatch_item);
    void  settingsStyle();
    void  blackStyle();                                                                  //黑色主题
    void  whiteStyle();                                                                  //白色主题

    QLabel *stopwatch1;
    QLabel *stopwatch2;
    QLabel *stopwatch3;
    QLabel *tipLabel;

    QFrame *stopwatch_line;
    void updateTipCommonStyle();
    void updateTipLongestStyle();
    void updateTipShortestStyle();

private:
    Ui::stopwatch_item *ui;
};

#endif // STOPWATCH_ITEM_H
