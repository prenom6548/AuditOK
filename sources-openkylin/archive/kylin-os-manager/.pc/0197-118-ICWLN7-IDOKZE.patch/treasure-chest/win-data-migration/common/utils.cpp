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

}
