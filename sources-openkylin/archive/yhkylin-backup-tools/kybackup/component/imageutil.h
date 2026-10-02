#ifndef IMAGEUTIL_H
#define IMAGEUTIL_H

#include <QPixmap>
// #include <QSvgRenderer>
#include <QIcon>

class ImageUtil
{
public:
    static const QPixmap loadTheme(const QString &theme, const QString &defaultTheme, const QString& color, QSize size);
    static const QPixmap loadTheme(const QString &theme, const QString &defaultTheme, const QString &color, int size = 16);
    static const QPixmap loadPixmap(QIcon &icon, const QString &color, int size = 16);
    static QPixmap drawSymbolicColoredPixmap(const QPixmap &source, const QString &cgColor);
};

#endif // IMAGEUTIL_H
