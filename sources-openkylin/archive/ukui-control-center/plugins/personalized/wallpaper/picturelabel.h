#ifndef PICTURELABEL_H
#define PICTURELABEL_H
#include<QLabel>

class PictureLabel : public QLabel
{
public:
    explicit PictureLabel(QWidget *parent=nullptr, Qt::WindowFlags f=Qt::WindowFlags());
    explicit PictureLabel(const QString &text, QWidget *parent=nullptr, Qt::WindowFlags f=Qt::WindowFlags());
    ~PictureLabel();
    void setLocalPixmap(QPixmap pixmap);
    QRect getSourceRect(const QPixmap &pixmap, const QRect &screenGeometry);
    QRect getSourceRect(const QPixmap &pixmap);
    QRect getDestRect(const QPixmap &pixmap);
    void setMode(QString mode);

protected:
    void paintEvent(QPaintEvent *e);

private:
    QString mode;
    QPixmap m_pixmap;
};

#endif // PICTURELABEL_H
