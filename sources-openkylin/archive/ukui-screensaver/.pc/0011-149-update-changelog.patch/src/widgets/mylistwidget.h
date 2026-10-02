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
#ifndef MYLISTWIDGET_H
#define MYLISTWIDGET_H
#include <QListWidget>
#include <QObject>

class MyListWidget : public QListWidget
{
    Q_OBJECT
public:
    MyListWidget(QWidget *parent = nullptr);
    void setUserListWidget(bool isUserListWidget);
    inline void setMaxHeight(int nMaxHeight)
    {
        m_nMaxHeight = nMaxHeight;
    }
    inline int maxHeight()
    {
        return m_nMaxHeight;
    }

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void changeEvent(QEvent *event);

private:
    void refreshTranslate();

private:
    //  标志位，记录前一个选中的item
    int m_lastRow = -1;
    bool m_isUserListWidget = false;
    int m_nMaxHeight = -1;
};

#endif // MYLISTWIDGET_H
