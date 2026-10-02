#ifndef CIRCLELABEL_H
#define CIRCLELABEL_H
#include <QPixmap>
#include <QLabel>
#include <QPushButton>

#define FACEWIDTH 64
#define FACEHEIGHT 64
#define FACEBODER 2

class CircleLabel : public QLabel
{
    Q_OBJECT
public:
    CircleLabel(QWidget *parent = nullptr);
    CircleLabel(QPixmap pixmap, QWidget *parent = nullptr);
    ~CircleLabel();

    void setPixmap(QPixmap pixmap);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent * event);
    void mouseReleaseEvent(QMouseEvent *event);

private:
    QPixmap mPixmap;
};

#endif // CIRCLELABEL_H
