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

#ifndef APPITEM_H_
#define APPITEM_H_

#include <QPushButton>
#include <QLabel>
#include <QWidget>

#include "utils.h"

class AppItem : public QWidget
{
    Q_OBJECT

public:
    AppItem(QWidget *parent = nullptr);
    ~AppItem();

    void setIcon(QIcon icon);
    void setText(QString str);
    void setDescribe(QString descri);
    void setStayHope(void);

private:
    void initGsettings(void);
    void setStyle();

    // QLabel* m_icon = nullptr;
    QPushButton *m_icon = nullptr;
    QLabel *m_text = nullptr;
    QLabel *m_describe = nullptr;
    Theme m_theme;
    double m_fontSize;
};

#endif
