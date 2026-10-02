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
#ifndef __CUSTOM_MESSAGE_BOX_H__
#define __CUSTOM_MESSAGE_BOX_H__

#ifdef USE_SELF_CONTAINED_DEP
    #include "kmessagebox.h"
#else
    #include <kysdk/applications/kmessagebox.h>
#endif // USE_SELF_CONTAINED_DEP

#include <QPushButton>

class CustomMessageBox : public kdk::KMessageBox
{
    Q_OBJECT
public:

    explicit CustomMessageBox(QWidget *parent = nullptr);
    explicit CustomMessageBox(Icon icon, const QString &title, const QString &text,
                StandardButtons buttons = NoButton, QWidget *parent = nullptr);
    explicit CustomMessageBox(QIcon icon, const QString &title, const QString &text,
                              StandardButtons buttons = NoButton, QWidget *parent = nullptr);
    QPushButton *addButton(const QString &text, ButtonRole role);
    QPushButton *clickedButton();
public Q_SLOTS:
    void show();
    int exec();

private:
    int m_ret{-1};
    QPushButton *m_yesButton{nullptr};
    QPushButton *m_noButton{nullptr};
};
#endif // __CUSTOM_MESSAGE_BOX_H__
