#ifndef FRAMELINE_H
#define FRAMELINE_H

#include <QWidget>
#include <QFrame>
#include <QPainter>

class FrameLine : public QFrame
{
public:
    FrameLine(QWidget *parent = nullptr);
    ~FrameLine() = default;

protected:
    void paintEvent(QPaintEvent *event);
};

#endif // FRAMELINE_H
