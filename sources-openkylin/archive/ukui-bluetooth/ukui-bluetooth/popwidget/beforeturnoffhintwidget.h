/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
**/

#ifndef BEFORETURNOFFHINTWIDGET_H
#define BEFORETURNOFFHINTWIDGET_H

#include <QDialog>
#include <QLabel>
#include <QGSettings>

namespace Ui {
class beforeTurnOffHintWidget;
}

class BeforeTurnOffHintWidget : public QDialog
{
    Q_OBJECT

public:
    explicit BeforeTurnOffHintWidget(QWidget *parent = nullptr);
    ~BeforeTurnOffHintWidget();

    QString textDataStr();
    QString cancelStr();
    QString closeStr();
private:
    Ui::beforeTurnOffHintWidget *ui;
    QGSettings *styleGSettings;
    QLabel *textLabel;

private:
    void initConnect();

signals:
    void buttonPressed(bool confirm);
};

#endif // BEFORETURNOFFHINTWIDGET_H
