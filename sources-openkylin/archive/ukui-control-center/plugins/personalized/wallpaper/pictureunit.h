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
#ifndef PICTUREUNIT_H
#define PICTUREUNIT_H

#include <QLabel>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QEvent>
#include "masklabel.h"

class PictureInfo {
public:
    PictureInfo() {
        fileName.clear();
    }
    QPixmap pixmap;
    QString fileName;
};

class PictureUnit : public QLabel
{
    Q_OBJECT

public:
    explicit PictureUnit(QWidget *parent = nullptr, QSize size = QSize(213, 145));
    ~PictureUnit();

public:
    void setFilenameText(QString fn);
    QString filenameText();
    void changeClickedFlag(bool flag);
    bool getClickedFlag();
    void setBackground(QPixmap background);
    void setColor(QColor color);
    void setColorName(QString colorName);
    QString getColorName();
    bool isColorUnit() const { return m_isColorUnit; }
    // 组内唯一 Tab 落点：当前选中 StrongFocus，其余 ClickFocus（颜色块 / 壁纸缩略图共用）
    void takeGroupTabStop();
    // 兼容旧名
    void takeColorGroupTabStop() { takeGroupTabStop(); }

protected:
    void mouseReleaseEvent(QMouseEvent * e);
    void keyPressEvent(QKeyEvent *e);
    bool eventFilter(QObject *w, QEvent *e);
    void paintEvent(QPaintEvent *event);
    void focusInEvent(QFocusEvent *e);
    void focusOutEvent(QFocusEvent *e);

private:
    PictureUnit *findGroupNeighbor(int key) const;
    void activateColorUnit();
    void activatePictureUnit();

    bool m_clickedFlag = false;
    bool m_isTouchControl = false;
    QPointF m_touchBeginPoint;
    QPointF m_touchEndPoint;
    MaskLabel* m_maskLabel = nullptr;
    QPixmap m_background;
    QString m_filename;
    QColor m_color;
    bool m_isColorUnit = false;
    QString m_colorName;

Q_SIGNALS:
    void clicked(QString filename);
};

#endif // PICTUREUNIT_H
