/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Yuan ShanShan   yuanshanshan@kylinos.cn
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

#ifndef GPSWINDOW_H
#define GPSWINDOW_H
#ifndef UKUI_THEME_GSETTING_PATH
#define UKUI_THEME_GSETTING_PATH "org.ukui.style"
#endif
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLayoutItem>
#include <QDialog>
#include "searchview.h"
#include "currentlocationview.h"
using namespace kmre_gps;
class GPSWINDOW : public QDialog
{
    Q_OBJECT
public:
    ~GPSWINDOW();
    GPSWINDOW(QWidget *parent = nullptr);
private:
    SearchView *m_searchView = nullptr;
    CurrentLocationView *m_currentlocationView = nullptr;
    void currentShow();
    void recoverCurrentShow();
    void restoreMapToDefault();

protected:
    bool event(QEvent *e) Q_DECL_OVERRIDE;

public Q_SLOTS:
    void pullUpWindow(const QString&);
};

#endif // GPSWINDOW_H
