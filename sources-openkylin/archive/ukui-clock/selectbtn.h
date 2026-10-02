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
#ifndef SELECTBTN_H
#define SELECTBTN_H

#include <QObject>
#include <QWidget>
#include <QToolButton>
#include <QLabel>
#include <QPushButton>

namespace Ui {
class Btn_new;
}
class Clock;

class SelectBtn : public QPushButton
{
    Q_OBJECT
public:
    explicit SelectBtn(QString name,QWidget *parent = nullptr);
    ~SelectBtn();
    QLabel *textLabel;
    QLabel *noName;
    QLabel *IconLabel;
    void paintEvent(QPaintEvent *event);
    void updateIconLabel(int status);
private:
    Ui::Btn_new *ui;
    Clock * m_pclock;
    int clock_num;
    int pressflag;

protected:
signals:

};

#endif // SELECTBTN_H
