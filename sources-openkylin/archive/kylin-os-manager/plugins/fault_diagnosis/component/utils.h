#ifndef K_UTILS_H
#define K_UTILS_H

#include <QPixmap>
#include <QDir>
#include <QColor>

namespace K {

class Utils {
public:
    Utils();
    ~Utils();

    enum class Theme {
        kUkuiDefault = 0,
        kUkuiLight,
        kUkuiDark,
    };

    static double AdaptPixelSizeToPointSize(double pixel_size);
    static QPixmap ChangePixmapColor(const QPixmap &pixmap, const QColor &color);
    static QString ByteToString(quint64 byte);
    static quint64 CalculateDirSize(const QDir &dir);
    static void RemoveDir(const QString &dir_path);
    /**
     * @brief: 获取当前主题
     */
    static enum Theme CurrentTheme();
    /**
     * @brief: 获取当前主题下的 PlaceholderText 颜色
     */
     static QColor CurrentPlaceholderTextColor();
};

}

#endif
