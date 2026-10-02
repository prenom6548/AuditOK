#ifndef K_UTILS_H
#define K_UTILS_H

#include <QPixmap>

namespace K {

class Utils {
public:
    Utils();
    ~Utils();
    static double AdaptPixelSizeToPointSize(double pixel_size);
    static QPixmap ChangePixmapColor(const QPixmap &pixmap, const QColor &color);
};

}

#endif
