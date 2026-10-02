/* rootWindowBackground.cpp
 * Copyright (C) 2018 Tianjin KYLIN Information Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 **/

#include <QScreen>
#include <QX11Info>
#include <QRect>
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <stdio.h>
#include <X11/Xlib.h>
#include <Imlib2.h>
#include <qmath.h>
#include <QStringList>

#include "rootWindowBackground.h"

static const XID INVAL_ID = ~0UL;
Pixmap pix = INVAL_ID;
Display *dpy = NULL;
Window root = 0;
Screen *scn = NULL;

struct RootWindowBGInfo
{
    bool isValid = false;
    int nType;
    int nColor;
    QString strFileName;
    int nOption;
    QList<QRect> listScreen;
};

static RootWindowBGInfo g_lastRootWndInfo;

typedef enum WNDBG_OPTION_e
{
    WNDBG_OPTION_SCALED,    // 填充
    WNDBG_OPTION_STRETCHED, // 拉伸
    WNDBG_OPTION_CENTERED,  // 居中
    WNDBG_OPTION_WALLPAPER, // 平铺
    WNDBG_OPTION_ZOOM,      // 适应
    WNDBG_OPTION_SPANNED    // 跨区
} WNDBG_OPTION;

static QRect getSourceRect(const QRect &destination, const QRect &source)
{
    qreal screenScale = qreal(destination.width()) / qreal(destination.height());
    qreal width = source.width();
    qreal height = source.height();

    if ((width / height) == screenScale) {
        return source;
    }

    bool isShortX = (width <= height);
    if (isShortX) {
        screenScale = qreal(destination.height()) / qreal(destination.width());
    }

    qreal shortEdge = isShortX ? width : height;
    qreal longEdge = isShortX ? height : width;

    while (shortEdge > 1) {
        qint32 temp = qFloor(shortEdge * screenScale);
        if (temp <= longEdge) {
            longEdge = temp;
            break;
        }

        qint32 spacing = qRound(shortEdge / 20);
        if (spacing <= 0) {
            spacing = 1;
        }
        shortEdge -= spacing;
    }

    QSize sourceSize = source.size();
    if (shortEdge > 1 && longEdge > 1) {
        sourceSize.setWidth(isShortX ? shortEdge : longEdge);
        sourceSize.setHeight(isShortX ? longEdge : shortEdge);
    }

    qint32 offsetX = 0;
    qint32 offsetY = 0;
    if (source.width() > sourceSize.width()) {
        offsetX = (source.width() - sourceSize.width()) / 2;
    }

    if (source.height() > sourceSize.height()) {
        offsetY = (source.height() - sourceSize.height()) / 2;
    }

    QPoint offsetPoint = source.topLeft();
    offsetPoint += QPoint(offsetX, offsetY);

    return QRect(offsetPoint, sourceSize);
}

static QRect getDestRect(const QRect &destination, const QRect &source)
{
    qreal screenScale = qreal(destination.width()) / qreal(destination.height());
    qreal pixmapScale = qreal(source.width() / source.height());
    qreal width = source.width();
    qreal height = source.height();

    if (pixmapScale == screenScale) {
        return destination;
    }

    qreal scaleWidth = destination.width() / width;
    qreal scaleHeight = destination.height() / height;
    qreal realPixmapWidth = 0;
    qreal realPixmapHeight = 0;

    if (pixmapScale < screenScale) {
        //图片比例小于屏幕比例时，按照图片和屏幕高度比进行缩放
        realPixmapWidth = width * scaleHeight;
        realPixmapHeight = destination.height();
    } else {
        //图片比例大于屏幕比例时，按照图片与屏幕宽度比进行缩放
        realPixmapWidth = destination.width();
        realPixmapHeight = height * scaleWidth;
    }

    QSize sourceSize = destination.size();
    qint32 offsetX = 0;
    qint32 offsetY = 0;
    if (destination.width() == realPixmapWidth) {
        offsetY = (destination.height() - realPixmapHeight) / 2;
        sourceSize.setHeight(realPixmapHeight);
    } else if (destination.height() == realPixmapHeight) {
        offsetX = (destination.width() - realPixmapWidth) / 2;
        sourceSize.setWidth(realPixmapWidth);
    }

    qDebug() << "=========getDestRect sourceSize:" << sourceSize;
    QPoint offsetPoint = destination.topLeft();
    offsetPoint += QPoint(offsetX, offsetY);

    return QRect(offsetPoint, sourceSize);
}

static QRect getSourceRect(const QRect &source, const QRect &screenGeometry, const QRect &screenVirtualGeometry)
{
    qreal pixWidth = source.width();
    qreal pixHeight = source.height();

    QSize sourceSize = source.size();
    sourceSize.setWidth(screenGeometry.width() * 1.0 / screenVirtualGeometry.width() * pixWidth);
    sourceSize.setHeight(screenGeometry.height() * 1.0 / screenVirtualGeometry.height() * pixHeight);

    qint32 offsetX = 0;
    qint32 offsetY = 0;
    if (screenGeometry.x() > 0) {
        offsetX = (screenGeometry.x() * 1.0 / screenVirtualGeometry.width() * pixWidth);
    }

    if (screenGeometry.y() > 0) {
        offsetY = (screenGeometry.y() * 1.0 / screenVirtualGeometry.height() * pixHeight);
    }

    QPoint offsetPoint = source.topLeft();
    offsetPoint += QPoint(offsetX, offsetY);

    return QRect(offsetPoint, sourceSize);
}

void setRootWindowBackground(
    bool type, unsigned int color, char *filename, const QList<QPair<QRect, QRect>> screenRectList, int nOption)
{
    Imlib_Image img;

    if (!dpy) {
        dpy = XOpenDisplay(NULL);
        if (!dpy)
            return;
    }

    int width = 0, height = 0;

    width = QApplication::desktop()->geometry().width() * qApp->devicePixelRatio();
    height = QApplication::desktop()->geometry().height() * qApp->devicePixelRatio();

    if (!scn)
        scn = DefaultScreenOfDisplay(dpy);
    if (!root)
        root = DefaultRootWindow(dpy);

    if (pix != INVAL_ID) {
        XFreePixmap(dpy, pix);
        pix = INVAL_ID;
    }
    pix = XCreatePixmap(dpy, root, width, height, DefaultDepthOfScreen(scn));

    imlib_context_set_display(dpy);
    imlib_context_set_visual(DefaultVisualOfScreen(scn));
    imlib_context_set_colormap(DefaultColormapOfScreen(scn));
    imlib_context_set_drawable(pix);

    if (type == 0) {
        img = imlib_load_image(filename);
        if (!img) {
            fprintf(stderr, "%s:Unable to load image\n", filename);
            return;
        }
        imlib_context_set_image(img);

    } else if (type == 1) {
        img = imlib_create_image(width, height);
        imlib_context_set_image(img);
        int blue = color & 0xFF;
        int green = color >> 8 & 0xFF;
        int red = color >> 16 & 0xFF;

        qDebug() << "red = " << red << " green = " << green << " blue = " << blue;
        imlib_context_set_color(red, green, blue, 255);
        imlib_image_fill_rectangle(0, 0, width, height);
    }
    g_lastRootWndInfo.isValid = true;
    g_lastRootWndInfo.nType = type;
    g_lastRootWndInfo.nColor = color;
    g_lastRootWndInfo.strFileName = filename;
    g_lastRootWndInfo.nOption = nOption;
    g_lastRootWndInfo.listScreen.clear();
    imlib_context_set_image(img);

    QRect rectImg(0, 0, imlib_image_get_width(), imlib_image_get_height());

    for (QPair<QRect, QRect> pair : screenRectList) {
        QRect rect = pair.first;
        //在每个屏幕上绘制背景
        rect.setRect(
            rect.x() * qApp->devicePixelRatio(),
            rect.y() * qApp->devicePixelRatio(),
            rect.width() * qApp->devicePixelRatio(),
            rect.height() * qApp->devicePixelRatio());
        QRect newSrcRect = rectImg;
        g_lastRootWndInfo.listScreen.append(rect);

        switch (nOption) {
            case WNDBG_OPTION_CENTERED: //居中
            {
                if (newSrcRect.width() > rect.width()) {
                    int nHDiff = (newSrcRect.width() - rect.width()) / 2;
                    newSrcRect.setX(newSrcRect.x() + nHDiff);
                    newSrcRect.setWidth(newSrcRect.width() - nHDiff);
                } else {
                    int nHDiff = (rect.width() - newSrcRect.width()) / 2;
                    rect.setX(rect.x() + nHDiff);
                    rect.setWidth(rect.width() - nHDiff);
                }
                if (newSrcRect.height() > rect.height()) {
                    int nVDiff = (newSrcRect.height() - rect.height()) / 2;
                    newSrcRect.setY(newSrcRect.y() + nVDiff);
                    newSrcRect.setHeight(newSrcRect.height() - nVDiff);
                } else {
                    int nVDiff = (rect.height() - newSrcRect.height()) / 2;
                    rect.setY(rect.y() + nVDiff);
                    rect.setHeight(rect.height() - nVDiff);
                }
                qDebug() << "Centered:" << rect << newSrcRect << rectImg;
                imlib_render_image_part_on_drawable_at_size(
                    newSrcRect.x(),
                    newSrcRect.y(),
                    newSrcRect.width(),
                    newSrcRect.height(),
                    rect.x(),
                    rect.y(),
                    rect.width(),
                    rect.height());
            } break;
            case WNDBG_OPTION_STRETCHED: //拉伸
            {
                qDebug() << "Stretched:" << rect << newSrcRect << rectImg;
                imlib_render_image_part_on_drawable_at_size(
                    newSrcRect.x(),
                    newSrcRect.y(),
                    newSrcRect.width(),
                    newSrcRect.height(),
                    rect.x(),
                    rect.y(),
                    rect.width(),
                    rect.height());
            } break;
            case WNDBG_OPTION_SCALED: //填充
            {
                newSrcRect = getSourceRect(rect, newSrcRect);
                qDebug() << "Scaled:" << rect << newSrcRect << rectImg;
                imlib_render_image_part_on_drawable_at_size(
                    newSrcRect.x(),
                    newSrcRect.y(),
                    newSrcRect.width(),
                    newSrcRect.height(),
                    rect.x(),
                    rect.y(),
                    rect.width(),
                    rect.height());
            } break;
            case WNDBG_OPTION_WALLPAPER: // 平铺
            {
                int drawedWidth = 0;
                int drawedHeight = 0;
                while (1) {
                    drawedWidth = 0;
                    while (1) {
                        imlib_render_image_part_on_drawable_at_size(
                            newSrcRect.x(),
                            newSrcRect.y(),
                            newSrcRect.width(),
                            newSrcRect.height(),
                            drawedWidth,
                            drawedHeight,
                            newSrcRect.width(),
                            newSrcRect.height());
                        drawedWidth += newSrcRect.width();
                        if (drawedWidth >= rect.width()) {
                            break;
                        }
                    }
                    drawedHeight += newSrcRect.height();
                    if (drawedHeight >= rect.height()) {
                        break;
                    }
                }
            } break;
            case WNDBG_OPTION_ZOOM: // 适应
            {
                rect = getDestRect(rect, newSrcRect);
                qDebug() << "Zoom:" << rect << newSrcRect << rectImg;
                imlib_render_image_part_on_drawable_at_size(
                    newSrcRect.x(),
                    newSrcRect.y(),
                    newSrcRect.width(),
                    newSrcRect.height(),
                    rect.x(),
                    rect.y(),
                    rect.width(),
                    rect.height());
            } break;
            case WNDBG_OPTION_SPANNED: // 跨区
            {
                // for(auto sscreen : qApp->screens()){
                //  if (screen->name() == sscreen->name()) {
                QRect srcRect = getSourceRect(newSrcRect, pair.first, pair.second);
                imlib_render_image_part_on_drawable_at_size(
                    srcRect.x(),
                    srcRect.y(),
                    srcRect.width(),
                    srcRect.height(),
                    rect.x(),
                    rect.y(),
                    rect.width(),
                    rect.height());
                break;
                // }
                //}
            } break;
            default: {
                imlib_render_image_part_on_drawable_at_size(
                    newSrcRect.x(),
                    newSrcRect.y(),
                    newSrcRect.width(),
                    newSrcRect.height(),
                    rect.x(),
                    rect.y(),
                    rect.width(),
                    rect.height());
            } break;
        }
    }

    imlib_free_image();
}

void updateRootWindowBackground()
{
    // 对比新的屏幕尺寸是否有变化
    if (g_lastRootWndInfo.isValid) {
        bool isSame = true;
        QList<QRect> listScreen;
        for (QScreen *screen : QApplication::screens()) {
            //在每个屏幕上绘制背景
            QRect rect = screen->geometry();
            rect.setX(rect.x() * screen->devicePixelRatio());
            rect.setY(rect.y() * screen->devicePixelRatio());
            rect.setWidth(rect.width() * screen->devicePixelRatio());
            rect.setHeight(rect.height() * screen->devicePixelRatio());
            listScreen.append(rect);
        }
        qDebug() << "ScreenRects:" << listScreen << "||" << g_lastRootWndInfo.listScreen;
        if (listScreen.size() > g_lastRootWndInfo.listScreen.size()) {
            isSame = false;
        } else {
            for (int n = 0; n < listScreen.size(); n++) {
                if (n < g_lastRootWndInfo.listScreen.size()) {
                    if (g_lastRootWndInfo.listScreen[n] != listScreen[n]) {
                        isSame = false;
                        break;
                    }
                }
            }
        }
        if (!isSame) {
            QList<QPair<QRect, QRect>> screenRectList;
            for (QScreen *screen : QApplication::screens()) {
                QRect rect1 = screen->geometry();
                QRect rect2 = screen->virtualGeometry();
                screenRectList.append(qMakePair(rect1, rect2));
            }
            setRootWindowBackground(
                g_lastRootWndInfo.nType,
                g_lastRootWndInfo.nColor,
                g_lastRootWndInfo.strFileName.toLatin1().data(),
                screenRectList,
                g_lastRootWndInfo.nOption);
        }
    }
}

void drawBackground(bool isDraw)
{
    if (!dpy) {
        return;
    }
    if (isDraw) {
        if (pix != INVAL_ID)
            XSetWindowBackgroundPixmap(dpy, root, pix);
        XClearWindow(dpy, root);
    }

    while (XPending(dpy)) {
        XEvent ev;
        XNextEvent(dpy, &ev);
    }
    if (pix != INVAL_ID) {
        XFreePixmap(dpy, pix);
        pix = INVAL_ID;
    }
    XCloseDisplay(dpy);
    dpy = NULL;
}
