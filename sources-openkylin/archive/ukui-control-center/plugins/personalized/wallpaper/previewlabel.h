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
#ifndef PREVIEWLABEL_H
#define PREVIEWLABEL_H
#include<QLabel>

class PreviewLabel : public QLabel
{
public:
    explicit PreviewLabel(QWidget *parent=nullptr, Qt::WindowFlags f=Qt::WindowFlags());
    explicit PreviewLabel(const QString &text, QWidget *parent=nullptr, Qt::WindowFlags f=Qt::WindowFlags());
    ~PreviewLabel();
    void setColor(const QColor &color);
    void setLocalPixmap(QPixmap pixmap);
    void setupScreenMonitoring();
    void onScreenAdded(QScreen *screen);
    void onScreenRemoved(QScreen *screen);
    void updateSize();
    QSize maxScreenSize() const;
    QPixmap scaleExistingPixmap(const QPixmap& originalPixmap);
    QSize calculateWallpaperTargetSize(const QSize &originalSize, const QSize &screenSize) const;
    QSize calculateScaledSize(const QSize &sourceSize, const QSize &maxSize) const;
    QRect getSourceRect(const QPixmap &pixmap, const QRect &screenGeometry);
    QRect getSourceRect(const QPixmap &pixmap);
    QRect getDestRect(const QPixmap &pixmap);
    void setMode(QString mode);

protected:
    void paintEvent(QPaintEvent *e);

private:
    QString mode;
    QPixmap m_pixmap;
    QColor mColor;
    QString displayType;
};

#endif // PREVIEWLABEL_H
