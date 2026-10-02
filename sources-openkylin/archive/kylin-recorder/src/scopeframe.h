#ifndef SCOPEFRAME_H
#define SCOPEFRAME_H

#include <QWidget>
#include <QFrame>
#include <QDebug>
#include <QPainter>

#include "mywave.h"

class ScopeFrame : public QFrame
{
    Q_OBJECT
public:
    explicit ScopeFrame(QWidget *parent = nullptr);

    void paintEvent(QPaintEvent *);

    int leftBtn_PosX = 0;       //左按钮的x坐标
    int rightBtn_RightPosX = 0; //右按钮的右边界x坐标

    int nonClipRightAreaWidth = 0;

    int clipAreaWidth = 0;

    QVector<myWave *> mywave;

signals:

public slots:

    void computeClipAreaWidth(int left_PosX, int right_PosX, int railWidth, QVector<myWave *> mywave);
};

#endif // SCOPEFRAME_H
