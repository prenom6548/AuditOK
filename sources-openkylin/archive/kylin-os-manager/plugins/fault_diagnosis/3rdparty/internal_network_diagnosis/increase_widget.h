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

#ifndef INCREASE_WIDGET_H
#define INCREASE_WIDGET_H

#include <QWidget>
#include "ipweb_widget.h"

namespace Ui {

class IncreaseWidget;

}

class IncreaseWidget : public QWidget {
    Q_OBJECT

public:
    explicit IncreaseWidget(QWidget *parent = nullptr);
    ~IncreaseWidget();

    void setItemNums(int, const QList<IpWeb> &ip_web_items, IPWebWidgetType type);
    void showListWidget(bool, IPWebWidgetType type);
    int getWidgetItemNums();
    QString getAllSettings();
    bool getAllFormatStatus();

signals:
    void addWinSize();
    void minWinSize();
    void changedEvent();

public slots:
    void addNewWidget();
    void delOneWidget();

private:
    void initUI();

    Ui::IncreaseWidget *ui;
    IPWebWidgetType m_curType;
    QVector<IPWebWidget *> m_WidgetVec;
    QList<QString> m_listStr;
    QList<IpWeb> ip_web_items_;
};

#endif
