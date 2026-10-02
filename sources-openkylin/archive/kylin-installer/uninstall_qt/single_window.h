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
#ifndef SINGLE_WINDOW_H
#define SINGLE_WINDOW_H

#include <QWidget>
#include <QMessageBox>

class single_window
{
public:
    explicit single_window();
    ~single_window();

    void create_interface(void);
    void init(void);

    QMessageBox *p_message_box;

signals:

};

#endif // SINGLE_WINDOW_H
