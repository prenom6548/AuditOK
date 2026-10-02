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

#ifndef TOOLBOXWIDGET_H
#define TOOLBOXWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QLabel>

#include "utils.h"
#include "appitem.h"

class ToolBoxWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ToolBoxWidget(QWidget *parent = nullptr);
    ~ToolBoxWidget();

private:
    void initWidget(void);
    void initAppWidget(void);
    void initGsettings(void);
    void createItem(const boxconfig_t &config);
    void createStayHopeItem(void);
    void getNameComment(const boxconfig_t &config, QString &name, QString &comment);
    void getTheme(void);
    void changeAppWidgetStyle(void);

    QLabel *m_title = nullptr;
    QLabel *m_describe = nullptr;
    QListWidget *m_appWidget = nullptr;

    Theme m_theme;
    double m_fontSize;

private slots:
    void startApp(QListWidgetItem *item);
};

#endif // TOOLBOXWIDGET_H
