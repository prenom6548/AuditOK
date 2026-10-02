#ifndef MYCIRCEL_H
#define MYCIRCEL_H

#include <QWidget>
#include <QPalette>

class MyCircel : public QWidget
{
    Q_OBJECT
public:
    explicit MyCircel(int width, int height, QWidget *parent = nullptr);
    ~MyCircel();

protected:
    void paintEvent(QPaintEvent *paintEvent);
signals:
};

#endif // MYCIRCEL_H
