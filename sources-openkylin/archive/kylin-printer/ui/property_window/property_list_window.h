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
#ifndef __PROPERTY_LIST_WINDOW_H__
#define __PROPERTY_LIST_WINDOW_H__

#include <QWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include "property_list_widget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class PropertyListWindow; }
QT_END_NAMESPACE


class PropertyListWindow : public QWidget
{
    Q_OBJECT
public:
    enum PAGE{
        BASE_PAGE=0,
        ADVANCED_PAGE,
        INK_PAGE,
        KWRE_PAGE,
        PAGE_COUNT
    };
    explicit PropertyListWindow(QWidget *parent = nullptr);
    ~PropertyListWindow();
    QStackedWidget *getStackedWidget();
    ProperyListWidget *getListWidget(int index = 0);
    QLabel *getNameLabel();
    QPushButton *getAcceptButton();
    QPushButton *getRefreshButton();

signals:
    void saveRequest();
    void closeRequest();

private:
    Ui::PropertyListWindow *ui;
};

Q_DECLARE_METATYPE(PropertyListWindow::PAGE)



#endif // __PROPERTY_LIST_WINDOW_H__
