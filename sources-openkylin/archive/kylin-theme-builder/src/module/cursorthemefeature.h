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
#ifndef CURSORTHEMEFEATURE_H
#define CURSORTHEMEFEATURE_H

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QButtonGroup>
#include <QPainter>
#include <QScrollArea>
#include <QLabel>
#include <QMap>
#include <QDebug>
#include <QFileDialog>
#include <QStackedWidget>
#include <QWheelEvent>
#include <QDir>
#include <QFileInfo>
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QDebug>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QtMath>
#include <QGraphicsPixmapItem>
#include <QPixmap>
#include <QGraphicsBlurEffect>
#include "iconwidgetfeature.h"
#include "batchimportdialog.h"

class CursorCustomLabel : public QLabel
{
    Q_OBJECT
public:
    explicit CursorCustomLabel(QWidget* parent = nullptr);

    void setPixmap(const QPixmap& pixmap);

    void initializeCloseIconRect();
    void hideCloseIcon();
    void showCloseIcon();
    void drawCloseIcon(QPainter &painter);
signals:
    void deleteCustomIcon();
protected:
    void paintEvent(QPaintEvent* event) override;

    void mousePressEvent(QMouseEvent *event);
    void leaveEvent(QEvent *event);
    void enterEvent(QEvent *event);
private:
    QRect closeIconRect;
    QPixmap pixmap;
    QPixmap closeIconPixmap;

};

class cursorAddIconWidget : public QWidget
{
    Q_OBJECT
public:
    cursorAddIconWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
signals:
    void buttonclick();
    void droprealicon(QString iconFilePath);

private:

};

class CursorEditWidget : public QWidget
{
    Q_OBJECT
public:
    CursorEditWidget(QWidget *parent = nullptr);
    void setdefaulticon(QString iconname);
    void setcustomicon(QString iconFilePath);

    DropButton *m_addiconbutton;
signals:
    void deleteCustomIcon();
private:
    QHBoxLayout *m_iconwidgetlayout;
    QLabel *m_icondefaultlabel;
    CursorCustomLabel *m_icondecustomlabel;
    QLabel *m_tiplabel;

};

class CursorGraphicsView : public QGraphicsView
{
public:
    explicit CursorGraphicsView(QGraphicsScene* scene);

protected:
    void wheelEvent(QWheelEvent* event) override;

};

class CursorImageWidget : public QWidget
{
public:
    CursorImageWidget(QWidget *parent = nullptr, const QMap<QString,QString>* iconMap = nullptr);
    void updateIcon(const QString& widgetName, const QString& newFilePath);

    void updateImage(const QString& imagePath);
    void updateIconMap(const QMap<QString, QString>* newIconMap);
    void setIconMap(QMap<QString,QString>*);

    void maxscale();
    void miniscale();

private:
    const QMap<QString,QString>* m_iconMap;
    CursorGraphicsView* graphicsView;
    QString m_coverFilePath;

};

class CursorThemeFeature : public QWidget
{
    Q_OBJECT
public:
    explicit CursorThemeFeature(QWidget *parent = nullptr, const QMap<QString,QString>* iconMap = nullptr);

signals:

};

#endif // CURSORTHEMEFEATURE_H
