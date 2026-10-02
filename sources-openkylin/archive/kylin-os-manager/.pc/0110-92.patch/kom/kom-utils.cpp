#include <QWidget>
#include "kom-ukui-gsettings.h"
#include "kom-utils.h"

namespace kom {

double KomUtils::adaptFontSize(double pixelSize) {
    static int defaultPointSize = 10;

    QWidget widget;
    double dpi = widget.logicalDpiX();
    if (dpi < 1)
        dpi = 96.0;
    double currentPointSize = pixelSize * 72 / dpi;

    return currentPointSize / defaultPointSize * UkuiGsettings::getInstance()->getFontSize().toDouble();
}

}
