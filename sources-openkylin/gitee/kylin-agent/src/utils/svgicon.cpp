#include "utils/svgicon.h"
#include <QSvgRenderer>
#include <QPixmap>
#include <QPainter>
#include <QColor>
#include <QDebug>

QIcon loadSvgIcon(const QString &resourcePath, int size)
{
    QSvgRenderer renderer(resourcePath);
    if (!renderer.isValid()) {
        qDebug() << "Invalid SVG:" << resourcePath;
        return QIcon();
    }
    
    QPixmap pixmap(size, size);
    pixmap.fill(QColor(0, 0, 0, 0));  // Transparent
    
    QPainter painter(&pixmap);
    renderer.render(&painter);
    painter.end();
    
    return QIcon(pixmap);
}

QIcon loadSvgIcon(const QString &resourcePath, const QSize &size)
{
    QSvgRenderer renderer(resourcePath);
    if (!renderer.isValid()) {
        qDebug() << "Invalid SVG:" << resourcePath;
        return QIcon();
    }
    
    QPixmap pixmap(size);
    pixmap.fill(QColor(0, 0, 0, 0));  // Transparent
    
    QPainter painter(&pixmap);
    renderer.render(&painter);
    painter.end();
    
    return QIcon(pixmap);
}
