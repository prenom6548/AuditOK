/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * Copyright (C) 2019 KylinSoft  Co., Ltd.
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
#ifndef THEMEICONLABEL_H
#define THEMEICONLABEL_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QIcon>
#include <QMouseEvent>
#include "masklabel.h"
#include "drawiconlabel.h"

class ThemeIconLabel : public QLabel
{
    Q_OBJECT
public:
    ThemeIconLabel(QSize size, QWidget *parent = nullptr);
    ThemeIconLabel(QPixmap pixmap, QSize size, QWidget *parent = nullptr);
    ThemeIconLabel(const QIcon &icon, QSize size, QWidget *parent);
    ThemeIconLabel(QStringList iStringList, QSize size, QWidget *parent = nullptr);
    ThemeIconLabel(QList<QPixmap> iconList, QSize size, QWidget *parent = nullptr);

    QPixmap pixmapToRound(const QString &filePath, const QSize &scaledSize);
    QPixmap pixmapToRound(const QPixmap &pixmap, const QSize &scaledSize);
    void setRadius(int radius);
    void setIcon(QPixmap pixmap);
    void setIconCenter();
    void changeClickedFlag(bool flag);
    void setMaskSize(const QSize &);

protected:
    bool eventFilter(QObject *w, QEvent *e);
    void paintEvent(QPaintEvent *event);

private:
    void initMaskLabel();

private:
    DrawIconLabel *mPictrueLabel = nullptr;
    QStringList mIconList;
    int mRadius = 10;
    QLabel *m_pIconLabel = nullptr;
    MaskLabel* m_maskLabel = nullptr;
    bool m_clickedFlag = false;
    QIcon m_background;

Q_SIGNALS:
    void enterWidget();
    void leaveWidget();
};

#endif // THEMEICONLABEL_H
