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
#ifndef MAININTERFACEFEATURE_H
#define MAININTERFACEFEATURE_H

#include "../fileProcess/filecheck.h"
#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QButtonGroup>
#include <QPainter>
#include <QScrollArea>
#include <QLabel>
#include <QMap>
#include <QDebug>
#include <QFileDialog>
#include <QStackedWidget>
#include <QWheelEvent>
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QDebug>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QtMath>
#include <QGraphicsPixmapItem>
#include <QGraphicsTextItem>
#include <QTextBlockFormat>
#include <QtWidgets>
#include <QPen>

class MainInterFaceFeature : public QWidget
{
    Q_OBJECT
public:
    explicit MainInterFaceFeature(QWidget *parent = nullptr);

};

class TypeButton : public QPushButton
{
public:
    TypeButton(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

};


class IconGraphicsView : public QGraphicsView {
public:
    explicit IconGraphicsView(QGraphicsScene* scene);

protected:
    void wheelEvent(QWheelEvent* event) override;
};

class ImageWidget : public QWidget {
    Q_OBJECT
public:
    ImageWidget(QWidget *parent = nullptr, const QMap<QString,QString>* iconMap = nullptr);
    void updateIcon(const QString& widgetName, const QString& newFilePath);
    void updateImage(const QString& imagePath);
    void updateIconMap(const QMap<QString, QString>* newIconMap,QString type);
    void maxscale();
    void miniscale();
    QMap<QString, QString> *initIconName(QMap<QString, QString> *name_CH, QList<QString> keys);
    QMap<QString, QString> *initSystemIconName(QMap<QString, QString> *name_CH, QList<QString> keys);
    QMap<QString, QString> *initPlaceIconName(QMap<QString, QString> *name_CH, QList<QString> keys);
    QMap<QString, QString> *initDeviceIconName(QMap<QString, QString> *name_CH, QList<QString> keys);
private:
    const QMap<QString,QString>* m_iconMap;
    QPixmap* image;
    IconGraphicsView* graphicsView;
    QString m_coverFilePath;
};

class CustomLabel : public QLabel {
    Q_OBJECT
public:
    explicit CustomLabel(QWidget* parent = nullptr);

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

class DropButton : public QPushButton {
    Q_OBJECT
public:
    DropButton(QWidget *parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
signals:
    void droprealicon(QString iconFilePath);
};

class addIconWidget : public QWidget
{
    Q_OBJECT
public:
    addIconWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
signals:
    void buttonclick();
    void droprealicon(QString iconFilePath);

private:

};

class IconEditWidget : public QWidget
{
    Q_OBJECT
public:
    IconEditWidget(QWidget *parent = nullptr);
    void setdefaulticon(QString iconname);
    void setdefaulticonname(QString iconname);
    void setcustomicon(QString iconFilePath);
    QLabel *m_label;
    DropButton *m_addiconbutton;
signals:
    void deleteCustomIcon();
private:

    QLabel *m_icondefaultlabel;
    CustomLabel *m_icondecustomlabel;
    addIconWidget *m_addwidget;
};
#endif // MAININTERFACEFEATURE_H
