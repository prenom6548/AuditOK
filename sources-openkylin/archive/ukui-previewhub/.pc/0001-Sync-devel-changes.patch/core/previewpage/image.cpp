/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2021, KylinSoft Co, Ltd.
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
 * Authors: Lu Hongxu <lhongxu@outlook.com>
 *
 */

#include "previewpage/image.h"

#include <QEvent>
#include <QImage>
#include <QMouseEvent>
#include <QWheelEvent>

ImagePreviewPage::ImagePreviewPage(QWidget *parent) : BasePreviewPage(parent)
{
    m_image_scene = new QGraphicsScene;
    m_image_item = new QGraphicsPixmapItem;
    m_image_view = new QGraphicsView(m_image_scene, this);

    this->setMouseTracking(true);
    m_image_view->setMouseTracking(true);
    m_image_view->viewport()->setMouseTracking(true);
    m_image_view->viewport()->installEventFilter(this);
    m_image_item->setFlag(QGraphicsItem::ItemIsMovable, true);

    m_scale = 1;
    m_base_scale = 1;
    m_is_moveing = false;
    m_is_rotate90 = false;

    m_tool_bar = new QFrame(this);
    m_tool_layout = new QHBoxLayout(m_tool_bar);

    const QSize TOOL_BUTTON = QSize(24,24);
    m_reduce_button = new QPushButton(m_tool_bar);
    //m_reduce_button->setText("reduce");
    m_reduce_button->setFixedSize(TOOL_BUTTON);
    m_reduce_button->setFocusPolicy(Qt::NoFocus);
    m_tool_layout->addWidget(m_reduce_button);
    connect(m_reduce_button, &QPushButton::clicked, this, &ImagePreviewPage::doReduce);

    m_enlarge_button = new QPushButton(m_tool_bar);
    //m_enlarge_button->setText("enlarge");
    m_enlarge_button->setFixedSize(TOOL_BUTTON);
    m_enlarge_button->setFocusPolicy(Qt::NoFocus);
    m_tool_layout->addWidget(m_enlarge_button);
    connect(m_enlarge_button, &QPushButton::clicked, this, &ImagePreviewPage::doEnlarge);

    m_adaptive_button = new QPushButton(m_tool_bar);
    //m_adaptive_button->setText("adaptive");
    m_adaptive_button->setFixedSize(TOOL_BUTTON);
    m_adaptive_button->setFocusPolicy(Qt::NoFocus);
    m_tool_layout->addWidget(m_adaptive_button);
    connect(m_adaptive_button, &QPushButton::clicked, this, &ImagePreviewPage::doAdaptive);

    m_rotate_button = new QPushButton(m_tool_bar);
    //m_rotate_button->setText("rotate");
    m_rotate_button->setFixedSize(TOOL_BUTTON);
    m_rotate_button->setFocusPolicy(Qt::NoFocus);
    m_tool_layout->addWidget(m_rotate_button);
    connect(m_rotate_button, &QPushButton::clicked, this, &ImagePreviewPage::doRotate);

    m_horizontal_flip_button = new QPushButton(m_tool_bar);
    //m_horizontal_flip_button->setText("Hfilp");
    m_horizontal_flip_button->setFixedSize(TOOL_BUTTON);
    m_horizontal_flip_button->setFocusPolicy(Qt::NoFocus);
    m_tool_layout->addWidget(m_horizontal_flip_button);
    connect(m_horizontal_flip_button, &QPushButton::clicked, this, &ImagePreviewPage::doHorizontalFlip);

    m_vertical_flip_button = new QPushButton(m_tool_bar);
    //m_vertical_flip_button->setText("Vflip");
    m_vertical_flip_button->setFixedSize(TOOL_BUTTON);
    m_vertical_flip_button->setFocusPolicy(Qt::NoFocus);
    m_tool_layout->addWidget(m_vertical_flip_button);
    connect(m_vertical_flip_button, &QPushButton::clicked, this, &ImagePreviewPage::doVerticalFlip);

    initControlQss();

    base_layout->addWidget(m_image_view);
    base_layout->addWidget(m_tool_bar);
}

ImagePreviewPage::~ImagePreviewPage() {
   m_image_scene->deleteLater();
}

void ImagePreviewPage::updateInfo(PreviewHubFileInfo *info) {
    m_is_rotate90 = false;
    QPixmap new_pixmap(info->filePath());
    m_image_item = m_image_scene->addPixmap(new_pixmap);
    m_image_scene->setSceneRect(QRectF(new_pixmap.rect()));
}

void ImagePreviewPage::cancel() {
    m_image_scene->clear();
}

void ImagePreviewPage::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event)
    m_image_view->resetTransform();

    int height = m_image_item->pixmap().height();
    int width = m_image_item->pixmap().width();
    int max_height = m_image_view->height();
    int max_width = m_image_view->width();
    int pic_size = qMax(width, height);
    int max_size = qMin(max_width, max_height) - 5;
    m_base_scale = 1.0 * max_size / pic_size;

    double val = m_scale * m_base_scale;
    m_image_view->scale(val, val);
}

void ImagePreviewPage::doHorizontalFlip() {
    /*QImage origin_image = m_image_item->pixmap().toImage();
    QImage new_image = QImage(QSize(origin_image.width(), origin_image.height()),
                                  QImage::Format_ARGB32);
    QColor tmp_color;
    int r, g, b;
    for (int i = 0; i < new_image.width(); ++i) {
        for (int j = 0; j < new_image.height(); ++j) {
            tmp_color = QColor(origin_image.pixel(i, j));
            r = tmp_color.red();
            g = tmp_color.green();
            b = tmp_color.blue();
            new_image.setPixel(new_image.width()-i-1, j, qRgb(r,g,b));
        }
    }
    QPixmap new_pixmap;
    new_pixmap.convertFromImage(new_image);
    m_image_item->setPixmap(new_pixmap);
    m_image_scene->setSceneRect(QRectF(new_pixmap.rect()));*/
    QRectF r = m_image_item->boundingRect();
    m_image_item->setTransform(m_image_item->transform()
                    .translate(r.width() / 2, r.height() /2)
                    .rotate(180, m_is_rotate90 ? Qt::XAxis : Qt::YAxis)
                    .translate(-r.width() / 2, -r.height() / 2));
}

void ImagePreviewPage::doVerticalFlip() {
    QRectF r = m_image_item->boundingRect();
    m_image_item->setTransform(m_image_item->transform()
                    .translate(r.width() / 2, r.height() /2)
                    .rotate(180, m_is_rotate90 ? Qt::YAxis : Qt::XAxis)
                    .translate(-r.width() / 2, -r.height() / 2));
}

void ImagePreviewPage::doRotate() {
    QRectF r = m_image_item->boundingRect();
    m_image_item->setTransform(m_image_item->transform()
                    .translate(r.width() / 2, r.height() /2)
                    .rotate(90)
                    .translate(-r.width() / 2, -r.height() / 2));
    m_is_rotate90 = !m_is_rotate90;
}

void ImagePreviewPage::doEnlarge() {
    m_scale += 0.05;
    this->repaint();
}

void ImagePreviewPage::doReduce() {
    if (m_scale > 0.06)
        m_scale -= 0.05;
    this->repaint();
}

void ImagePreviewPage::doAdaptive() {
    m_scale = 1;
    m_image_item->resetTransform();
}

bool ImagePreviewPage::eventFilter(QObject *watched, QEvent *event) {
    qDebug() << event->type();
    if (event->type() == QEvent::Wheel) {
        QWheelEvent *ev = static_cast<QWheelEvent *>(event);
        if (ev->modifiers() & Qt::ControlModifier) {
            if (ev->angleDelta().y() > 0) {
                doEnlarge();
            } else if (ev->angleDelta().y() < 0) {
                doReduce();
            }
            return true;
        }
    }
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *ev = static_cast<QMouseEvent *>(event);
        m_is_moveing = true;
        setCursor(Qt::CrossCursor);
        m_last_point = ev->pos();
    }
    if (event->type() == QEvent::MouseMove && m_is_moveing) {
        QMouseEvent *ev = static_cast<QMouseEvent *>(event);
        //qDebug() << "R " << ev->pos() <<  ev->globalPos() << ev->screenPos();

        double dx = (ev->pos().x() - m_last_point.x()) / (m_base_scale * m_scale);
        double dy = (ev->pos().y() - m_last_point.y()) / (m_base_scale * m_scale);

        m_image_item->setTransform(m_image_item->transform().translate(dx, dy));
        m_last_point = ev->pos();
    }
    if (event->type() == QEvent::MouseButtonRelease) {
        m_is_moveing = false;
        setCursor(Qt::ArrowCursor);
    }
    return QObject::eventFilter(watched, event);
}

void ImagePreviewPage::initControlQss() {
    bool isdark = palette().color(QPalette::Base).lightness() < 128;
    m_tool_bar->setStyleSheet("QPushButton{border:0px;border-radius:4px;background:transparent;}");
    if (isdark) {
        m_reduce_button->setStyleSheet("QPushButton{background-image: url(:/res/1reduce.png);}"
                                       "QPushButton:hover{background-image: url(:/res/1reduce_hover.png);}");

        m_enlarge_button->setStyleSheet("QPushButton{background-image: url(:/res/1enlarge.png);}"
                                        "QPushButton::hover{background-image: url(:/res/1enlarge_hover.png);}");

        m_adaptive_button->setStyleSheet("QPushButton{background:transparent;background-image: url(:/res/1adaptiveWidget.png);}"
                                         "QPushButton::hover{background-image: url(:/res/1adaptiveWidget_hover.png);}");

        m_rotate_button->setStyleSheet("QPushButton{background-image: url(:/res/1rotate.png);}"
                                       "QPushButton::hover{background-image: url(:/res/1rotate_hover.png);}");

        m_horizontal_flip_button->setStyleSheet("QPushButton{background-image: url(:/res/1flipH.png);}"
                                                "QPushButton::hover{background-image: url(:/res/1flipH_hover.png);}");

        m_vertical_flip_button->setStyleSheet("QPushButton{background-image: url(:/res/1flipV.png);}"
                                              "QPushButton::hover{background-image: url(:/res/1flipV_hover.png);}");
    } else {
        m_reduce_button->setStyleSheet("QPushButton{background-image: url(:/res/reduce.png);}"
                                       "QPushButton:hover{background-image: url(:/res/reduce_hover.png);}");

        m_enlarge_button->setStyleSheet("QPushButton{background-image: url(:/res/enlarge.png);}"
                                        "QPushButton::hover{background-image: url(:/res/enlarge_hover.png);}");

        m_adaptive_button->setStyleSheet("QPushButton{background:transparent;background-image: url(:/res/adaptiveWidget.png);}"
                                         "QPushButton::hover{background-image: url(:/res/adaptiveWidget_hover.png);}");

        m_rotate_button->setStyleSheet("QPushButton{background-image: url(:/res/rotate.png);}"
                                       "QPushButton::hover{background-image: url(:/res/rotate_hover.png);}");

        m_horizontal_flip_button->setStyleSheet("QPushButton{background-image: url(:/res/flipH.png);}"
                                                "QPushButton::hover{background-image: url(:/res/flipH_hover.png);}");

        m_vertical_flip_button->setStyleSheet("QPushButton{background-image: url(:/res/flipV.png);}"
                                              "QPushButton::hover{background-image: url(:/res/flipV_hover.png);}");
    }
}

void ImagePreviewPage::changeEvent(QEvent *event) {
    // 仅监听 PaletteChange：主题切换必然伴随 palette 变化，足以触发；
    // 不监听 StyleChange，避免 initControlQss() 内 setStyleSheet 潜在的自触发。
    if (event->type() == QEvent::PaletteChange) {
        initControlQss();
    }
    BasePreviewPage::changeEvent(event);
}
