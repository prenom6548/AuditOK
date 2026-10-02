/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
#ifndef ITEMWIDGET_H
#define ITEMWIDGET_H

#include <QWidget>

namespace Ui {
class ItemWidget;
}

class QSettings;
class QMovie;
class QString;
class QMouseEvent;
class QPaintEvent;
class QGSettings;
class ItemWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ItemWidget(QString itemname,QWidget *parent = nullptr);
    ~ItemWidget();

protected:
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent*);

public slots:
    void startMovieSlot();
    void stopMovieSlot();

private:
    Ui::ItemWidget* ui;
    QMovie* movie;
    QSettings* gesturesetting = nullptr;
    QGSettings* stylegsetting = nullptr;

    int status = -1;
    QPixmap currentpix;
    QColor color1;
    QColor color2;
    QColor color1_pre;
    QColor color2_pre;

Q_SIGNALS:
    void picClicked();
};

#endif // ITEMWIDGET_H
