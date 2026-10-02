#ifndef K_UTILS_H
#define K_UTILS_H

#include <QPixmap>
#include <QDir>

namespace K {

class Utils {
public:
    Utils();
    ~Utils();
    static double AdaptPixelSizeToPointSize(double pixel_size);
    static QPixmap ChangePixmapColor(const QPixmap &pixmap, const QColor &color);
    static QString ByteToString(quint64 byte);
    static quint64 CalculateDirSize(const QDir &dir);
    static void RemoveDir(const QString &dir_path);
};

}

#endif
