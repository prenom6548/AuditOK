/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: LiKe <like1@kylinos.cn>
 *
 */
#ifndef GRUBTHEMEFEATURE_H
#define GRUBTHEMEFEATURE_H

#include <QObject>
#include <QApplication>
#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QGraphicsView>
#include <QVBoxLayout>
#include <QWheelEvent>
#include <QPainter>
#include <QtMath>
#include <QPushButton>
#include <QMimeData>
#include <QLabel>

#include "../fileProcess/filecheck.h"

class GrubGraphicsView : public QGraphicsView {
public:
    explicit GrubGraphicsView(QGraphicsScene* scene);

protected:
    void wheelEvent(QWheelEvent* event) override;
//    void drawBackground(QPainter *painter, const QRectF &rect) override;


};

class GrubEditButton : public QPushButton {
    Q_OBJECT
public:
    GrubEditButton(QWidget *parent = nullptr);

    void setPixmap(const QPixmap& pixmap);
    void showCloseIcon();
    void hideCloseIcon();
    void initializeCloseIconRect();
    void drawCloseIcon(QPainter &painter);
protected:
    void paintEvent(QPaintEvent* event) override;

    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void dragEnterEvent(QDragEnterEvent *event);
    void dropEvent(QDropEvent *event);

signals:
    void deleteCustomPic();
    void changePic();
    void droprealgrub(QString grubFilePath);
private:
    QRect closeIconRect;
    QPixmap pixmap;
    QPixmap closeIconPixmap;
    QPushButton *subbtn;
};

class GrubImageWidget : public QWidget {
public:
    GrubImageWidget(QWidget *parent = nullptr);
    void updateBackground(const QString& imagePath);
    void maxscale();
    void miniscale();
private:
    GrubGraphicsView* graphicsView;

    QGraphicsScene* m_scene;
    QPixmap m_mergedImage;
    QPixmap m_image;
    QGraphicsPixmapItem* m_backgroundItem;

    QPixmap m_overlayPixmap;
    QGraphicsPixmapItem* m_overlayItem;
    QGraphicsProxyWidget* m_proxy;
};
class grubAddPicWidget : public QWidget
{
    Q_OBJECT
public:
    grubAddPicWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
signals:

    void buttonclick();
    void dropRealPic(QString picFilePath);

};
class GrubThemeFeature : public QWidget
{
    Q_OBJECT
public:
    explicit GrubThemeFeature(QWidget *parent = nullptr);

signals:

};

#endif // GRUBTHEMEFEATURE_H
