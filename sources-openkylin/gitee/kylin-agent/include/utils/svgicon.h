#ifndef SVGICON_H
#define SVGICON_H

#include <QIcon>
#include <QString>
#include <QSize>

QIcon loadSvgIcon(const QString &resourcePath, int size = 24);
QIcon loadSvgIcon(const QString &resourcePath, const QSize &size);

#endif // SVGICON_H
