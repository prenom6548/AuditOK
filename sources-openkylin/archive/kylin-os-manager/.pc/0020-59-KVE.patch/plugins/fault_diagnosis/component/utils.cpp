#include <QWidget>
#include "kom-ukui-gsettings.h"
#include "utils.h"

namespace K {

Utils::Utils() {

}

Utils::~Utils() {

}

double Utils::AdaptPixelSizeToPointSize(double pixel_size) {
    static int default_point_size = 10;

    // pixel size 转 point size
    QWidget widget;
    double dpi = widget.logicalDpiX();
    if (dpi < 1)
        dpi = 96.0;
    double point_size = pixel_size * 72 / dpi;

    // 适配 point size 到当前字号
    return point_size / default_point_size * kom::UkuiGsettings::getInstance()->getFontSize().toDouble();
}

QPixmap Utils::ChangePixmapColor(const QPixmap &pixmap, const QColor &color) {
    auto image = pixmap.toImage();
    for (int i = 0; i < image.width(); i++) {
        for (int j = 0; j < image.height(); j++) {
            auto pixel_color = image.pixelColor(i, j);
            if (pixel_color.alpha() > 0) {
                pixel_color.setRed(color.red());
                pixel_color.setGreen(color.green());
                pixel_color.setBlue(color.blue());
                image.setPixelColor(i, j, pixel_color);
            }
        }
    }

    return QPixmap::fromImage(image);
}

}
