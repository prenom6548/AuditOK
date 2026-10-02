#ifndef PICTUREICONLABEL_H
#define PICTUREICONLABEL_H

#include <QLabel>

#include "drawiconlabel.h"

class PictureIconLabel : public QLabel
{
    Q_OBJECT
public:
    explicit PictureIconLabel(QWidget *parent = nullptr);
    explicit PictureIconLabel(const QPixmap &pixmap, QWidget *parent = nullptr);
    explicit PictureIconLabel(QStringList iStringList, QWidget *parent = nullptr);
    explicit PictureIconLabel(QList<QPixmap> iconList, QWidget *parent = nullptr);

    QPixmap pixmapToRound(const QString &filePath, const QSize &scaledSize) const;
    QPixmap pixmapToRound(const QPixmap &pixmap, const QSize &scaledSize) const;
    void setRadius(int radius);
    void setIcon(const QPixmap &pixmap);

protected:
    void leaveEvent(QEvent * event);
    void enterEvent(QEvent * event);

private:
    DrawIconLabel* m_pictrueLabel = nullptr;
    QStringList m_iconList;
    int m_radius;

Q_SIGNALS:
    void enterWidget();
    void leaveWidget();
};

#endif // PICTUREICONLABEL_H
