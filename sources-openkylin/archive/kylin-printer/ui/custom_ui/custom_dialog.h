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
#ifndef CUSTOM_DIALOG_H
#define CUSTOM_DIALOG_H

#ifdef USE_SELF_CONTAINED_DEP
    #include "kdialog.h"
#else
    #include <kysdk/applications/kdialog.h>
#endif // USE_SELF_CONTAINED_DEP

class CustomDialog : public kdk::KDialog
{
    Q_OBJECT
private:
    bool m_isSubWindow;
    QWidget *m_parentWindow;
public:
    explicit CustomDialog(QWidget *parent = nullptr, bool isSubWindow = false);
    ~CustomDialog();
    void setWindowTitle(const QString& widgetName);
public Q_SLOTS:
    void show();
    int exec();
};

#endif // CUSTOM_DIALOG_H
