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

#ifndef CURRENTLOCATIONVIEW_H
#define CURRENTLOCATIONVIEW_H
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QGSettings>
#include "currentlocation.h"
#include "dbusclient.h"
#include "formatconversion.h"

namespace kmre_gps
{
class CurrentLocationView : public QWidget
{
    Q_OBJECT
public:
    CurrentLocationView(QWidget *parent = 0);

    QLabel *m_tip = nullptr;
    QLabel *m_label = nullptr;
    QPushButton *m_saveBtn = nullptr;
    QString m_data = "";

private:
    void initWidget();
    void currentLocationShow();

private Q_SLOTS:
    void connectServer();

Q_SIGNALS:
    void restoreMapToDefault();
};

} // namespace kmre_gps
#endif // CURRENTLOCATIONVIEW_H
