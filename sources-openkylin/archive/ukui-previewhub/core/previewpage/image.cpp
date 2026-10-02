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
#include <QIcon>
#include <QImage>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>
#include <QTransform>
#include <QWheelEvent>

namespace {
constexpr qreal kZoomStep = 0.05;
constexpr qreal kMinimumScale = 0.05;
constexpr int kZoomUpdateIntervalMs = 16;
constexpr int kToolBarWidth = 352;
constexpr int kToolButtonSize = 36;
}

ImagePreviewPage::ImagePreviewPage(bool simplePreview, QWidget *parent)
    : BasePreviewPage(parent),
      m_simple_preview(simplePreview)
{
    m_image_scene = new QGraphicsScene(this);
    m_image_item = nullptr;
    m_image_view = new QGraphicsView(m_image_scene, this);

    this->setMouseTracking(true);
    m_image_view->setMouseTracking(true);
    m_image_view->viewport()->setMouseTracking(true);
    m_image_view->installEventFilter(this);
    m_image_view->viewport()->installEventFilter(this);
    m_image_view->setFrameShape(QFrame::NoFrame);
    m_image_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_image_view->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_image_view->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    m_image_view->setBackgroundBrush(palette().brush(QPalette::Base));

    m_scale = 1;
    m_base_scale = 1;
    m_is_moveing = false;
    m_is_rotate90 = false;

    m_tool_bar = new PreviewControlBar(this);
    m_tool_bar->setMaximumWidth(kToolBarWidth);
    m_tool_bar->setMinimumWidth(0);
    m_tool_layout = new QHBoxLayout(m_tool_bar);
    m_tool_layout->setContentsMargins(8, 0, 8, 0);
    m_tool_layout->setSpacing(8);
    m_tool_layout->setSizeConstraint(QLayout::SetNoConstraint);
    m_tool_layout->addStretch();

    const QSize TOOL_BUTTON = QSize(kToolButtonSize, kToolButtonSize);
    m_reduce_button = new QPushButton(m_tool_bar);
    //m_reduce_button->setText("reduce");
    m_reduce_button->setFixedSize(TOOL_BUTTON);
    m_reduce_button->setFocusPolicy(Qt::NoFocus);
    m_reduce_button->setToolTip(tr("Zoom Out"));
    m_tool_layout->addWidget(m_reduce_button);
    connect(m_reduce_button, &QPushButton::clicked, this, &ImagePreviewPage::doReduce);

    m_scale_label = new QLabel(m_tool_bar);
    m_scale_label->setAlignment(Qt::AlignCenter);
    m_scale_label->setFixedWidth(40);
    m_tool_layout->addWidget(m_scale_label);

    m_enlarge_button = new QPushButton(m_tool_bar);
    //m_enlarge_button->setText("enlarge");
    m_enlarge_button->setFixedSize(TOOL_BUTTON);
    m_enlarge_button->setFocusPolicy(Qt::NoFocus);
    m_enlarge_button->setToolTip(tr("Zoom In"));
    m_tool_layout->addWidget(m_enlarge_button);
    connect(m_enlarge_button, &QPushButton::clicked, this, &ImagePreviewPage::doEnlarge);

    m_adaptive_button = new QPushButton(m_tool_bar);
    //m_adaptive_button->setText("adaptive");
    m_adaptive_button->setFixedSize(TOOL_BUTTON);
    m_adaptive_button->setFocusPolicy(Qt::NoFocus);
    m_adaptive_button->setToolTip(tr("Fit to Window"));
    m_tool_layout->addWidget(m_adaptive_button);
    connect(m_adaptive_button, &QPushButton::clicked, this, &ImagePreviewPage::doAdaptive);

    m_horizontal_flip_button = new QPushButton(m_tool_bar);
    //m_horizontal_flip_button->setText("Hfilp");
    m_horizontal_flip_button->setFixedSize(TOOL_BUTTON);
    m_horizontal_flip_button->setFocusPolicy(Qt::NoFocus);
    m_horizontal_flip_button->setToolTip(tr("Flip Horizontally"));
    m_tool_layout->addWidget(m_horizontal_flip_button);
    connect(m_horizontal_flip_button, &QPushButton::clicked, this, &ImagePreviewPage::doHorizontalFlip);

    m_vertical_flip_button = new QPushButton(m_tool_bar);
    //m_vertical_flip_button->setText("Vflip");
    m_vertical_flip_button->setFixedSize(TOOL_BUTTON);
    m_vertical_flip_button->setFocusPolicy(Qt::NoFocus);
    m_vertical_flip_button->setToolTip(tr("Flip Vertically"));
    m_tool_layout->addWidget(m_vertical_flip_button);
    connect(m_vertical_flip_button, &QPushButton::clicked, this, &ImagePreviewPage::doVerticalFlip);

    m_rotate_button = new QPushButton(m_tool_bar);
    //m_rotate_button->setText("rotate");
    m_rotate_button->setFixedSize(TOOL_BUTTON);
    m_rotate_button->setFocusPolicy(Qt::NoFocus);
    m_rotate_button->setToolTip(tr("Rotate Right"));
    m_tool_layout->addWidget(m_rotate_button);
    connect(m_rotate_button, &QPushButton::clicked, this, &ImagePreviewPage::doRotate);
    m_tool_layout->addStretch();

    initControlQss();
    m_reduce_button->setVisible(!m_simple_preview);
    m_scale_label->setVisible(!m_simple_preview);
    m_enlarge_button->setVisible(!m_simple_preview);

    base_layout->setContentsMargins(0, 0, 0, 0);
    base_layout->addWidget(m_image_view);
    m_tool_bar->setPreviewWidget(m_image_view);
    m_tool_bar->setFloating(true);

    m_fit_timer = new QTimer(this);
    m_fit_timer->setSingleShot(true);
    connect(m_fit_timer, &QTimer::timeout, this, [this]() {
        updateImageTransform(true);
    });

    m_zoom_timer = new QTimer(this);
    m_zoom_timer->setSingleShot(true);
    m_zoom_timer->setInterval(kZoomUpdateIntervalMs);
    connect(m_zoom_timer, &QTimer::timeout, this, [this]() {
        applyImageTransform();
    });
}

ImagePreviewPage::~ImagePreviewPage() {
}

void ImagePreviewPage::updateInfo(PreviewHubFileInfo *info) {
    m_fit_timer->stop();
    m_zoom_timer->stop();
    m_image_view->setUpdatesEnabled(true);
    m_tool_bar->showBar(true);
    m_is_rotate90 = false;
    m_scale = 1;
    m_image_scene->clear();
    QPixmap new_pixmap(info->filePath());
    m_image_item = m_image_scene->addPixmap(new_pixmap);
    m_image_item->setFlag(QGraphicsItem::ItemIsMovable, true);
    m_image_item->setTransformationMode(Qt::SmoothTransformation);
    updateSceneRect();
    scheduleImageFit();
}

void ImagePreviewPage::cancel() {
    m_fit_timer->stop();
    m_zoom_timer->stop();
    m_is_moveing = false;
    m_image_view->setUpdatesEnabled(false);
    m_tool_bar->stop();
    m_image_scene->clear();
    m_image_item = nullptr;
}

void ImagePreviewPage::doHorizontalFlip() {
    if (!m_image_item)
        return;
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
    updateSceneRect();
}

void ImagePreviewPage::doVerticalFlip() {
    if (!m_image_item)
        return;
    QRectF r = m_image_item->boundingRect();
    m_image_item->setTransform(m_image_item->transform()
                    .translate(r.width() / 2, r.height() /2)
                    .rotate(180, m_is_rotate90 ? Qt::YAxis : Qt::XAxis)
                    .translate(-r.width() / 2, -r.height() / 2));
    updateSceneRect();
}

void ImagePreviewPage::doRotate() {
    if (!m_image_item)
        return;
    m_fit_timer->stop();
    m_zoom_timer->stop();
    QRectF r = m_image_item->boundingRect();
    m_image_item->setTransform(m_image_item->transform()
                    .translate(r.width() / 2, r.height() /2)
                    .rotate(90)
                    .translate(-r.width() / 2, -r.height() / 2));
    m_is_rotate90 = !m_is_rotate90;
    updateSceneRect();
    updateImageTransform(true);
}

void ImagePreviewPage::doEnlarge() {
    if (!m_image_item)
        return;
    m_scale += kZoomStep;
    scheduleImageTransform();
}

void ImagePreviewPage::doReduce() {
    if (!m_image_item)
        return;
    m_scale = qMax(kMinimumScale, m_scale - kZoomStep);
    scheduleImageTransform();
}

void ImagePreviewPage::doAdaptive() {
    if (!m_image_item)
        return;
    m_fit_timer->stop();
    m_zoom_timer->stop();
    m_scale = 1;
    m_image_item->resetTransform();
    m_is_rotate90 = false;
    updateSceneRect();
    updateImageTransform(true);
}

bool ImagePreviewPage::eventFilter(QObject *watched, QEvent *event) {
    if (!m_image_item)
        return BasePreviewPage::eventFilter(watched, event);

    // The viewport also resizes when a scroll bar appears or disappears.
    // Re-fitting here feeds that change back into the scroll-bar calculation.
    // Only an actual QGraphicsView resize needs a new fit scale.
    if (watched == m_image_view && event->type() == QEvent::Resize) {
        m_zoom_timer->stop();
        scheduleImageFit();
        return BasePreviewPage::eventFilter(watched, event);
    }

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
        setCursor(Qt::ClosedHandCursor);
        m_last_point = ev->pos();
    }
    if (event->type() == QEvent::MouseMove && m_is_moveing) {
        QMouseEvent *ev = static_cast<QMouseEvent *>(event);
        //qDebug() << "R " << ev->pos() <<  ev->globalPos() << ev->screenPos();

        const qreal scale = m_base_scale * m_scale;
        if (qFuzzyIsNull(scale))
            return BasePreviewPage::eventFilter(watched, event);

        double dx = (ev->pos().x() - m_last_point.x()) / scale;
        double dy = (ev->pos().y() - m_last_point.y()) / scale;

        m_image_item->setTransform(m_image_item->transform().translate(dx, dy));
        m_last_point = ev->pos();
    }
    if (event->type() == QEvent::MouseButtonRelease) {
        m_is_moveing = false;
        setCursor(Qt::ArrowCursor);
    }
    return BasePreviewPage::eventFilter(watched, event);
}

void ImagePreviewPage::initControlQss() {
    bool isdark = palette().color(QPalette::Base).lightness() < 128;
    const auto setButtonIcon = [](QPushButton *button, const QString &normal,
                                  const QString &hover) {
        QIcon icon;
        icon.addFile(normal, QSize(), QIcon::Normal);
        icon.addFile(hover, QSize(), QIcon::Active);
        button->setIcon(icon);
        button->setIconSize(QSize(24, 24));
        button->setStyleSheet(
            "QPushButton{border:0px;border-radius:4px;background:transparent;}");
    };
    if (isdark) {
        setButtonIcon(m_reduce_button, ":/res/1reduce.png", ":/res/1reduce_hover.png");
        setButtonIcon(m_enlarge_button, ":/res/1enlarge.png", ":/res/1enlarge_hover.png");
        setButtonIcon(m_adaptive_button, ":/res/1adaptiveWidget.png", ":/res/1adaptiveWidget_hover.png");
        setButtonIcon(m_rotate_button, ":/res/1rotate.png", ":/res/1rotate_hover.png");
        setButtonIcon(m_horizontal_flip_button, ":/res/1flipH.png", ":/res/1flipH_hover.png");
        setButtonIcon(m_vertical_flip_button, ":/res/1flipV.png", ":/res/1flipV_hover.png");
    } else {
        setButtonIcon(m_reduce_button, ":/res/reduce.png", ":/res/reduce_hover.png");
        setButtonIcon(m_enlarge_button, ":/res/enlarge.png", ":/res/enlarge_hover.png");
        setButtonIcon(m_adaptive_button, ":/res/adaptiveWidget.png", ":/res/adaptiveWidget_hover.png");
        setButtonIcon(m_rotate_button, ":/res/rotate.png", ":/res/rotate_hover.png");
        setButtonIcon(m_horizontal_flip_button, ":/res/flipH.png", ":/res/flipH_hover.png");
        setButtonIcon(m_vertical_flip_button, ":/res/flipV.png", ":/res/flipV_hover.png");
    }
    m_scale_label->setStyleSheet(QString("QLabel{color:%1;font-size:12px;background:transparent;}")
                                     .arg(isdark ? "#f1f1f1" : "#2d2d2d"));
}

void ImagePreviewPage::updateImageTransform(bool centerImage) {
    if (!m_image_item || m_image_item->pixmap().isNull())
        return;

    // Keep the fit scale independent from scroll bars that are temporarily
    // shown while the image is zoomed.  The current viewport is smaller in
    // that case, while maximumViewportSize() is the eventual fit area.
    const QSize viewportSize = m_image_view->maximumViewportSize();
    if (viewportSize.isEmpty())
        return;

    const QPixmap pixmap = m_image_item->pixmap();
    const QSize imageSize = m_is_rotate90
        ? QSize(pixmap.height(), pixmap.width())
        : pixmap.size();
    m_base_scale = qMin(static_cast<qreal>(viewportSize.width()) / imageSize.width(),
                        static_cast<qreal>(viewportSize.height()) / imageSize.height());

    applyImageTransform(centerImage);
}

void ImagePreviewPage::applyImageTransform(bool centerImage) {
    if (!m_image_item)
        return;

    QTransform transform;
    const qreal scale = m_base_scale * m_scale;
    transform.scale(scale, scale);
    if (m_image_view->transform() != transform)
        m_image_view->setTransform(transform);
    if (centerImage)
        m_image_view->centerOn(m_image_item);
    updateScaleLabel();
}

void ImagePreviewPage::scheduleImageFit() {
    if (!m_fit_timer->isActive())
        m_fit_timer->start();
}

void ImagePreviewPage::scheduleImageTransform() {
    if (!m_zoom_timer->isActive())
        m_zoom_timer->start();
}

void ImagePreviewPage::updateScaleLabel() {
    const QString text = QString::number(qRound(m_base_scale * m_scale * 100)) + "%";
    if (m_scale_label->text() != text)
        m_scale_label->setText(text);
    if (m_scale_label->toolTip() != text)
        m_scale_label->setToolTip(text);
}

void ImagePreviewPage::updateSceneRect() {
    if (m_image_item)
        m_image_scene->setSceneRect(m_image_item->sceneBoundingRect());
}

void ImagePreviewPage::changeEvent(QEvent *event) {
    // 仅监听 PaletteChange：主题切换必然伴随 palette 变化，足以触发；
    // 不监听 StyleChange，避免 initControlQss() 内 setStyleSheet 潜在的自触发。
    if (event->type() == QEvent::PaletteChange) {
        initControlQss();
        m_image_view->setBackgroundBrush(palette().brush(QPalette::Base));
    }
    BasePreviewPage::changeEvent(event);
}
