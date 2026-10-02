/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
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
#ifndef ADDBTN_H
#define ADDBTN_H

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QGSettings>
#include <QLabel>
#include <kysdk/applications/accessinfohelper.h>

#define THEME_QT_SCHEMA  "org.ukui.style"
#define MODE_QT_KEY      "style-name"

class EnrollBtn : public QPushButton
{
    Q_OBJECT
public:
    EnrollBtn(QWidget *parent = nullptr);
    void setName(const QString text);
    ~EnrollBtn();

protected:
    virtual void leaveEvent(QEvent * event);
    virtual void enterEvent(QEvent * event);

Q_SIGNALS:
    void enterWidget();
    void leaveWidget();

private:
    QLabel *textLabel = nullptr;
    QLabel *iconLabel = nullptr;
};

#endif // ADDBTN_H
