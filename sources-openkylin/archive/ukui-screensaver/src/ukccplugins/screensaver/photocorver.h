/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#ifndef PHOTOCORVER_H
#define PHOTOCORVER_H

#include <QLabel>
#include <QWidget>
#include <QMenu>
#include <QEvent>
#include <QMouseEvent>
#include "../../screensaver/masklable.h"

class PhotoCorver : public QLabel
{
    Q_OBJECT
public:
    explicit PhotoCorver(QWidget *parent = nullptr, bool isSystemCorver = false, bool isScreensaver = true);
    void setBackground(QPixmap background);
    void setClickStatus(bool state);

protected:
    void paintEvent(QPaintEvent *event);
    bool eventFilter(QObject *watched, QEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
Q_SIGNALS:
    void photoCorverLeftClicked();
    void openFileFolderClicked();
    void removePathClicked();

private:
    QPixmap m_background;
    bool m_bHoverStatus = false;
    bool m_bSelectStatus = false;
    MaskLable *m_maskLabel = nullptr;
    QMenu *m_rightClickMenu = nullptr;
    bool m_bSystemCorver = false;
    bool m_bScreensaver = true;
};

#endif // PHOTOCORVER_H
