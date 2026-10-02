#include <QPainter>
#include <QPen>
#include <QConicalGradient>
#include <QIcon>
#include <QApplication>
#include "utils.h"
#include "icon_label.h"

namespace K {

IconLabel::IconLabel(QWidget *parent)
    : QLabel(parent),
      timer_(new QTimer(this)),
      start_angle_(0),
      is_loading_(false)
{
    timer_->setInterval(100);
    connect(timer_, &QTimer::timeout, this, [this]() {
        update();
    });
}

IconLabel::~IconLabel() {

}

void IconLabel::Loading(bool is_loading) {
    is_loading_ = is_loading;
    if (is_loading_)
        timer_->start();
    else
        timer_->stop();
    update();
}

void IconLabel::SetBackgroundColor(const QColor &color, BackgroundMode background_mode) {
    background_color_ = color;
    background_mode_ = background_mode;
    update();
}

void IconLabel::SetIcon(const QIcon &icon, double reduction_ratio) {
    icon_ = icon;
    icon_reduction_ratio_ = reduction_ratio;
    update();
}

void IconLabel::SetPixmap(const QPixmap &pixmap, double reduction_ratio) {
    pixmap_ = pixmap;
    pixmap_reduction_ratio_ = reduction_ratio;
    update();
}

void IconLabel::paintEvent(QPaintEvent *event) {
    QPainter p(this);
    p.setRenderHint(QPainter::HighQualityAntialiasing);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    // 画背景色
    if (background_color_.isValid()) {
        QBrush brush(background_color_);
        p.setPen(Qt::NoPen);
        p.setBrush(brush);
        switch (background_mode_) {
        case BackgroundMode::kCircularMode:
            p.drawEllipse(rect());
            break;
        default:
            break;
        }
    }

    // 画加载中圆环
    if (is_loading_) {
        QConicalGradient gradient(rect().center(), 0);
        gradient.setColorAt(0, Qt::blue);
        gradient.setColorAt(0.49, Qt::white);
        gradient.setColorAt(0.5, Qt::blue);
        gradient.setColorAt(1, Qt::white);

        QPen pen(gradient, 3);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        p.drawArc(rect(), start_angle_, -120 * 16);
        p.drawArc(rect(), start_angle_ - 120 * 16 - 60 * 16, -120 * 16);
        start_angle_ -= 10 * 16;
        if (start_angle_ <= -360 * 16)
            start_angle_ = 0;
    }

    // 画图片，将矩形进行等比缩小
    if (!icon_.isNull()) {
        auto icon_rect_width = rect().width() * (1 - icon_reduction_ratio_);
        auto icon_rect_height = rect().height() * (1 - icon_reduction_ratio_);
        auto icon_rect_x = rect().x() + (rect().width() - icon_rect_width) / 2;
        auto icon_rect_y = rect().y() + (rect().height() - icon_rect_height) / 2;
        QRectF icon_rect(icon_rect_x, icon_rect_y, icon_rect_width, icon_rect_height);
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::NoBrush);
        auto pixmap = icon_.pixmap(icon_rect_width, icon_rect_height);
        auto target_pixmap = Utils::ChangePixmapColor(pixmap, qApp->palette().color(QPalette::Text));
        p.drawPixmap(icon_rect, target_pixmap, QRectF(0, 0, icon_rect_width, icon_rect_height));
    }

    if (!pixmap_.isNull()) {
        auto pixmap_rect_width = rect().width() * (1 - pixmap_reduction_ratio_);
        auto pixmap_rect_height = rect().height() * (1 - pixmap_reduction_ratio_);
        auto pixmap_rect_x = rect().x() + (rect().width() - pixmap_rect_width) / 2;
        auto pixmap_rect_y= rect().y() + (rect().height() - pixmap_rect_height) / 2;
        QRectF pixmap_rect(pixmap_rect_x, pixmap_rect_y, pixmap_rect_width, pixmap_rect_height);
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::NoBrush);
        p.drawPixmap(pixmap_rect, pixmap_.scaled(pixmap_rect_width, pixmap_rect_height), QRectF(0, 0, pixmap_rect_width, pixmap_rect_height));
    }
}

}
