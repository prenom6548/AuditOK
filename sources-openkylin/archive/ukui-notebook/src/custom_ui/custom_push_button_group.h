/*
 * Copyright (C) 2020, KylinSoft Co., Ltd.
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

#ifndef __CUSTOM_PUSH_BUTTON_GROUP_H__
#define __CUSTOM_PUSH_BUTTON_GROUP_H__

#include <QFrame>
#include <QPushButton>

QT_BEGIN_NAMESPACE
namespace Ui { class CustomPushButtonGroup; }
QT_END_NAMESPACE

class CustomPushButtonGroup : public QFrame
{
    Q_OBJECT
public:
    enum BtnState {
        Left = 0,
        Right,
        Mid
    };
    CustomPushButtonGroup(QWidget *parent = nullptr);
    ~CustomPushButtonGroup();

    void loadQSS();
    void addButton(QPushButton *button, BtnState state);
    void addSeparator();
    void frameUpdate(bool isClassical);
private:
    Ui::CustomPushButtonGroup *ui;
    static QString m_buttonGroupQSS;
};
#endif // __CUSTOM_PUSH_BUTTON_GROUP_H__
