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
#ifndef GLOBALTHEMEFEATURE_H
#define GLOBALTHEMEFEATURE_H

#include <QObject>
#include <QWidget>
#include <QVBoxLayout>
#include <QApplication>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QDebug>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QtMath>
#include <QGraphicsProxyWidget>
#include <QPushButton>
#include <QPainter>
#include <QLineEdit>
#include <QMimeData>

class RoundedWidget : public QWidget
{
    Q_OBJECT

public:
    RoundedWidget(QWidget *parent = nullptr);

    void setRadius(int radius);
    void setTransparency(int transparency);
    void setBackgroundColor(const QColor& color);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int m_radius;
    double m_transparency;
    QColor backgroundColor = Qt::white;    // 背景颜色
    QColor borderColor = Qt::white;        // 边框颜色
    int borderWidth = 1;                    // 边框宽度
};

class RoundedButton : public QPushButton {
public:
    RoundedButton(QWidget *parent = nullptr);

    void setRadius(int radius);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int m_radius;
};

class ShowLineEdit : public QLineEdit {
public:
    ShowLineEdit(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;

};

class ShowLineEdit1 : public ShowLineEdit {
public:
    ShowLineEdit1(QWidget *parent = nullptr);

protected:
    void focusOutEvent(QFocusEvent *event) override;

};



class EditButton : public QPushButton {
    Q_OBJECT
public:
    EditButton(QWidget *parent = nullptr,QString type = "");

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
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

signals:
    void deleteCustomPic();
    void changePic();
    void droprealcover(QString coverFilePath);
    void droprealwallpaper(QString wallpaperFilePath);

private:
    QRect closeIconRect;
    QPixmap pixmap;
    QPixmap closeIconPixmap;
    QPushButton *subbtn;
    QString m_type;
};

class CustomGraphicsView : public QGraphicsView {
public:
    explicit CustomGraphicsView(QGraphicsScene* scene);

protected:
    void wheelEvent(QWheelEvent* event) override;
};

class GlobalImageWidget : public QWidget {
public:
    GlobalImageWidget(QWidget *parent = nullptr, const QString& coverFilePath = "");
    void updateWidgetRadius(int sliderValue);
    void updateButtonColor(const QColor& color);
    void updateWidgetBackgroundColor(const QColor& color);
    void updateWallpaper(const QString& WallpaperFilePath);
    void updateOverlayImage(const QString& overlayImagePath);
    void updatescale();
    void updateControlRadius(int Value);
    void updateTransparency(int transparency);
    void updatescale1();
    void maxscale();
    void miniscale();
private:
    CustomGraphicsView* graphicsView;
    QString m_coverFilePath;
    RoundedWidget *m_showwidget;
    RoundedButton *m_showbtn;
    QGraphicsScene* m_scene;
    QPixmap m_mergedImage;
    QPixmap m_image;
    QGraphicsProxyWidget* m_proxy;
    bool m_islight;
};
class addPicWidget : public QWidget
{
    Q_OBJECT
public:
    addPicWidget(QWidget *parent = nullptr);
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
};
class Globalthemefeature : public QWidget {
    Q_OBJECT
public:
    explicit Globalthemefeature(QWidget *parent = nullptr, const QString& coverFilePath = "");

signals:

};

#endif // GLOBALTHEMEFEATURE_H
