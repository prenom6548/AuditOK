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
#ifndef __CUSTOM_REFRESH_BUTTON_H__
#define __CUSTOM_REFRESH_BUTTON_H__

#include "custom_push_button.h"
#include <QLabel>
#include <QMovie>


class CustomRefreshButton : public CustomPushButton
{
    Q_OBJECT
public:
    CustomRefreshButton(QWidget *parent = nullptr);
    ~CustomRefreshButton();

    void setText(const QString &text);
    void refreshStart();
    void refreshFinish();
private:
    QLabel *m_label;

    QString m_showText{QString()};
};
#endif // __CUSTOM_REFRESH_BUTTON_H__