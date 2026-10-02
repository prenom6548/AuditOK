/*
 * kylin-os-manager
 *
 * Copyright (C) 2022, KylinSoft Co., Ltd.
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

#ifndef DETAILBUTTON_H
#define DETAILBUTTON_H

#include <QPushButton>
#include <QLabel>
#include <custom_label.h>

class DetailButton : public QPushButton
{
    Q_OBJECT
public:
    explicit DetailButton(QWidget *parent = nullptr);
    void setButtonStyle(bool isDown);
    void setSystemFontSizeChange(int size);

private:
    CustomLabel *mLabelText = nullptr;
    QLabel *mLabelIcon = nullptr;

signals:
};

#endif // DETAILBUTTON_H
