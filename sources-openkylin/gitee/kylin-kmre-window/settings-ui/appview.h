/*
 * Copyright (C) 2013 ~ 2021 KylinSoft Ltd.
 *
 * Authors:
 *  Kobe Lee    lixiang@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _APP_VIEW_H
#define _APP_VIEW_H

#include "appitem.h"
#include <vector>
#include <QListWidget>


class AppView : public QListWidget
{
    Q_OBJECT

public:
    AppView(QWidget *parent = 0);
    ~AppView() {}

    void reloadItems(const std::vector<std::pair<QString, QString>> &appList, const QStringList& enableList);
    void loadItem(const QString &appName, const QString &pkgName, bool checked);
    void selectItemWithPkgName(QString pkgName);

public slots:
    void deleteItem();
    void handleEnter();
    void onRightClick(QPoint pos);

signals:
    void sigCheckedChanged(QString, bool);
    void rightClick(QPoint pos);
    void sigClear();

protected:
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

private:
    QListWidgetItem *m_rightSelectItem = nullptr;
};

#endif // _APP_VIEW_H
