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
#ifndef PLYMOUTHTHEMEFEATURE_H
#define PLYMOUTHTHEMEFEATURE_H

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
#include <QTimer>
#include <QLabel>
#include <QFileInfo>
#include <QDir>
#include <QPushButton>
#include <QMimeData>
#include "../fileProcess/filecheck.h"

class PlymouthGraphicsView : public QGraphicsView {
public:
    explicit PlymouthGraphicsView(QGraphicsScene* scene);

protected:
    void wheelEvent(QWheelEvent* event) override;
    void drawBackground(QPainter *painter, const QRectF &rect) override;


};

class DropPlyButton : public QPushButton {
    Q_OBJECT
public:
    DropPlyButton(QWidget *parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
signals:
    void droprealPly(QString iconFilePath);
};

class PlymouthCustomLabel : public QLabel {
    Q_OBJECT
public:
    explicit PlymouthCustomLabel(QWidget* parent = nullptr);

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
signals:
    void deleteCustomIcon();
private:
    QRect closeIconRect;
    QPixmap pixmap;
    QPixmap closeIconPixmap;
};
class plymouthAddPicWidget : public QWidget
{
    Q_OBJECT
public:
    plymouthAddPicWidget(QWidget *parent = nullptr);
    void setType(QString type);
protected:
    void paintEvent(QPaintEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
signals:

    void buttonclick();
    void dropRealPic(QString picFilePath);

private:
    QString m_type;
    QLabel *tip;
};
class PlymouthImageWidget : public QWidget {
    Q_OBJECT
public:
    PlymouthImageWidget(QWidget *parent = nullptr);
    void updatePlymouth(const QString& imagePath);
    void showPlymouth();
    void maxscale();
    void miniscale();
private:
    PlymouthGraphicsView* graphicsView = NULL;
    QGraphicsScene* scene;
    QVBoxLayout* layout;
    QTimer timer;
    int sceneWidth = 0;
    int sceneHeight = 0;
    int index = 0;
    QList<QPixmap> pixmaps;
    QGraphicsPixmapItem *pixmapItem;
};

class PlymouthThemeFeature : public QWidget
{
    Q_OBJECT
public:
    explicit PlymouthThemeFeature(QWidget *parent = nullptr);

signals:

};

#endif // PLYMOUTHTHEMEFEATURE_H
