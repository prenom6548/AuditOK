#include "gridlinepaint.h"
#include <QPainter>
#include "button.h"

static int BUTTONPAGEHEIGHT = 100;

GridLinePaint::GridLinePaint(QWidget *parent) : QWidget(parent)
{

}
void GridLinePaint::paintEvent(QPaintEvent *event)
{
    /**********根据窗口的自适应行为来设置网格线的位置************/
    QPainter painter(this);
    //设置画笔
    QPen pen(Qt::SolidLine);
    QColor penColor = Qt::white;
    penColor.setAlpha(77);
    pen.setColor(penColor);
    painter.setPen(pen);
    //1. 获取摄像头分辨率的高宽（重点是得到获取比例）
    qreal currentWidth = CurrentDeviceInfo::current_size.available_size.first;
    qreal currentHeight = CurrentDeviceInfo::current_size.available_size.second;
    //2. 获取适配窗口的高宽
    qreal winWidth = this->width();
    qreal winHeight;
    if(Button::video_mode){
        winHeight = CurrentDeviceInfo::winHeight;
    }else{
        winHeight = CurrentDeviceInfo::winHeight - BUTTONPAGEHEIGHT;
    }
    //实际绘制区域的高宽
    qreal viewWidth;
    qreal viewHeight;
    //水平||垂直网格线的间距
    qreal horInterval;
    qreal verInterval;
    //第一条水平直线的两点
    QPointF horLine1Point1;
    QPointF horLine1Point2;
    //第二条水平直线的两点
    QPointF horLine2Point1;
    QPointF horLine2Point2;
    //第一条垂直直线的两点
    QPointF verLine1Point1;
    QPointF verLine1Point2;
    //第二条垂直直线的两点
    QPointF verLine2Point1;
    QPointF verLine2Point2;

    //3.1 若窗口宽/窗口高 >= 分辨率宽/分辨率高，则实际绘制的高度跟随窗口高（此时，判断到黑边在左右两侧）
    if(winWidth/winHeight >= currentWidth/currentHeight){
        viewHeight = winHeight;
        viewWidth = (currentWidth/currentHeight)*winHeight;
        horInterval = viewHeight/3;
        verInterval = viewWidth/3;
        //确定两点位置，从而确定水平方向的第一条直线
        horLine1Point1.setX((winWidth-viewWidth)/2);
        horLine1Point1.setY(horInterval);
        horLine1Point2.setX((winWidth-viewWidth)/2+viewWidth);
        horLine1Point2.setY(horInterval);
        QLineF horLine1(horLine1Point1,horLine1Point2);
        painter.drawLine(horLine1);
        //确定两点位置，从而确定水平方向的第二条直线
        horLine2Point1.setX((winWidth-viewWidth)/2);
        horLine2Point1.setY(horInterval*2);
        horLine2Point2.setX((winWidth-viewWidth)/2+viewWidth);
        horLine2Point2.setY(horInterval*2);
        QLineF horLine2(horLine2Point1,horLine2Point2);
        painter.drawLine(horLine2);
        //确定两点位置，从而确定垂直方向的第一条直线
        verLine1Point1.setX((winWidth-viewWidth)/2 + verInterval);
        verLine1Point1.setY(0);
        verLine1Point2.setX((winWidth-viewWidth)/2 + verInterval);
        verLine1Point2.setY(viewHeight);
        QLineF verLine1(verLine1Point1,verLine1Point2);
        painter.drawLine(verLine1);
        //确定两点位置，从而确定垂直方向的第二条直线
        verLine2Point1.setX((winWidth-viewWidth)/2 + verInterval*2);
        verLine2Point1.setY(0);
        verLine2Point2.setX((winWidth-viewWidth)/2 + verInterval*2);
        verLine2Point2.setY(viewHeight);
        QLineF verLine2(verLine2Point1,verLine2Point2);
        painter.drawLine(verLine2);

    }else{//3.2 若窗口宽/窗口高 < 分辨率宽/分辨率高，则实际绘制的高度跟随窗口宽（此时，判断到黑边在上下两侧）
        viewWidth = winWidth;
        viewHeight = (currentHeight/currentWidth)*winWidth;
        horInterval = viewHeight/3;
        verInterval = viewWidth/3;
        //确定两点位置，从而确定水平方向的第一条直线
        horLine1Point1.setX(0);
        horLine1Point1.setY((winHeight-viewHeight)/2 + horInterval);
        horLine1Point2.setX(viewWidth);
        horLine1Point2.setY((winHeight-viewHeight)/2 + horInterval);
        QLineF horLine1(horLine1Point1,horLine1Point2);
        painter.drawLine(horLine1);
        //确定两点位置，从而确定水平方向的第二条直线
        horLine2Point1.setX(0);
        horLine2Point1.setY((winHeight-viewHeight)/2 + horInterval*2);
        horLine2Point2.setX(viewWidth);
        horLine2Point2.setY((winHeight-viewHeight)/2 + horInterval*2);
        QLineF horLine2(horLine2Point1,horLine2Point2);
        painter.drawLine(horLine2);
        //确定两点位置，从而确定垂直方向的第一条直线
        verLine1Point1.setX(verInterval);
        verLine1Point1.setY((winHeight-viewHeight)/2);
        verLine1Point2.setX(verInterval);
        verLine1Point2.setY((winHeight-viewHeight)/2 +  + viewHeight);
        QLineF verLine1(verLine1Point1,verLine1Point2);
        painter.drawLine(verLine1);
        //确定两点位置，从而确定垂直方向的第二条直线
        verLine2Point1.setX(verInterval*2);
        verLine2Point1.setY((winHeight-viewHeight)/2);
        verLine2Point2.setX(verInterval*2);
        verLine2Point2.setY((winHeight-viewHeight)/2 + viewHeight);
        QLineF verLine2(verLine2Point1,verLine2Point2);
        painter.drawLine(verLine2);
    }
    QWidget::paintEvent(event);
}
