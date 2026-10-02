#include "scopeframe.h"
#include <QApplication>
ScopeFrame::ScopeFrame(QWidget *parent) : QFrame(parent) {}

void ScopeFrame::paintEvent(QPaintEvent *event)
{

    Q_UNUSED(event);
    QPainter painter(this);
    painter.save();
    // 绘制矩形的时候，设置画笔颜色即可
    painter.setPen(QPen(QApplication::palette().highlight().color(), 1));

    painter.drawRect(QRect(leftBtn_PosX, 0, clipAreaWidth, 22));
    QColor NonClipBlock_Color;
    NonClipBlock_Color = QColor(191, 191, 191, 125);

    QPainter leftPainter(this); //左区域非剪辑部分
    leftPainter.setBrush(QBrush(NonClipBlock_Color));
    leftPainter.setPen(QColor(191, 191, 191, 0));
    leftPainter.drawRect(QRect(0, 0, leftBtn_PosX, 22));

    QPainter rightPainter(this); //右区域非剪辑部分
    rightPainter.setBrush(QBrush(NonClipBlock_Color));
    rightPainter.setPen(QColor(191, 191, 191, 0));
    rightPainter.drawRect(QRect(rightBtn_RightPosX, 0, nonClipRightAreaWidth, 22));

    painter.restore();
    return QFrame::paintEvent(event);
}

void ScopeFrame::computeClipAreaWidth(int leftBtn_PosX, int rightBtn_RightBorderPosX, int railWidth,
                                      QVector<myWave *> mywave)
{
    this->leftBtn_PosX = leftBtn_PosX;
    rightBtn_RightPosX = rightBtn_RightBorderPosX;
    clipAreaWidth = rightBtn_RightBorderPosX - leftBtn_PosX;
    nonClipRightAreaWidth = railWidth;
    qDebug() << "小矩形框的个数:" << mywave.count();
    int i, j;
    for (i = 0, j = 0; i < mywave.count(); i++, j++) //频率直方图
    {

        if (i <= leftBtn_PosX * mywave.count() / railWidth) {
            mywave.at(i)->toBlueColor(false); // true:蓝色;false:灰色
        } else {
            mywave.at(i)->toBlueColor(true); // true:蓝色;false:灰色
        }
        if (j > rightBtn_RightPosX * mywave.count() / railWidth) {
            mywave.at(j)->toBlueColor(false); // true:蓝色;false:灰色
        }
    }
    repaint(); //能迅速刷新界面
    //    qDebug()<<"剪辑区域宽度:"<<clipAreaWidth<<"右按钮的右边界位置"<<rightBtn_RightBorderPosX<<"轨道宽度:"<<railWidth;
}
